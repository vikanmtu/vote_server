#include "srv_inir.h"
#include "srv_webr.h"

//user public data
reg_web  srweb;
reg_web* rweb = &srweb;

//registrator's statistic
reg_stat srstat;
reg_stat* rstat = &srstat;

//DB request for for user by id
const char* SQL_SEL_WEBR = "SELECT * FROM Persons WHERE Idn=%d;";
//DB request for all users
const char* SQR_SEL_STATR = "SELECT * FROM Persons;";


//sql read callback
int srv_statr_cb(void* arg, int n, char** data ,char** name)
{
 if(n<6) return 0;
 rstat->all++; //count all entries in DB
 if(data[0] && data[0][0]) rstat->idn++; //count existed IDs
 if(data[1] && data[1][0]) rstat->fio++; //count existed FIO
 if(data[2] && data[2][0]) rstat->inn++; //cont existed INN
 if(data[3] && data[3][0]) rstat->psw++; //cound existed passwords
 if(data[4] && data[3][0]) rstat->idnt++; //count identified user's
 if(data[5] && data[3][0]) rstat->regs++; //count registered user's
 return 0;
}



//sql read callback
int srv_webr_cb(void* arg, int n, char** data ,char** name)
{
 if(n<6) return 0;

 if(data[0]) //check id
 {
  rweb->Idn=atoi(data[0]); //set finded id
  rweb->found=1; //set fiund flag
 }
 if(data[1]) strncpy(rweb->Fio, data[1], sizeof(rweb->Fio)); //output FIO
 if(data[2]) strncpy(rweb->Inn, data[2], sizeof(rweb->Inn)); //output INN
 //output flag (but not output secret data itself!!!)
 rweb->Flg='X'; //initial flag
 if(data[3] && data[3][0]) rweb->Flg='P'; //check password is set
 if(data[4] && data[4][0]) rweb->Flg='I'; //check shared secret is set
 if(data[5] &&data[5][0])  rweb->Flg='R'; //check signature is set
 return 0;
}


//Get voter's public data from registrator's DB
short srv_webr_get(int id)
{
 char str[256]; //string
 char *err = 0;
 short ret=0;

 memset(rweb, 0, sizeof(reg_web));
 
  //check db is open
  if(!reg_db)
  {
   printf("Error SQL: DB not opened\r\n");
   return -1;
  }

  //serch in DB by id
  sprintf(str, SQL_SEL_WEBR, id);
  ret=sqlite3_exec(reg_db, str, srv_webr_cb, 0, &err);
  if (ret) //db error
  {
   printf("Error SQL: %sn", err);
   sqlite3_free(err);
   return -2;
  }

  if(!rweb->found)  //not found
  {
   return -3;
  }

  //user finded: user's data is in rweb structure
  return 0;
}

 //get statistic from registrator's DB
short srv_statr_get(void)
{
 char str[256]; //string
 char *err = 0;
 short ret=0;

 //clear statistic
 memset(rstat, 0, sizeof(reg_stat));
 
  //check db is open
  if(!reg_db)
  {
   printf("Error SQL: DB not opened\r\n");
   return -1;
  }

  //serch in DB all entries
  sprintf(str, "SELECT * FROM Persons;"); //SQR_SEL_STATR
  ret=sqlite3_exec(reg_db, str, srv_statr_cb, 0, &err);
  if (ret) //db error
  {
   printf("Error SQL: %sn", err);
   sqlite3_free(err);
   return -2;
  }

  //DB search OK, Registrator's statistic is in rstat structure
  return 0;
}
