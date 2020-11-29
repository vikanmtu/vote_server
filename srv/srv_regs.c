#include "srv_inir.h"

const char* SQL_SEL_REGS = "SELECT * FROM Persons WHERE Idn=%d;";
const char* SQL_UPD_REGS = "UPDATE Persons SET Sig = '%s' WHERE Idn = %d;";

//sql read callback
int srv_regs_cb(void* arg, int n, char** data ,char** name)
{
 int i;
 if(n<6) return 0;
 i=rdata->n;
 memset(rdata, 0, sizeof(reg_data));
 rdata->n=i;
 rdata->n++;
 if(data[0]) rdata->Idn=atoi(data[0]);
 if(data[1]) strncpy(rdata->Fio, data[1], sizeof(rdata->Fio));
 if(data[2]) strncpy(rdata->Inn, data[2], sizeof(rdata->Inn));
 if(data[3]) strncpy(rdata->Psw, data[3], sizeof(rdata->Psw));
 if(data[4]) strncpy(rdata->Sec, data[4], sizeof(rdata->Sec));
 if(data[5]) strncpy(rdata->Sig, data[5], sizeof(rdata->Sig));
 return 0;
}



//process request for personal registration, compose answer
//returns answer length
short srv_regs(unsigned char* pkt)
{
 unsigned int id=mtoi(pkt+REGS_RD); //user's id
 unsigned int crc=mtoi(pkt+REGS_RC); //packets crc
 short ret=0; 
 short i; //general

 char str[256]; //string
 char *err = 0;
 unsigned char e=0; //error flag
 unsigned char r=0; //replay flag
 fp_t x; //SPEKE secret
 fp_t q; //ECC field value
 ecpoint_fp P; //ECC uncompressed point
 ecpoint_fp R; //ECC multiplication result

 unsigned char res[32];



 while(1) //loop for use break instead return
 {
  //check packet's type
  if(REGS!=pkt[2])
  {
   e=ERR_REGS_TYPE;
   break;
  }

 //check packet's length
  if(REGS_RL!=mtos(pkt))
  {
   e=ERR_REGS_LEN;
   break;
  }

 //check crc
  if(crc!=crc32_le(pkt, REGS_RC))
  {
   e=ERR_REGS_CRC;
   break; //no send any answer
  }

  //check db is open
  if(!reg_db)
  {
   printf("Error SQL: DB not opened\r\n");
   e=ERR_REGS_DBOPEN;
   break;
  }

  //serch in DB by id
  sprintf(str, SQL_SEL_REGS, id);
  rdata->n=0;
  ret=sqlite3_exec(reg_db, str, srv_regs_cb, 0, &err);
  if (ret) //db error
  {
   printf("Error SQL: %sn", err);
   sqlite3_free(err);
   e=ERR_REGS_DBREAD;
   break;
  }

  if(rdata->n!=1)  //not found
  {
   printf("Idnt search: %d\r\n", rdata->n);
   e=ERR_REGS_DBFIND;
   break;
  }


  //check password is specified for this id
  if(!rdata->Psw[0]) //user's password not specified
  {
   e=ERR_REGS_NOPASS;
   break;
  }

  //check secret is agreed
  if(!rdata->Sec[0]) //no secret
  {
   e=ERR_REGS_NOSSEC;
   break;
  }

  //get secret from DB
  rdata->Sec[sizeof(rdata->Sec)-1]=0;
  i=str2bin(rdata->Sec, res, 16);
  if(i!=16)  //invalid
  {
   e=ERR_REGS_SSLEN;
   break;
  }

  //check mac
   sh_ini();
   sh_upd(res, 16);
   sh_upd(pkt, REGS_RM);
   sh_xof();
   sh_crp(pkt+REGS_RM, 16);
   i=iszero(pkt+REGS_RM, 16);

  if(!i) //invalid mac
  {
   e=ERR_REGS_MAC;
   break;
  }

  //get signature from DB
  rdata->Sig[sizeof(rdata->Sig)-1]=0;
  i=str2bin(rdata->Sig, res, 32);

  if(i && (i!=32))
  {
   e=ERR_REGS_BADSIG;
   break;
  }


  if(i==32)  //signature exist
  {
   r=SRV_REGS_ISREGS; //signature already exist
   break; //compose packet
  }

   //decompress blinded message into point
  memcpy(q, pkt+REGS_RQ, sizeof(q));
  i=bls_uncompress(&P, q);
  if(!i) //point invalid
  {
   printf("Invalid point!"); //set error verbose
   e=ERR_REGS_POINT;
   break;
  }

  //sign P to R
  bls_sign(&R, &P, rkeys->x);

  //compress to res
  bls_compress(q, &R);
  memcpy(res, q, 32);

  //convert to string
  bin2str(res, (char*)pkt, 32);


  //save str to DB by id as Sig field
  sprintf(str, SQL_UPD_REGS, (char*)pkt, id);
  ret=sqlite3_exec(reg_db, str, 0, 0, &err);
  if (ret) //some DB error
  {
   printf("Error SQL: %sn", err);
   sqlite3_free(err);
   e=ERR_REGS_DBSAVESIG;
   break;
  }

  break;
 }

 //pack data
  memclr(pkt, REGS_RL);


  if(!e)
  {
   memclr(pkt, REGS_AL); //clear
   stom(pkt, REGS_AL);  //packet's len
   pkt[2]=REGS; //type
   pkt[3]=r; //note

   itom(pkt+REGS_AD, id);  //id
   memcpy(pkt+REGS_AB, res, REGS_AB_LEN); //blind signature
   memcpy(pkt+REGS_AG, rkeys->p, REGS_AG_LEN); //registrators pk in Q1

   //compute mac
   sh_ini();
   sh_upd(res, 16);
   sh_upd(pkt, REGS_AM);
   sh_xof();
   sh_out(pkt+REGS_AM, 16); //mac
  }
  else //fatal error
  {
   memclr(pkt, REGS_AL); //clear data
   stom(pkt, REGS_AL);  //packet's len
   pkt[2]=REGS; //type
   pkt[3]=e+SRV_FATAL_ERR; //server's fatal error
  }

  //compute CRC
  crc=crc32_le(pkt, REGS_AC); //compute CRC
  itom(pkt+REGS_AC, crc);  //output
  #ifdef SRV_VLOG
  printf("Client %d register fail: %s  %s\r\n", id, srv_note(r), srv_verb(e));
  #else
  if(e) printf("Registrator: client %d registration error %d\r\n", id, e);
  else if(r) printf("Registrator: client %d ('%s' INN '%s') already registered\r\n", id, rdata->Fio, rdata->Inn);
  else
  {
   char fio[32]={0,};
   r2tru(rdata->Fio, fio);
   printf("Registrator: client %d ('%s' INN '%s') register success\r\n", id, fio, rdata->Inn);
  }
  #endif
   //clear data
  id=0; crc=0;
  e=0; r=0;
  sh_clr();
  memclr(rdata, sizeof(reg_data));
  memclr(q, sizeof(q));
  memclr(x, sizeof(x));
  memclr(&P, sizeof(P));
  memclr(&R, sizeof(R));
  memclr(str, sizeof(str)); //clear incoming data
  memclr(res, sizeof(res)); //clear incoming data
  return REGS_AL;  //returns length of output
  
}