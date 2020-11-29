#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "convert.h"
#include "srv_webr.h"
#include "srv_webv.h"
#include "srv_inir.h"
#include "srv_iniv.h"
#include "websrv_ru.h"

#define REQ_G 1
#define REQ_R 2
#define REQ_V 3


#define LINES_PER_PAGE 20

unsigned char web_req=0; //type of request (server, registrator or calculator)
int web_first=0; //requested first database line
int web_cnt=0; //counters of database leines
int ru=0; //flag of russian lang


short websrv_busy(unsigned char* buf, short max)
{
 char str[256]; //utf8 string
 short n;

 if(ru)
 {
  n=webrsrv_busy(buf, max);
  return n;
 }

 //compose header
 buf[0]=0; n=0;

 snprintf((char*)buf+n, max-n, "HTTP/1.1 200 OK\r\n");
 n=strlen((char*)buf);
 snprintf((char*)buf+n, max-n, "Server: Vote\r\n");
 n=strlen((char*)buf);
 snprintf((char*)buf+n, max-n, "Connection: close\r\n");
 n=strlen((char*)buf);
 snprintf((char*)buf+n, max-n, "Content-Type:text/html;charset=utf-8\r\n\r\n");
 n=strlen((char*)buf);

 snprintf((char*)buf+n, max-n, "<HTML><HEAD><META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html; charset=UTF-8\">\r\n");
 n=strlen((char*)buf);

 win2utf(str, "Voting server", sizeof(str));
 snprintf((char*)buf+n, max-n, "<TITLE>%s</TITLE></HEAD><BODY>", str);
 n=strlen((char*)buf);

 win2utf(str, "Server is overloaded, try again!", sizeof(str));
 snprintf((char*)buf+n, max-n, "<FONT SIZE=6><P>%s</P></FONT></BODY></HTML>\r\n", str);
 n=strlen((char*)buf);

 return n;
}


short websrv_deny(unsigned char* buf, short max)
{
 char str[256]; //utf8 string
 short n;

 if(ru)
 {
  n=webrsrv_deny(buf, max);
  return n;
 }

 //compose header
 buf[0]=0; n=0;

 snprintf((char*)buf+n, max-n, "HTTP/1.1 200 OK\r\n");
 n=strlen((char*)buf);
 snprintf((char*)buf+n, max-n, "Server: Vote\r\n");
 n=strlen((char*)buf);
 snprintf((char*)buf+n, max-n, "Connection: close\r\n");
 n=strlen((char*)buf);
 snprintf((char*)buf+n, max-n, "Content-Type:text/html;charset=utf-8\r\n\r\n");
 n=strlen((char*)buf);

 snprintf((char*)buf+n, max-n, "<HTML><HEAD><META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html; charset=UTF-8\">\r\n");
 n=strlen((char*)buf);

 win2utf((char*)str, "Voting server", sizeof(str));
 snprintf((char*)buf+n, max-n, "<TITLE>%s</TITLE></HEAD><BODY>", str);
 n=strlen((char*)buf);

 win2utf(str, "WEB-interface is disabled by admin!", sizeof(str));
 snprintf((char*)buf+n, max-n, "<FONT SIZE=6><P>%s</P></FONT></BODY></HTML>\r\n", str);
 n=strlen((char*)buf);

 return n;
}


