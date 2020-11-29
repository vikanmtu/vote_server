#include "srv_iniv.h"

const char* SQL_SEL_JOIN = "SELECT * FROM Voters WHERE Key='%s';";
const char* SQL_ADD_JOIN = "INSERT INTO Voters (Key, Tmr) VALUES ('%s', %d);";




//sql read callback
int srv_join_cb(void* arg, int n, char** data ,char** name)
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
short srv_join(unsigned char* pkt)
{
 unsigned int id;//user's id
 unsigned int crc=mtoi(pkt+JOIN_RC); //packets crc
 short ret=0;
 short i; //general
 unsigned int tt; //time

 char str[256]; //string
 char *err = 0;    //sql error pointer
 unsigned char e=0; //error flag
 unsigned char r=0; //replay flag
 unsigned char res[36]; //data arrey
 fp_t q; //ECC field value
 ecpoint_fp P; //ECC uncompressed point
 ecpoint_fp R; //ECC multiplication result





 while(1) //loop for use break instead return
 {
  //check packet's type
  if(JOIN!=pkt[2])
  {
   e=ERR_JOIN_TYPE;
   break;
  }

 //check packet's length
  if(JOIN_RL!=mtos(pkt))
  {
   e=ERR_JOIN_LEN;
   break;
  }


 //check crc
  if(crc!=crc32_le(pkt, JOIN_RC))
  {
   e=ERR_JOIN_CRC;
   break; //no send any answer
  }

  //check db is open
  if(!vot_db)
  {
   printf("Error SQL: DB not opened\r\n");
   e=ERR_JOIN_DBOPEN;
   break;
  }


  //serch in DB by Key
  bin2str(pkt+JOIN_RS, (char*)res, 16);  //convert key[16] from request to hex string in res
  sprintf(str, SQL_SEL_JOIN, (char*)res); //SQL DB search requst
  vdata->n=0; //clear number of results
  ret=sqlite3_exec(vot_db, str, srv_join_cb, 0, &err);  //search this key in voters DB
  if (ret) //db error
  {
   printf("Error SQL: %sn", err);
   sqlite3_free(err);
   e=ERR_JOIN_DBREAD;
   break;
  }

  if(vdata->n>1)  //multiple?
  {
   printf("Key search: %d\r\n", vdata->n);
   e=ERR_JOIN_DBFIND;
   break;
  }


  if(vdata->n==1) //this key already exist
  {
   id=vdata->Idn; //set id of this voter
   r=SRV_JOIN_ISJOIN; //set replay flag
   break;
  }

  //get register's signature from request
  memcpy(q, pkt+JOIN_RU, sizeof(q));
  i=bls_uncompress(&P, q); //registrator's signature
  if(!i) //point invalid
  {
   printf("Invalid point!"); //set error verbose
   e=ERR_JOIN_POINT;
   break;
  }

  //get message point from key
  sh_ini();
  sh_upd(pkt+JOIN_RS, JOIN_RS_LEN);
  sh_xof();
  sh_out(res, 32); //hash key[16] to fp[32] value

  //hash message to point
  memcpy(q, res, sizeof(q));
  i=bls_hash(&R, q);
  if(!i) //point invalid
  {
   printf("Invalid point!"); //set error verbose
   e=ERR_JOIN_HASH;
   break;
  }

  //verify register's signature
  i=bls_verify(&R, &P, &(vkeys->YY));
  if(!i)
  {
   printf("Invalid signature!"); //set error verbose
   e=ERR_JOIN_VERIFY;
   break;
  }

  //add new key to DB
  tt=1; //set count
  bin2str(pkt+JOIN_RS, (char*)res, 16); //convert key to  hex string
  sprintf(str, SQL_ADD_JOIN, (char*)res, tt); //compose SQL request for add
  ret=sqlite3_exec(vot_db, str, 0, 0, &err); //add hash to DB in new line
  if (ret) //db error
  {
   printf("Error SQL: %sn", err);
   sqlite3_free(err);
   e=ERR_JOIN_DBSAVEK;
   break;
  }
  id=sqlite3_last_insert_rowid(vot_db); //get ID of inserted line

  break;
 }


 //pack data
  memcpy(res, pkt+JOIN_RS, JOIN_RS_LEN); //save key from request
  memclr(pkt, JOIN_RL); //clear data

  //check there are no fatal errors
  if(!e)
  {
   memclr(pkt, JOIN_AL); //clear
   stom(pkt, JOIN_AL);  //packet's len
   pkt[2]=JOIN; //type
   pkt[3]=r; //note
   
   itom(pkt+JOIN_AN, id); //add id
   memcpy(pkt+JOIN_AK, vkeys->cnd, JOIN_AK_LEN);
   memcpy(pkt+JOIN_AS, res, JOIN_AS_LEN); //add key from request

   //signing header, id and key
   memcpy(res, "JOIN", 4); //salt for join ticket
   sh_ini();
   sh_upd(res, 4); //hash salt
   sh_upd(pkt, JOIN_AT); //hash packet
   sh_xof();
   sh_out(res, 32); //output fp[32] message

   //convert message to point
   memcpy(q, res, sizeof(q));
   i=bls_hash(&P, q); //message
   if(!i) //point invalid
   {
    printf("Invalid point!"); //set error verbose
    e=ERR_JOIN_TI_HASH;
   }
   else //point valid: sign it
   {
    bls_sign(&R, &P, vkeys->x); //sign message point
    bls_compress(q, &R); //compress signature
    memcpy(pkt+JOIN_AT, q, sizeof(q)); //output to packet
   }
  }

  if(e)//check for fatal error
  {
   memclr(pkt, JOIN_AL); //clear data
   stom(pkt, JOIN_AL);  //packet's len
   pkt[2]=JOIN; //type
   pkt[3]=e+SRV_FATAL_ERR; //server's fatal error
  }

  //compute CRC
  crc=crc32_le(pkt, JOIN_AC); //compute CRC
  itom(pkt+JOIN_AC, crc);  //output
   #ifdef SRV_VLOG
  printf("join %d: %s  %s\r\n", id, srv_note(r), srv_verb(e));
  #else
  if(e) printf("Scorer: voter not joined with error %d\r\n", e);
  else if(r) printf("Scorer: voter already joined as %d\r\n", id);
  else printf("Scorer: voter anonimesly join as %d\r\n", id);

  #endif
   //clear data
  id=0; crc=0;
  e=0; r=0;
  sh_clr();
  memclr(vdata, sizeof(vot_data));
  memclr(q, sizeof(q));
  memclr(&P, sizeof(P));
  memclr(&R, sizeof(R));
  memclr(str, sizeof(str)); //clear incoming data
  memclr(res, sizeof(res)); //clear incoming data
  return JOIN_AL;  //returns length of output
  
}
