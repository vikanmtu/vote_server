#include "srv_iniv.h"
#include "srv_webv.h"

//voter's data
vot_web  svweb;
vot_web* vweb = &svweb;

//Calculator's statistic
vot_stat  svstat;
vot_stat* vstat = &svstat;

//DB request for voter by id (bulleten number)
const char* SQL_SEL_WEBV = "SELECT * FROM Voters WHERE Idn = %d;";
//DB request for all voters getting bulletens
const char* SQL_SEL_STATV = "SELECT * FROM Voters;";
//DB reuquest for specified candidate in voting result field
const char* SQL_SEL_CNDV = "SELECT * FROM Voters WHERE Vot = \"%s\";";

//sql read callback for statistic
int srv_statv_cb(void* arg, int n, char** data ,char** name)
{
 int i=0;
 if(n<5) return 0;

 vstat->all++; //total DB entries
 if(data[1] && data[1][0]) vstat->key++; //joined: giving bulletens
 if(data[2] && data[2][0]) vstat->vot++; //voted
 if(data[4]) i=atoi(data[4]); //tmr
 if(i==0x7FFFFFFF) vstat->opn++; //opened
 return 0;
}

//sql read callback for bulleten
int srv_webv_cb(void* arg, int n, char** data ,char** name)
{
 int i=0;

 if(n<5) return 0;

 //set default values
 strcpy(vweb->Vot, "-");
 vweb->Flg='X';

 //bulleten id
 if(data[0])
 {
  vweb->Idn=atoi(data[0]); //bulleten id
  vweb->found = 1; //set found flag
 }

 if(data[1]&&data[1][0]) vweb->Flg='B'; //having key (joined, has bulleten)
 if(data[2]&&data[2][0]) vweb->Flg='V'; //voted (non-opened and opened)
 if(data[4]) i=atoi(data[4]); //tmr
 if(i==0x7FFFFFFF) //opened
 {
  vweb->Flg='O'; //set flag
  if(data[2]) strncpy(vweb->Vot, data[2], sizeof(vweb->Vot));//copy opened vote
 }

 return 0;
}


//get public data from Calculator's DB by bulleten number (user's anonimous id)
//returns 0 is OK or error code
short srv_webv_get(int id)
{
 char str[256]; //string
 char *err = 0;
 short ret=0;

 //clear voter's public data before search
  memset(vweb, 0, sizeof(vot_web));
  
  //check db is open
  if(!vot_db)
  {
   printf("Error SQL: DB not opened\r\n");
   return -1;
  }

  //search in DB by id
  sprintf(str, SQL_SEL_WEBV, id);
  ret=sqlite3_exec(vot_db, str, srv_webv_cb, 0, &err);
  if (ret) //db error
  {
   printf("Error SQL: %sn", err);
   sqlite3_free(err);
   return -2;
  }

  //check entry was found
  if(!vweb->found)  //not found
  {
   return -3;
  }

  //bulleten findsd, data is in vweb structure
 return 0;
}

//get statistic from Calculator's DB
//if cnd is in range 1-12 get statistic for specified candidate
//otherwise get total statistic
//returns 0 is OK or error code
short srv_statv_get(int cnd)
{
 char str[256]; //string
 char *err = 0;
 short ret=0;

 //clear statistic before search
  memset(vstat, 0, sizeof(vot_stat));

  //check db is open
  if(!vot_db)
  {
   printf("Error SQL: DB not opened\r\n");
   return -1;
  }

  //check candidat specified in range 1-12
  if((cnd>=1)&&(cnd<=12))
  {
   vstat->iscnd=cnd; //number of candidate for search
   strncpy(vstat->cnd, vkeys->cnd + 16*(cnd-1), sizeof(vstat->cnd)); //copy càndidate name
   if(!vstat->cnd[0]) return -4; //check candidate is in list
   sprintf(str, SQL_SEL_CNDV, vstat->cnd); //SQL request only votes for this candidate
  }
  else sprintf(str, "SELECT * FROM Voters;"); //SQL_SEL_STATV: SQL request all entries

  //search in DB 
  ret=sqlite3_exec(vot_db, str, srv_statv_cb, 0, &err);
  if (ret) //db error
  {
   printf("Error SQL: %sn", err);
   sqlite3_free(err);
   return -2;
  }

  //DB serch OK, Calculator's statistic is in vstat structure
 return 0;
}