//process http request
//returns 0 for error or length of answer
short websrv_put(unsigned char* buf, short max, short len)
{
 short n=0;
 short i;
 char str[256]; //utf8 string
 int first;
 int last;

 //chack lang
 ru=0;
 if(len>=6)
 {
  if( (buf[5]=='P') || (buf[5]=='K') || (buf[5]=='C')) ru=1;
 }

 //call russian version if requested
 if(ru)
 {
  web_req=0;
  i=webrsrv_put(buf, max, len);
  return i;
 } 

 //parse parameters of GET request
 web_req=0;
 if(!len) return 0;
 if(buf[0]!=0x47) return 0; //not get
 if(len<6) web_req=REQ_G;
 else if(buf[5]=='R') web_req=REQ_R;
 else if(buf[5]=='V') web_req=REQ_V;
 else web_req=REQ_G;

 if((len>=8)&&(buf[6]=='='))
 {
  web_first=atoi((char*)buf+7)-1;
  if(web_first<0) web_first=0;
 }
 else web_first=0;

 if(web_req==REQ_G)
 {
  web_cnt=12;  //show candidates
  web_first=0;
 }
 else web_cnt=LINES_PER_PAGE; //show voters


 first=web_first+1;
 last=first+19;
 //web_req=REQ_R; web_cnt=20; web_first = 0; //test!!!

 //compose header
 buf[0]=0; n=0;

 snprintf((char*)buf+n, max-n, "HTTP/1.1 200 OK\r\n");
 n=strlen((char*)buf);
 snprintf((char*)buf+n, max-n, "Server: Vote\r\n");
 n=strlen((char*)buf);
 snprintf((char*)buf+n, max-n, "Connection: close\r\n");
 n=strlen((char*)buf);
 snprintf((char*)buf+n, max-n, "Content-Type:text/html;charset=utf-8\r\n\r\n");
 n=strlen((char*)buf);

 snprintf((char*)buf+n, max-n, "<HTML><HEAD><META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html; charset=UTF-8\">\r\n");
 n=strlen((char*)buf);

 if(web_req==REQ_R)
 {
  win2utf(str, "Registrator", sizeof(str));

  snprintf((char*)buf+n, max-n, "<TITLE>%s</TITLE></HEAD><BODY>", str);
  n=strlen((char*)buf);

  snprintf((char*)buf+n, max-n, "<FONT SIZE=4><P>%s %d - %d :</P></FONT><TABLE BORDER CELLSPACING=1>", str, first, last);
  n=strlen((char*)buf);
 }
 else if(web_req==REQ_V)
 {
  win2utf(str, "Calculator", sizeof(str));

  snprintf((char*)buf+n, max-n, "<TITLE>%s</TITLE></HEAD><BODY>", str);
  n=strlen((char*)buf);

  //snprintf(buf+n, max-n, "<FONT SIZE=4><P>%s %d - %d :</P></FONT><TABLE BORDER CELLSPACING=1>", str, first, last);
  snprintf((char*)buf+n, max-n, "<FONT SIZE=4><P>%s %d - %d :</P><TABLE BORDER CELLSPACING=1>", str, first, last);
  n=strlen((char*)buf);
 }
 else //general
 {
  win2utf(str, "Voting server", sizeof(str));
  snprintf((char*)buf+n, max-n, "<TITLE>%s</TITLE></HEAD><BODY>", str);
  n=strlen((char*)buf);

  win2utf(str, "Welcome to the voting server!", sizeof(str));
  snprintf((char*)buf+n, max-n, "<FONT SIZE=4><P>%s</P></FONT><hr>", str);
  n=strlen((char*)buf);

  win2utf(str, "Developer: Viktoria Malevanchenko, student of MNTU named  acad. Yu.Bugai, Ukraine, Poltava - 2020<hr>", sizeof(str));
  snprintf((char*)buf+n, max-n, "<P>%s</P>", str);
  n=strlen((char*)buf);

  i=srv_statr_get();
  i+=srv_statv_get(0);

  win2utf(str, "<P>Registrator: voters-%d, invited-%d, registered-%d<P>", sizeof(str));
  snprintf((char*)buf+n, max-n, str, rstat->idn, rstat->psw, rstat->regs);
  n=strlen((char*)buf);
  win2utf(str, "<P>Calculator: ballots-%d, voted-%d, opened-%d<P><TABLE BORDER CELLSPACING=1>", sizeof(str));
  snprintf((char*)buf+n, max-n, str, vstat->key, vstat->vot, vstat->opn);
  n=strlen((char*)buf);

 }
 return n;
}

