 #include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "srv_iniv.h"

 #if defined _WIN32
#include <direct.h>
#elif defined __GNUC__
#include <sys/types.h>
#include <sys/stat.h>
#endif

vot_keys  svkeys;
vot_keys* vkeys = &svkeys;
vot_data svdata;
vot_data* vdata = &svdata;

sqlite3 *vot_db = 0;

const char* SQL_VOT_CREATE = "CREATE TABLE IF NOT EXISTS Voters (Idn INTEGER PRIMARY KEY ASC, Key varchar(63) NOT NULL UNIQUE, Vot varchar(63), Mac varchar(63), Tmr INTEGER );";

 //initialization of voter
short srv_iniv(void)
{
 char path[512];
 char str[512];
 int pathlen=0;
 char *err = 0;
 int i, j, n, l=0;
 short ret;
 FILE * pFile;
 short e=0;
 fp_t q;
 //fp_t r;
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
   e=ERR_INIV_RNG;
   break;
  }
  trng_get((unsigned char*)vkeys->x, sizeof(vkeys->x));
  cprng_init(vkeys->x, sizeof(vkeys->x));


  //get path
  i=wai_getExecutablePath(NULL, 0, NULL);
  if(i<(sizeof(path)-64)) wai_getExecutablePath(path, i, &l);
  if(!l) path[0]=0;
  else for(i=l; i>=0; i--) if((path[i]==92)||(path[i]==47)) break;
  if(i) path[i]=0;
  pathlen=strlen(path);

  //create voter's data directory
  strncpy(path+pathlen, (char*)"/vot_data", sizeof(path)-pathlen); //set directory path
  #ifdef _WIN32
  mkdir(path);
  #else
  mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
  #endif

  //load list of voters
  strncpy(path+pathlen, (char*)"/vot_data/vot.txt", sizeof(path)-pathlen); //set directory path
  pFile = fopen (path,"rt"); //try open for read
  if(pFile) //opened
  {
   n=0; //clear count
   for(i=0;i<12;i++)    //try loade up to 12 candidates
   {
    if(!fgets(str, sizeof(str), pFile)) break; //breck on eof
    l=strlen(str); //get string length
    if(l) //skip empty strings
    {
     for(j=0;j<l;j++) if((str[j]<0x20)&&(str[j]>=0)) str[j]=0; //remove unprintable chars
     l=strlen(str); //get new length
     if(l) //skip empty
     {
      strncpy(vkeys->cnd + n*16, str, 16); //add candidate to list
      n++; //count candidates
     }
    }
   }
   fclose (pFile); //close file
   pFile=0;
   printf("List of %d candidates loaded\r\n", n);
  }
  else  //no candidate file
  {
   pFile = fopen (path,"wt"); //try open for write
   if(!pFile) //not opened
   {
    printf("Error vot create candidates\r\n");
    e=ERR_INIV_SAVEXX;
    break;
   }
   fclose (pFile); //close empty file
  }

  //create DB if not exist
  strncpy(path+pathlen, (char*)"/vot_data/vot.db3", sizeof(path)-pathlen); //set directory path
  if( sqlite3_open("vot_data/vot.db3", &vot_db) )
  {
   printf("error of open/create vot DB: %s\n", sqlite3_errmsg(vot_db));
   e=ERR_INIV_DBOPEN;
   break;
  }

  //create table
  ret=sqlite3_exec(vot_db, SQL_VOT_CREATE, 0, 0, &err);
  if (ret)
  {
   printf("Error SQL: %sn", err);
   sqlite3_free(err);
   e=ERR_INIV_DBTAB;
   break;
  }
  //read voter's private key
  strncpy(path+pathlen, (char*)"/vot_data/vot.sec", sizeof(path)-pathlen); //set directory path
  l=0;
  pFile = fopen (path,"rt"); //try open for read
  if(pFile) //opened
  {
   str[0]=0;
   fgets(str, sizeof(str), pFile);
   fclose (pFile);
   l=str2bin(str, (unsigned char*)vkeys->x, sizeof(vkeys->x));
   memclr(str, sizeof(str));
   pFile=0;
   printf("Voter's secret key loaded\r\n");
  }



  //check secret key: create voter's keys , save
  if(l!=sizeof(vkeys->x))
  {
   cprng_get_bytes(vkeys->x, BI_BYTES); //generate new random secret key
   i=bls_key(&(vkeys->XX), &(vkeys->X), vkeys->x); //output: XX is G2 public key, X is G1 public key, x is private key
   if(!i)
   {
    printf("Error vot generate sec\r\n");
    e=ERR_INIV_GENX;
    break;
   }

   pFile = fopen (path,"wt"); //try open for write
   if(!pFile) //not opened
   {
    printf("Error vot save sec\r\n");
    e=ERR_INIV_SAVEX;
    break;
   }
   bin2str((unsigned char*)vkeys->x, str, sizeof(vkeys->x));
   fprintf(pFile, "%s\r\n", str);
   fclose (pFile);

   strncpy(path+pathlen, (char*)"/vot_data/vot.pub", sizeof(path)-pathlen); //set directory path
   pFile = fopen (path,"wt"); //try open for write
   if(!pFile) //not opened
   {
    printf("Error vot save sec\r\n");
    e=ERR_INIV_SAVEXX;
    break;
   }
   bin2str((unsigned char*)&vkeys->XX, str, sizeof(vkeys->XX));
   fprintf(pFile, "%s\r\n", str);
   fclose (pFile);


   printf("Voter's keys created\r\n");
  }

  else  //key secret is loaded success
  {
   i=bls_key(&(vkeys->XX), &(vkeys->X), vkeys->x); //output: XX is G2 public key, X is G1 public key, x is private key
   if(!i)
   {
    printf("Error vot loaded sec\r\n");
    e=ERR_INIV_GENXX;
    break;
   }
  }


  bls_compress(vkeys->p, &vkeys->X); //compress Q1 public key


   //read voter'ss pubkey
   strncpy(path+pathlen, (char*)"/vot_data/vot.pub", sizeof(path)-pathlen); //set directory path
   pFile = fopen (path,"rt"); //try open for read
   if(!pFile) //not opened
   {
    printf("Error vot read pub\r\n");
    e=ERR_INIV_LOADXX;
    break;
   }

   memset(&YY, 0, sizeof(YY));
   str[0]=0;
   fgets(str, sizeof(str), pFile);
   l=str2bin(str, (unsigned char*)(&YY), sizeof(YY));
   fclose (pFile);

   if(l!=sizeof(YY))
   {
    printf("Error vot pub length\r\n");
    e=ERR_INIV_LENXX;
    break;
   }

   i=sizeof(YY);
    i=isequal((unsigned char*)&vkeys->XX, (unsigned char*)&YY, sizeof(YY));
    if(!i)
    {
    printf("Error vot pub\r\n");
    e=ERR_INIV_CHECKXX;
    break;
   }

  //test of blind signature of random message
    cprng_get_bytes(q, BI_BYTES); //generate random message 32 bytes
    bls_hash(&M, q); //hash to point

    cprng_get_bytes(q, BI_BYTES); //generate random blind value
    bls_blind(&B, &M, q); //blind message point to M

    bls_compress(b, &B); //compress blinded point to 32 bytes value
    bls_uncompress(&D, b); //decompress blinded point to D

    bls_sign(&S, &B, vkeys->x); //bling signing

    bls_uncompress(&Y, vkeys->p); //uncompress Q1 key
    bls_unblind(&R, &S, &Y, q); //unbling signature to R
    i=bls_verify(&M, &R, &vkeys->XX); //verify
    if(!i)
    {
     e=ERR_INIV_TEST;
     break;
    }

    break;
  }

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
    #ifdef SRV_VLOG
   printf("Voter initialized:  %s\r\n", srv_verb(e));
  #endif
    return e; //success

}

