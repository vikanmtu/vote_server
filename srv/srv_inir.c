 #include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "srv_inir.h"

 #if defined _WIN32
#include <direct.h>
#elif defined __GNUC__
#include <sys/types.h>
#include <sys/stat.h>
#endif

reg_keys  srkeys;
reg_keys* rkeys = &srkeys;
reg_data  srdata;
reg_data* rdata = &srdata;
sqlite3 *reg_db = 0;

const char* SQL_REG_CREATE = "CREATE TABLE IF NOT EXISTS Persons (Idn INTEGER PRIMARY KEY ASC, Fio varchar(127) NOT NULL, Inn varchar(15) NOT NULL UNIQUE, Psw varchar(31), Sec varchar(63), Sig varchar(127) );";
const char* SQL_REG_ADD = "INSERT INTO Persons (Fio, Inn, Psw) VALUES ('%s','%s','%s');";
//initialization of registrator
short srv_inir(void)
{
 char path[512];
 char str[512];
 int pathlen=0;
 char *err = 0;
 int i, l=0;
 short ret;
 short e=0;
 FILE * pFile;
 fp_t q;
 fp_t b;
 ecpoint_fp M;
 ecpoint_fp B;
 ecpoint_fp D;
 ecpoint_fp S;
 ecpoint_fp R;
 ecpoint_fp Y;
 ecpoint_fp2 YY;

 while(1)
 {

  //initialize trng
  for(i=0;i<1000;i++)
  {
   ret=trng_init();
   if(ret==3) break;
  }

  if(ret!=3)
  {
   e=ERR_INIR_RNG;
   break;
  }


  trng_get((unsigned char*)rkeys->x, sizeof(rkeys->x));
  cprng_init(rkeys->x, sizeof(rkeys->x));


  //get path
  i=wai_getExecutablePath(NULL, 0, NULL);
  if(i<(sizeof(path)-64)) wai_getExecutablePath(path, i, &l);
  if(!l) path[0]=0;
  else for(i=l; i>=0; i--) if((path[i]==92)||(path[i]==47)) break;
  if(i) path[i]=0;
  pathlen=strlen(path);

  //create registrator's data directory
  strncpy(path+pathlen, (char*)"/reg_data", sizeof(path)-pathlen); //set directory path
  #ifdef _WIN32
  mkdir(path);
  #else
  mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
  #endif

  //create DB if not exist
  strncpy(path+pathlen, (char*)"/reg_data/reg.db3", sizeof(path)-pathlen); //set directory path
  if( sqlite3_open("reg_data/reg.db3", &reg_db) )
  {
   printf("error of open/create reg DB: %s\r\n", sqlite3_errmsg(reg_db));
   e=ERR_INIR_DBOPEN;
   break;
  }

  //create table
  ret=sqlite3_exec(reg_db, SQL_REG_CREATE, 0, 0, &err);
  if (ret)
  {
   printf("Error SQL: %sn\r\n", err);
   sqlite3_free(err);
   e=ERR_INIR_DBTAB;
   break;
  }



  //read registrator's private key
  strncpy(path+pathlen, (char*)"/reg_data/reg.sec", sizeof(path)-pathlen); //set directory path
  l=0;
  pFile = fopen (path,"rt"); //try open for read
  if(pFile) //opened
  {
   str[0]=0;
   fgets(str, sizeof(str), pFile);
   fclose (pFile);
   l=str2bin(str, (unsigned char*)rkeys->x, sizeof(rkeys->x));
   memclr(str, sizeof(str));
   pFile=0;
   printf("Registrator's secret key loaded\r\n");
  }
   //read fail: create registrator's keys , save
  if(l!=sizeof(rkeys->x))
  {
   cprng_get_bytes(rkeys->x, BI_BYTES);
   i=bls_key(&(rkeys->XX), &(rkeys->X), rkeys->x); //output: XX is G2 public key, X is G1 public key, x is private key
   if(!i)
   {
    printf("Error reg generate sec\r\n");
    e=ERR_INIR_GENX;
    break;
   }

   //save secret key
   pFile = fopen (path,"wt"); //try open for write
   if(!pFile) //not opened
   {
    printf("Error reg save sec\r\n");
    e=ERR_INIR_SAVEX;
    break;
   }
   bin2str((unsigned char*)rkeys->x, str, sizeof(rkeys->x));
   fprintf(pFile, "%s\r\n", str);
   fclose (pFile);

   //save Q2 pubkey
   strncpy(path+pathlen, (char*)"/reg_data/reg.pub", sizeof(path)-pathlen); //set directory path
   pFile = fopen (path,"wt"); //try open for write
   if(!pFile) //not opened
   {
    printf("Error reg save sec\r\n");
    e=ERR_INIR_SAVEXX;
    break;
   }
   bin2str((unsigned char*)&rkeys->XX, str, sizeof(rkeys->XX));
   fprintf(pFile, "%s\r\n", str);
   fclose (pFile);




   printf("Registrator's keys created\r\n");
  }
  else
  {
   i=bls_key(&(rkeys->XX), &(rkeys->X), rkeys->x); //output: XX is G2 public key, X is G1 public key, x is private key
   if(!i)
   {
    printf("Error reg loaded sec\r\n");
    e=ERR_INIR_GENXX;
    break;
   }
  }

  bls_compress(rkeys->p, &rkeys->X); //compress Q1 pk

  //read registartor's pubkey
  strncpy(path+pathlen, (char*)"/reg_data/reg.pub", sizeof(path)-pathlen); //set directory path
   pFile = fopen (path,"rt"); //try open for read
   if(!pFile) //not opened
   {
    printf("Error reg read pub\r\n");
    e=ERR_INIR_LOADXX;
    break;
   }

   memset(&YY, 0, sizeof(YY));
   str[0]=0;
   fgets(str, sizeof(str), pFile);
   l=str2bin(str, (unsigned char*)(&YY), sizeof(YY));
   fclose (pFile);

   if(l!=sizeof(YY))
   {
    printf("Error reg pub length\r\n");
    e=ERR_INIR_LENXX;
    break;
   }

    i=sizeof(YY);
    i=isequal((unsigned char*)&rkeys->XX, (unsigned char*)&YY, sizeof(YY));
    if(!i)
    {
    printf("Error reg pub\r\n");
    e=ERR_INIR_CHECKXX;
    break;
   }



   //------------------test-----------------------
    //test of blind signature of random message
    cprng_get_bytes(q, BI_BYTES); //generate random message 32 bytes
    bls_hash(&M, q); //hash to point

    cprng_get_bytes(q, BI_BYTES); //generate random blind value
    bls_blind(&B, &M, q); //blind message point to M

    bls_compress(b, &B); //compress blinded point to 32 bytes value
    bls_uncompress(&D, b); //decompress blinded point to D

    bls_sign(&S, &B, rkeys->x); //bling signing

    bls_uncompress(&Y, rkeys->p); //uncompress Q1 key
    bls_unblind(&R, &S, &Y, q); //unbling signature to R
    i=bls_verify(&M, &R, &rkeys->XX); //verify

    if(!i)
    {
     e=ERR_INIR_TEST;
     break;
    }
    break;
   }
   #ifdef SRV_VLOG
  printf("Registrator initialized:  %s\r\n", srv_verb(e));
  #endif

   memclr(path, sizeof(path));
   memclr(str, sizeof(str));
   pathlen=0;
   err = 0;
   l=0;
   ret=0;
   pFile=0;
 memclr(q, sizeof(q));
 memclr(b, sizeof(b));
 memclr(&M, sizeof(M));
 memclr(&B, sizeof(B));
 memclr(&D, sizeof(D));
 memclr(&S, sizeof(S));
 memclr(&R, sizeof(R));
 memclr(&Y, sizeof(Y));
  memclr(&YY, sizeof(YY));




   return e; //success

}

