                       //client's data from DB
typedef struct
{
 int Idn;  //ID
 char Fio[128]; //FIO
 char Inn[16];  //INN
 char Flg;
 char found;
}  reg_web;

          //registrator's statistic
typedef struct
{
 int all; //total count of clients
 int idn; 
 int fio;
 int inn;
 int psw; //all with password
 int idnt; //identified
 int regs; //registered
}  reg_stat;

extern reg_web* rweb;
extern reg_stat* rstat;


//read public data of user with id from registrator's DB
//returns 0 if OK or error codes
//results are in struct rweb
short srv_webr_get(int id);

//read sttistic from registrator's DB
//returns 0 if OK or error codes
//results are in struct rstat
short srv_statr_get(void);
