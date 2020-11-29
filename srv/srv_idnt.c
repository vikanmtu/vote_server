
#include "ecc.h"
#include "srv_inir.h"

const char* SQL_SEL_IDNT = "SELECT * FROM Persons WHERE Idn=%d;";
const char* SQL_UPD_IDNT = "UPDATE Persons SET Sec = '%s' WHERE Idn = %d;";

//sql read callback
int srv_idnt_cb(void* arg, int n, char** data ,char** name)
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


//process request for personal identification, compose answer
//returns answer length
short srv_idnt(unsigned char* pkt)
{
 unsigned int id=mtoi(pkt+IDNT_RD); //user's id
 unsigned int crc=mtoi(pkt+IDNT_RC); //packets crc
 //short i; //general
 short ret;
 char str[256]; //string
 char *err = 0;
 unsigned char e=0; //error flag
 unsigned char r=0; //replay flag

 fp_t x; //SPEKE secret
 fp_t q; //ECC field value
 ecpoint_fp P; //ECC uncompressed point
 ecpoint_fp R; //ECC multiplication result
 unsigned char BP[32];  //SPEKE X25519 base point

 while(1) //loop for use break instead return
 {
  //check packet's type
  if(IDNT!=pkt[2])
  {
   e=ERR_IDNT_TYPE;
   break;
  }

 //check packet's length
  if(IDNT_RL!=mtos(pkt))
  {
   e=ERR_IDNT_LEN;
   break;
  }

  //check crc
  if(crc!=crc32_le(pkt, IDNT_RC))
  {
   e=ERR_IDNT_CRC;
   break; //no send any answer
  }

  //check db is open
  if(!reg_db)
  {
   printf("Error SQL: DB not opened\r\n");
   e=ERR_IDNT_DBOPEN;
   break;
  }

  //serch in DB by id
  sprintf(str, SQL_SEL_IDNT, id);
  rdata->n=0;
  ret=sqlite3_exec(reg_db, str, srv_idnt_cb, 0, &err);
  if (ret) //db error
  {
   printf("Error SQL: %sn", err);
   sqlite3_free(err);
   e=ERR_IDNT_DBREAD;
   break;
  }

  if(rdata->n!=1)  //not found
  {
   printf("Idnt search: %d\r\n", rdata->n);
   e=ERR_IDNT_DBFIND;
   break;
  }


  if(rdata->Sig[0])
  {
   r=SRV_IDNT_ISREGS;
  }

  //check password is specified for this id
  if(!rdata->Psw[0]) //user's password not specified
  {
   e=ERR_IDNT_NOPASS;
   break;
  }

   //decompress pkt+IDNT_RP[32] into point (their SPEKE pubkey)
  memcpy(q, pkt+IDNT_RP, sizeof(q));





  /*
  i=bls_uncompress(&P, q);
  if(!i) //point invalid
  {
   printf("Invalid point!"); //set error verbose
   e=ERR_IDNT_POINT;
   break;
  }
 */


  //generate random speke secret key
  cprng_get_bytes(x, BI_BYTES); //generate random secret  for SPEKE protocol

  //compute SPEKE shared secret

  /*
  i=bls_mult(&R, &P, x);
  if(!i)
  {
   printf("Error mult\r\n");
   e=ERR_IDNT_SSEC;
   break;
  }
 */
   scalarmult(BP, (unsigned char*)x, (unsigned char*)q); //compute SPEKE


  //hash SPEKE shared secret in point R to 16 bytes in res
  sh_ini();
  //sh_upd(&R, 64);
  sh_upd(BP, 32);
  sh_xof();
  sh_out(str, 16);

  //convert hash to hex string
  bin2str((unsigned char*)str, (char*)pkt, 16); //convert 16 bytes in res to string in pkt

  //save str to DB by id as Sec field
  sprintf(str, SQL_UPD_IDNT, (char*)pkt, id);
  ret=sqlite3_exec(reg_db, str, 0, 0, &err);
  if (ret) //some DB error
  {
   printf("Error SQL: %sn", err);
   sqlite3_free(err);
   e=ERR_IDNT_DBSAVESS;
   break;
  }

  //hash password
  rdata->Psw[sizeof(rdata->Psw)-1]=0;
  sh_ini();
  sh_upd(rdata->Psw, strlen(rdata->Psw));
  sh_xof();
  sh_out((unsigned char*)q, sizeof(q)); //hash to fp value


  r2p(BP, (unsigned char*)q); //hash fp value to X25519 point
  scalarmult((unsigned char*)q, (unsigned char*)x, BP); //compute SPEKE pubkey


  /*
  i=bls_hash(&P, q); //hash to point

  if(!i) //invalid point
  {
   printf("Error hash\r\n");
   e=ERR_IDNT_HASH;
   break;
  }

  //compute SPECE public key
  i=bls_mult(&R, &P, x);
  if(!i)
  {
   printf("Error mult\r\n");
   e=ERR_IDNT_GETQ;
   break;
  }

  //compress SPEKE public key to fp value
  bls_compress(q, &R);
  */
  
  break;
 }

 memclr(pkt, IDNT_RL); //clear

 if(!e)
 {
   //output abswer
  memclr(pkt, IDNT_AL); //clear
  stom(pkt, IDNT_AL);  //packet's len
  pkt[2]=IDNT; //type
  pkt[3]=r; //note

  memcpy(pkt+IDNT_AP, q, IDNT_AP_LEN);  //output compressed SPEKE pubkey
  memcpy(pkt+IDNT_AR, &rkeys->XX, IDNT_AR_LEN); //output registrator's pubkey

 }


 if(e) //check for fatal error
 {
  memclr(pkt, IDNT_AL); //clear data
  stom(pkt, IDNT_AL);  //packet's len
  pkt[2]=IDNT; //type
  pkt[3]=e+SRV_FATAL_ERR; //server's fatal error
 }

  crc=crc32_le(pkt, IDNT_AC); //compute CRC
  itom(pkt+IDNT_AC, crc);  //output


  #ifdef SRV_VLOG
  printf("Client %d identify: %s  %s\r\n", id, srv_note(r), srv_verb(e));
  #else
  if(!e) printf("Registrator: client %d identified\r\n", id);
  else printf("Registrator: client %d identification error %d\r\n", id, e);
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
  memclr(BP, sizeof(BP));
  
  return IDNT_AL;  //returns length of output
}