//get data after tcp_request will be answered 
//returns length of data or 0 if no data
short websrv_get(unsigned char* buf, int max)
{
 char str[256]; //utf8 string
 int n;
 short i;

 if(ru)
 {
  i=webrsrv_get(buf, max);
  return i;
 }

 if(!web_cnt) return 0;

 buf[0]=0; n=0;

 if(web_req==REQ_R) //Registrator's DB
 {

  while(web_cnt) //read up to specified number of entries
  {
   web_cnt--; //count entries
   web_first++; //set next entry

   i=srv_webr_get(web_first); //read entry from DB
   if(i) continue;  //to first valid first valid entry

   snprintf((char*)buf+n, max-n, "<TR><TD WIDTH=\"14%%\" VALIGN=\"MIDDLE\"><P>%d</TD>", rweb->Idn);
   n=strlen((char*)buf);

   win2utf(str, rweb->Fio, 32);
   snprintf((char*)buf+n, max-n, "<TD WIDTH=\"61%%\" VALIGN=\"MIDDLE\"><P>%s</TD>", str);
   n=strlen((char*)buf);

   win2utf(str, rweb->Inn, 32);
   snprintf((char*)buf+n, max-n, "<TD WIDTH=\"18%%\" VALIGN=\"MIDDLE\"><P>%s</TD>", str);
   n=strlen((char*)buf);

   snprintf((char*)buf+n, max-n, "<TD WIDTH=\"8%%\" VALIGN=\"MIDDLE\"><P>%c</TD></TR>", rweb->Flg);
   n=strlen((char*)buf);
   break;  //if entry readed success
  }

 }
 else if(web_req==REQ_V)  //Voter's DB
 {
  while(web_cnt) //read up to specified number of entries
  {
   web_cnt--; //count entries
   web_first++; //set next entry

   i=srv_webv_get(web_first); //read entry from DB
   if(i) continue;  //to first valid first valid entry

   snprintf((char*)buf+n, max-n, "<TR><TD WIDTH=\"25%%\" VALIGN=\"MIDDLE\"><P>%d</TD>", vweb->Idn);
   n=strlen((char*)buf);

   win2utf(str, vweb->Vot, 32);
   snprintf((char*)buf+n, max-n, "<TD WIDTH=\"65%%\" VALIGN=\"MIDDLE\"><P>%s</TD>", str);
   n=strlen((char*)buf);

   snprintf((char*)buf+n, max-n, "<TD WIDTH=\"10%%\" VALIGN=\"MIDDLE\"><P>%c</TD></TR>", vweb->Flg);
   n=strlen((char*)buf);
   break;
  }
 }
 else  //Candidates
 {
  while(web_cnt)
  {
   web_cnt--;
   web_first++;
   if(web_first>12) break;
   i=srv_statv_get(web_first);
   if(i) continue;
   win2utf(str, vstat->cnd, 32);
   snprintf((char*)buf+n, max-n, "<TR><TD WIDTH=\"80%%\" VALIGN=\"MIDDLE\"><P>%s</TD>", str);
   n=strlen((char*)buf);
   snprintf((char*)buf+n, max-n, "<TD WIDTH=\"20%%\" VALIGN=\"MIDDLE\"><P>%d</TD></TR>", vstat->all);
   n=strlen((char*)buf);
   break;
  }
 }

 return n;
}

