#include "srv_iniv.h"

const char* SQL_SEL_VOTE = "SELECT * FROM Voters WHERE Idn = %d;";
const char* SQL_UPDV_VOTE = "UPDATE Voters SET Vot = '%s' WHERE Idn = %d;";
const char* SQL_UPDM_VOTE = "UPDATE Voters SET Mac = '%s' WHERE Idn = %d;";
const char* SQL_UPDT_VOTE = "UPDATE Voters SET Tmr = %d WHERE Idn = %d;";



//sql read callback
int srv_vote_cb(void* arg, int n, char** data ,char** name)
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
short srv_vote(unsigned char* pkt)
{
 unsigned int id=mtoi(pkt+VOTE_RN); //user's id
 int tt=mtoi(pkt+VOTE_RW); //count
 unsigned int crc=mtoi(pkt+VOTE_RC); //packets crc

 short ret=0;
 short i; //general
 char vote_once=1;

 char str[256]; //string
 char *err = 0;    //sql error pointer
 unsigned char e=0; //error flag
 unsigned char r=0; //replay flag
 unsigned char res[36]; //data arrey
 fp_t q; //ECC field value
 ecpoint_fp P; //ECC uncompressed point
 ecpoint_fp R; //ECC multiplication result
 ecpoint_fp2 PP; //ECC Q2 point




 while(1) //loop for use break instead return
 {
  //check packet's type
  if(VOTE!=pkt[2])
  {
   e=ERR_VOTE_TYPE;
   break;
  }

 //check packet's length
  if(VOTE_RL!=mtos(pkt))
  {
   e=ERR_VOTE_LEN;
   break;
  }

 //check crc
  if(crc!=crc32_le(pkt, VOTE_RC))
  {
   e=ERR_VOTE_CRC;
   break; //no send any answer
  }

  //check db is open
  if(!vot_db)
  {
   printf("Error SQL: DB not opened\r\n");
   e=ERR_VOTE_DBOPEN;
   break;
  }


  //serch in DB by Id

  sprintf(str, SQL_SEL_VOTE, id); //SQL DB search requst
  vdata->n=0; //clear number of results
  ret=sqlite3_exec(vot_db, str, srv_vote_cb, 0, &err);  //search this key in voters DB
  if (ret) //db error
  {
   printf("Error SQL: %sn", err);
   sqlite3_free(err);
   e=ERR_VOTE_DBREAD;
   break;
  }

  if(vdata->n!=1)  //not exist
  {
   printf("Id search: %d\r\n", vdata->n);
   e=ERR_VOTE_DBFIND;
   break;
  }


  if(vdata->Tmr==0x7FFFFFFF)
  {
   printf("Tmr opened\r\n");
   e=ERR_VOTE_TMRF;
   break;
  }

  if((vdata->Tmr>tt) || (!vdata->Tmr))
  {
   printf("Tmr invalid: %d-%d\r\n", vdata->Tmr, tt);
   e=ERR_VOTE_TMRL;
   break;
  }

  //if((vdata->Tmr==tt)&&(tt>1)) r=SRV_VOTE_REVOTE;  //replay

  //get hash of client's Q2 pubkey from DB data
  vdata->Key[sizeof(vdata->Key)-1]=0;
  i=str2bin(vdata->Key, res, 16);
  if(i!=16)
  {
   printf("Key invalid\r\n");
   e=ERR_VOTE_KEY;
   break;
  }

  //check hash of client's Q2 pubkey is equal to Key
  memset(&PP, 0, sizeof(PP));
  memcpy(&PP, pkt+VOTE_RK, VOTE_RK_LEN); //get  client's Q2 pubkey
  sh_ini();
  sh_upd(&PP, FP2_PK_LEN);
  sh_xof();
  sh_crp(res, 16); //check hash
  i=iszero(res, 16);
  if(!i)
  {
   printf("Pubkey invalid\r\n");
   e=ERR_VOTE_PUB;
   break;
  }

  //hash packet to message
  sh_ini();
  sh_upd(pkt, VOTE_RA);
  sh_xof();
  sh_out(res, 32); //output fp value

  //hash to point
  memcpy(q, res, sizeof(q));
  i=bls_hash(&R, q);   //message
  if(!i) //point invalid
  {
   printf("Invalid point!"); //set error verbose
   e=ERR_VOTE_HASH;
   break;
  }


  //get client's signature
  memcpy(q, pkt+VOTE_RA, sizeof(q)); //get compressed client's signature
  i=bls_uncompress(&P, q); //uncompress signature
  if(!i) //point invalid
  {
   printf("Invalid point!"); //set error verbose
   e=ERR_VOTE_SIG;
   break;
  }

  //verufy client's signature
  i=bls_verify(&R, &P, &PP);
  if(!i)
  {
   printf("Invalid signature!"); //set error verbose
   e=ERR_VOTE_VERIFY;
   break;
  }

  //check we already voted
  if(vdata->Vot[0])
  {
   r=SRV_VOTE_REPLAY; //vote already exist and will be replaced
   if(vote_once) break;         
  }

    //add count to DB
  sprintf(str, SQL_UPDT_VOTE, tt, id); //SQL DB search requst
  ret=sqlite3_exec(vot_db, str, 0, 0, &err);  //search this key in voters DB
  if (ret) //db error
  {
   printf("Error SQL: %sn", err);
   sqlite3_free(err);
   e=ERR_VOTE_DBSAVET;
   break;
  }

  //add mac to DB
  bin2str(pkt+VOTE_RM, (char*)res, 16);
  strncpy(vdata->Mac, (char*)res, sizeof(vdata->Mac));
  sprintf(str, SQL_UPDM_VOTE, (char*)res, id); //SQL DB search requst
  ret=sqlite3_exec(vot_db, str, 0, 0, &err);  //search this key in voters DB
  if (ret) //db error
  {
   printf("Error SQL: %sn", err);
   sqlite3_free(err);
   e=ERR_VOTE_DBSAVEM;
   break;
  }

  //add voting to DB
  bin2str(pkt+VOTE_RV, (char*)res, 16);
  strncpy(vdata->Vot, (char*)res, sizeof(vdata->Vot));
  sprintf(str, SQL_UPDV_VOTE, (char*)res, id); //SQL DB search requst
  ret=sqlite3_exec(vot_db, str, 0, 0, &err);  //search this key in voters DB
  if (ret) //db error
  {
   printf("Error SQL: %sn", err);
   sqlite3_free(err);
   e=ERR_VOTE_DBSAVEV;
   break;
  }

  break;
 }

 //get actual voting data from DB 
 while(1)
 {
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
  break;

 }
   memclr(pkt, VOTE_RL); //clear data

  //check there are no fatal errors
  if(!e)
  {
   memclr(pkt, VOTE_AL); //clear data
   stom(pkt, VOTE_AL);  //packet's len
   pkt[2]=VOTE; //type
   pkt[3]=r; //note

   itom(pkt+VOTE_AN, id);  //id
   itom(pkt+VOTE_AW, tt);  //counter
   memcpy(pkt+VOTE_AV, res, VOTE_AV_LEN); //encrypted vote
   memcpy(pkt+VOTE_AM, res+VOTE_AV_LEN, VOTE_AM_LEN); //mac

   //signing header, id and key
   memcpy(res, "VOTE", 4); //salt for joivote ticket
   sh_ini();
   sh_upd(res, 4); //hash salt
   sh_upd(pkt, VOTE_AT); //hash packet
   sh_xof();
   sh_out(res, 32); //output fp[32] message

   //convert message to point
   memcpy(q, res, sizeof(q));
   i=bls_hash(&P, q); //message
   if(!i) //point invalid
   {
    printf("Invalid point!"); //set error verbose
    e=ERR_VOTE_TI_HASH;
   }
   else //point valid: sign it
   {
    bls_sign(&R, &P, vkeys->x); //sign message point
    bls_compress(q, &R); //compress signature
    memcpy(pkt+VOTE_AT, q, sizeof(q)); //output to packet
   }
  }

  if(e)//check for fatal error
  {
   memclr(pkt, VOTE_AL); //clear data
   stom(pkt, VOTE_AL);  //packet's len
   pkt[2]=VOTE; //type
   pkt[3]=e+SRV_FATAL_ERR; //server's fatal error
  }

  //compute CRC
  crc=crc32_le(pkt, VOTE_AC); //compute CRC
  itom(pkt+VOTE_AC, crc);  //output

   #ifdef SRV_VLOG
  printf("Client %d voting N%d: %s  %s\r\n", id, tt, srv_note(r), srv_verb(e));
  #else
  if(e) printf("Scorer: voter not vote with error %d\r\n", e);
  else if(r==SRV_VOTE_REPLAY) printf("Scorer: voter %d replay his vote\r\n", id);
  else if(r==SRV_VOTE_REVOTE) printf("Scorer: voter %d change his vote\r\n", id);
  else printf("Scorer: voter %d set his vote\r\n", id);

  #endif



   //clear data
  id=0; crc=0;
  e=0; r=0;
  sh_clr();
  memclr(vdata, sizeof(vot_data));
  memclr(q, sizeof(q));
  memclr(&P, sizeof(P));
  memclr(&R, sizeof(R));
  memclr(&PP, sizeof(R));
  memclr(str, sizeof(str)); //clear incoming data
  memclr(res, sizeof(res)); //clear incoming data
  return VOTE_AL;  //returns length of output
  
}