#include "srv_iniv.h"

const char* SQL_SEL_OPEN = "SELECT * FROM Voters WHERE Idn = %d;";
const char* SQL_UPDV_OPEN = "UPDATE Voters SET Vot = '%s' WHERE Idn = %d;";
const char* SQL_UPDT_OPEN = "UPDATE Voters SET Tmr = %d WHERE Idn = %d;";

//sql read callback
int srv_open_cb(void* arg, int n, char** data ,char** name)
{
 int i;
 if(n<5) return 0;
 i=vdata->n;
 memset(vdata, 0, sizeof(vot_data));
 vdata->n=i;
 vdata->n++;
 if(data[0]) vdata->Idn=atoi(data[0]);
 if(data[1]) strncpy(vdata->Key, data[1], sizeof(vdata->Key));
 if(data[2]) strncpy(vdata->Vot, data[2], sizeof(vdata->Vot));
 if(data[3]) strncpy(vdata->Mac, data[3], sizeof(vdata->Mac));
 if(data[4]) vdata->Tmr=atoi(data[4]);
 return 0;
}

//returns answer length
short srv_open(unsigned char* pkt)
{
 unsigned int id=mtoi(pkt+OPEN_RN); //user's id
 unsigned int crc=mtoi(pkt+OPEN_RC); //packets crc
 short ret=0;
 short i; //general

 char str[256]; //string
 char *err = 0;    //sql error pointer
 unsigned char e=0; //error flag
 unsigned char r=0; //replay flag
 
 unsigned char res[36]; //data arrey
 unsigned char vd[16]; //decrypted vote
 fp_t q; //ECC field value
 ecpoint_fp P; //ECC uncompressed point
 ecpoint_fp R; //ECC multiplication result


 while(1) //loop for use break instead return
 {
  //check packet's type
  if(OPEN!=pkt[2])
  {
   e=ERR_OPEN_TYPE;
   break;
  }

 //check packet's length
  if(OPEN_RL!=mtos(pkt))
  {
   e=ERR_OPEN_LEN;
   break;
  }

 //check crc
  if(crc!=crc32_le(pkt, OPEN_RC))
  {
   e=ERR_OPEN_CRC;
   break; //no send any answer
  }

  //check db is open
  if(!vot_db)
  {
   printf("Error SQL: DB not opened\r\n");
   e=ERR_OPEN_DBOPEN;
   break;
  }


  //serch in DB by Id
  sprintf(str, SQL_SEL_OPEN, id); //SQL DB search requst
  vdata->n=0; //clear number of results
  ret=sqlite3_exec(vot_db, str, srv_open_cb, 0, &err);  //search this key in voters DB
  if (ret) //db error
  {
   printf("Error SQL: %sn", err);
   sqlite3_free(err);
   e=ERR_OPEN_DBREAD;
   break;
  }

  //check client finded
  if((vdata->n>1)||(!vdata->n))  //not finded
  {
   printf("Key search: %d\r\n", vdata->n);
   e=ERR_OPEN_DBFIND;
   break;
  }

  //check vote not opened yet
  if(vdata->Tmr==0x7FFFFFFF)
  {
    r=SRV_OPEN_ISOPEN;
    printf("Already opened\r\n");
    strncpy((char*)vd, vdata->Vot, sizeof(vd));
    break;
  }


  //get vote from DB
  vdata->Vot[sizeof(vdata->Vot)-1]=0;
  i=str2bin(vdata->Vot, res, 16);
  if(i!=16)
  {
    printf("Bad voting length\r\n");
    e=ERR_OPEN_VLEN;
    break;
  }

  //get mac from db
  vdata->Mac[sizeof(vdata->Mac)-1]=0;
  i=str2bin(vdata->Mac, res+16, 16);
  if(i!=16)
  {
    printf("Bad mac length\r\n");
    e=ERR_OPEN_MLEN;
    break;
  }


  //decrypt
  sh_ini();
  sh_upd(&(vdata->Tmr), sizeof(vdata->Tmr));  //counter (IV)
  sh_upd(pkt+OPEN_RE, OPEN_RE_LEN);  //decryption key
  sh_xof();
  sh_dec(res, vd, 16); //decrypt
  sh_xof();
  sh_crp(res+16, 16); //check mac
  i=iszero(res+16, 16);
  if(!i)
  {
    printf("Bad mac\r\n");
    e=ERR_OPEN_MAC;
    break;
  }

  //save open vote to DB
  vd[15]=0;
  sprintf(str, SQL_UPDV_OPEN, (char*)vd, id); //compose SQL request for add
  ret=sqlite3_exec(vot_db, str, 0, 0, &err); //add hash to DB in new line
  if (ret) //db error
  {
   printf("Error SQL: %sn\r\n", err);
   sqlite3_free(err);
   e=ERR_OPEN_DBSAVEV;
   break;
  }

 //save count to DB
  sprintf(str, SQL_UPDT_OPEN, 0x7FFFFFFF, id); //compose SQL request for add
  ret=sqlite3_exec(vot_db, str, 0, 0, &err); //add hash to DB in new line
  if (ret) //db error
  {
   printf("Error SQL: %sn\r\n", err);
   sqlite3_free(err);
   e=ERR_OPEN_DBSAVET;
   break;
  }

  break;
 }


 //pack data
  memclr(pkt, OPEN_RL); //clear data

  //check there are no fatal errors
  if(!e)
  {
   memclr(pkt, OPEN_AL); //clear data
   stom(pkt, OPEN_AL);  //packet's len
   pkt[2]=OPEN; //type
   pkt[3]=r; //note
   
   itom(pkt+OPEN_AN, id); //add id
   memcpy(pkt+OPEN_AO, vd, OPEN_AO_LEN); //add devrypted vote

   //signing header, id and vote
   memcpy(res, "OPEN", 4); //salt for open ticket
   sh_ini();
   sh_upd(res, 4); //hash salt
   sh_upd(pkt, OPEN_AT); //hash packet
   sh_xof();
   sh_out(res, 32); //output fp[32] message

   //convert message to point
   memcpy(q, res, sizeof(q));
   i=bls_hash(&P, q); //message
   if(!i) //point invalid
   {
    printf("Invalid point!"); //set error verbose
    e=ERR_OPEN_TI_HASH;
   }
   else //point valid: sign it
   {
    bls_sign(&R, &P, vkeys->x); //sign message point
    bls_compress(q, &R); //compress signature
    memcpy(pkt+OPEN_AT, q, sizeof(q)); //output to packet
   }
  }

  if(e)//check for fatal error
  {
   memclr(pkt, OPEN_AL); //clear data
   stom(pkt, OPEN_AL);  //packet's len
   pkt[2]=OPEN; //type
   pkt[3]=e+SRV_FATAL_ERR; //server's fatal error
  }

  //compute CRC
  crc=crc32_le(pkt, OPEN_AC); //compute CRC
  itom(pkt+OPEN_AC, crc);  //output
  #ifdef SRV_VLOG
  printf("Client %d opens vote '%s': %s  %s\r\n", id, vd, srv_note(r), srv_verb(e));
  #else
  if(e) printf("Scorer: voter not open his vote with error %d\r\n", e);
  else if(r) printf("Scorer: voter %d already opened, vote: '%s'\r\n", id, vd);
  else
  {
   char fio[32]={0,};
   r2tru((char*)vd, fio);
   printf("Scorer: voter %d open his vote: '%s'\r\n", id, fio
   );
  }

  #endif
   //clear data
  id=0; crc=0;
  e=0; r=0;
  sh_clr();
  memclr(vdata, sizeof(vot_data));
  memclr(q, sizeof(q));
  memclr(&P, sizeof(P));
  memclr(&R, sizeof(R));
  memclr(str, sizeof(str));
  memclr(res, sizeof(res));
  memclr(vd, sizeof(vd)); 
  return OPEN_AL;  //returns length of output

}