//get registrator's pubkey for voter
short srv_get_reg_pubkey(void)
{
 char path[512];
 char str[512];
 int pathlen=0;
 int i, l=0;

 FILE * pFile;

 fp_t q;
 ecpoint_fp M;
 ecpoint_fp S;
 short e=0;

 //get path
  i=wai_getExecutablePath(NULL, 0, NULL);
  if(i<(sizeof(path)-64)) wai_getExecutablePath(path, i, &l);
  if(!l) path[0]=0;
  else for(i=l; i>=0; i--) if((path[i]==92)||(path[i]==47)) break;
  if(i) path[i]=0;
  pathlen=strlen(path);


  while(1)
  {
   strncpy(path+pathlen, (char*)"/reg_data/reg.pub", sizeof(path)-pathlen); //set directory path
   pFile = fopen (path,"rt"); //try open for read
   if(!pFile) //not opened
   {
    printf("Error vot read reg pub\r\n");
    e=ERR_INIV_LOADYY;
    break;
   }

   //read registrator's pubkey from file
   str[0]=0;
   fgets(str, sizeof(str), pFile);
   l=str2bin(str, (unsigned char*)(&vkeys->YY), sizeof(vkeys->YY));
   fclose (pFile);

   if(l!=sizeof(vkeys->YY))
   {
    printf("Error reg pub length for vot\r\n");
    e=ERR_INIV_LENYY;
    break;
   }

   //hash registration pk
   sh_ini();
   sh_upd(&vkeys->YY, FP2_PK_LEN);
   sh_xof();
   sh_out(q, sizeof(q)); //hash key[16] to fp[32] value
   i=bls_hash(&M, q); //hash to poin M

   if(!i) //point invalid
   {
    printf("Invalid point!"); //set error verbose
    e=ERR_INIV_HASHYY;
    break;
   }

   bls_sign(&S, &M, vkeys->x); //sign
   bls_compress(vkeys->rps, &S); //compress signature

   break;
 }
   memclr(&M, sizeof(M));
   memclr(&S, sizeof(S));
   memclr(q, sizeof(q));
   memclr(path, sizeof(path));
   memclr(str, sizeof(str));

    #ifdef SRV_VLOG
   printf("Voter load registrator's pubkey:  %s\r\n", srv_verb(e));
   #endif
   return e;
}