//return final answer
short websrv_end(unsigned char* buf, int max)
{
 short n=0;
 char str[256]; //utf8 string
 int first;
 int last;
  buf[0]=0;

 if(ru)
 {
  n=webrsrv_end(buf, max);
  return n;
 }

 first=web_first-39;
 if(first<1) first=1;
 last=web_first+1;

 if(web_req==REQ_R)
 {
  win2utf(str, " Previous ", sizeof(str));
  snprintf((char*)buf+n,max-n, "</TABLE><P><A HREF=\"http://%s/R=%d\"><U><FONT SIZE=4 COLOR=\"#0000ff\">%s</U></FONT></A>\r\n", vkeys->onion, first, str);
  n=strlen((char*)buf);
  win2utf(str, " На русском ", sizeof(str));
  snprintf((char*)buf+n, max-n, "<A HREF=\"http://%s/P=%d\"><U><FONT SIZE=4 COLOR=\"#0000ff\">%s</U></FONT></A>\r\n", vkeys->onion, last, str);
  n=strlen((char*)buf);
  win2utf(str, " Next ", sizeof(str));
  snprintf((char*)buf+n, max-n, "<A HREF=\"http://%s/R=%d\"><U><FONT SIZE=4 COLOR=\"#0000ff\">%s</U></FONT></A></P></BODY></HTML>\r\n", vkeys->onion, last, str);
  n=strlen((char*)buf);
 }
 else if(web_req==REQ_V)
 {
  win2utf(str, " Previous ", sizeof(str));
  //snprintf((char*)buf+n,max-n, "</TABLE><P><A HREF=\"http://%s/V=%d\"><U><FONT SIZE=4 COLOR=\"#0000ff\">%s</U></FONT></A>\r\n", vkeys->onion, first, str);
  snprintf((char*)buf+n,max-n, "</TABLE></FONT><P><A HREF=\"http://%s/V=%d\"><U><FONT SIZE=4 COLOR=\"#0000ff\">%s</U></FONT></A>\r\n", vkeys->onion, first, str);
  n=strlen((char*)buf);
  win2utf(str, " На русском ", sizeof(str));
  snprintf((char*)buf+n, max-n, "<A HREF=\"http://%s/K=%d\"><U><FONT SIZE=4 COLOR=\"#0000ff\">%s</U></FONT></A>\r\n", vkeys->onion, first+20, str);
  n=strlen((char*)buf);
  win2utf(str, " Next  ", sizeof(str));
  snprintf((char*)buf+n, max-n, "<A HREF=\"http://%s/V=%d\"><U><FONT SIZE=4 COLOR=\"#0000ff\">%s</U></FONT></A></P></BODY></HTML>\r\n", vkeys->onion, last, str);
  n=strlen((char*)buf);
 }
 else
 {
  win2utf(str, "</TABLE><hr>You can check registrator's data: names and ITINs.", sizeof(str));
  snprintf((char*)buf+n, max-n, "<P>%s</P>", str);
  n=strlen((char*)buf);

  win2utf(str, "Flags of state: X - blocked, Q - invited, I - identified, R - registered.", sizeof(str));
  snprintf((char*)buf+n, max-n, "<P>%s</P>", str);
  n=strlen((char*)buf);

  win2utf(str, "Also you can check calculator's data: voting results by ballot's ID.", sizeof(str));
  snprintf((char*)buf+n, max-n, "<P>%s</P>", str);
  n=strlen((char*)buf);

  win2utf(str, "Flag of state: X - error, B - ballot issued, V - voted, O - opened.", sizeof(str));
  snprintf((char*)buf+n, max-n, "<P>%s</P>", str);
  n=strlen((char*)buf);

  win2utf(str, "To access the Registrator's and Calculator's database records follow links:", sizeof(str));
  snprintf((char*)buf+n, max-n, "<P>%s</P>", str);
  n=strlen((char*)buf);

  win2utf(str, " Registrator ", sizeof(str));
  snprintf((char*)buf+n,max-n, "<P><A HREF=\"http://%s/R=0\"><U><FONT SIZE=4 COLOR=\"#0000ff\">%s</U></FONT></A>\r\n", vkeys->onion, str);
  n=strlen((char*)buf);
  win2utf(str, " Calculator ", sizeof(str));
  snprintf((char*)buf+n, max-n, "<A HREF=\"http://%s/V=0\"><U><FONT SIZE=4 COLOR=\"#0000ff\">%s</U></FONT></A>\r\n", vkeys->onion, str);
  n=strlen((char*)buf);
  win2utf(str, " На русском ", sizeof(str));
  snprintf((char*)buf+n, max-n, "<A HREF=\"http://%s/C\"><U><FONT SIZE=4 COLOR=\"#0000ff\">%s</U></FONT></A></P></BODY></HTML>\r\n", vkeys->onion, str);
  n=strlen((char*)buf);
 }


 web_req=0;
 web_first=0;
 web_cnt=0;
 return n;
}
