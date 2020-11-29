                       //client's data from DB
typedef struct
{
 int Idn;  //ID
 char Vot[63]; //vote
 char Flg;
 char found;
}  vot_web;

          //registrator's statistic
typedef struct
{
 int iscnd; //request for candidate
 int all; //total count of clients
 int key; //joined
 int vot; //voted
 int opn; //opened
 char cnd[16]; //candidat
}  vot_stat;

extern vot_web* vweb;
extern vot_stat* vstat;

short srv_webv_get(int id);
short srv_statv_get(int cnd);