//get voter pubkey for registrator
short srv_get_voter_pubkey(void)
{
 char path[512];
 char str[512];
 int pathlen=0;
 int i, l=0;
 short e=0;
 FILE * pFile;


 while(1)
 {

 //get path
  i=wai_getExecutablePath(NULL, 0, NULL);
  if(i<(sizeof(path)-64)) wai_getExecutablePath(path, i, &l);
  if(!l) path[0]=0;
  else for(i=l; i>=0; i--) if((path[i]==92)||(path[i]==47)) break;
  if(i) path[i]=0;
  pathlen=strlen(path);

  strncpy(path+pathlen, (char*)"/vot_data/vot.pub", sizeof(path)-pathlen); //set directory path
   pFile = fopen (path,"rt"); //try open for read
   if(!pFile) //not opened
   {
    printf("Error reg read vot pub\r\n");
    e=ERR_INIR_LOADYY;
    break;
   }
   memset(&rkeys->YY, 0, sizeof(rkeys->YY));
   str[0]=0;
   fgets(str, sizeof(str), pFile);
   l=str2bin(str, (unsigned char*)(&rkeys->YY), sizeof(rkeys->YY));
   fclose (pFile);

   if(l!=sizeof(rkeys->YY))
   {
    printf("Error vot pub length for reg\r\n");
    e=ERR_INIR_LENYY;
    break;
   }
   break;
 }
   
   memclr(path, sizeof(path));
   memclr(str, sizeof(str));
    #ifdef SRV_VLOG
   printf("Registrator loads voter's pubkey:  %s\r\n", srv_verb(e));
   #endif
   return e;
}


short srv_set_test(void)
{
  char *err = 0;
 //int i;
 short ret;
 short e=0;
 char str[526];

 if(!reg_db) return -1;


 sprintf(str, SQL_REG_ADD, "Ivanov I.I.", "1111111111", "1111");
 ret=sqlite3_exec(reg_db, str, 0, 0, &err);
 if (ret)
 {
  printf("Error SQL: %sn\r\n", err);
  sqlite3_free(err);
  e+=1;
 }

 sprintf(str, SQL_REG_ADD, "Petrov P.P.", "2222222222", "2222");
 ret=sqlite3_exec(reg_db, str, 0, 0, &err);
 if (ret)
 {
  printf("Error SQL: %sn\r\n", err);
  sqlite3_free(err);
  e+=2;
 }

 sprintf(str, SQL_REG_ADD, "Sidorov S.S.", "3333333333", "3333");
 ret=sqlite3_exec(reg_db, str, 0, 0, &err);
 if (ret)
 {
  printf("Error SQL: %sn\r\n", err);
  sqlite3_free(err);
  e+=4;
 }


 return e;
}
