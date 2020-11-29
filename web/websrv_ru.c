#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "convert.h"
#include "srv_webr.h"
#include "srv_webv.h"
#include "srv_inir.h"
#include "srv_iniv.h"

#define REQ_G 1
#define REQ_R 2
#define REQ_V 3


#define LINESR_PER_PAGE 20

unsigned char webr_req=0;
int webr_first=0;
int webr_cnt=0;


short webrsrv_busy(unsigned char* buf, short max)
{
 char str[256]; //utf8 string
 short n;

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

 win2utf(str, "Сервер тайного голосования", sizeof(str));
 snprintf((char*)buf+n, max-n, "<TITLE>%s</TITLE></HEAD><BODY>", str);
 n=strlen((char*)buf);

 win2utf(str, "Сервер перегружен, попробуйте еще раз!", sizeof(str));
 snprintf((char*)buf+n, max-n, "<FONT SIZE=6><P>%s</P></FONT></BODY></HTML>\r\n", str);
 n=strlen((char*)buf);

 return n;
}


short webrsrv_deny(unsigned char* buf, short max)
{
 char str[256]; //utf8 string
 short n;

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

 win2utf((char*)str, "Сервер тайного голосования", sizeof(str));
 snprintf((char*)buf+n, max-n, "<TITLE>%s</TITLE></HEAD><BODY>", str);
 n=strlen((char*)buf);

 win2utf(str, "WEB-интерфейс отключен администратором!", sizeof(str));
 snprintf((char*)buf+n, max-n, "<FONT SIZE=6><P>%s</P></FONT></BODY></HTML>\r\n", str);
 n=strlen((char*)buf);

 return n;
}


//process http request
//returns 0 for error or length of answer
short webrsrv_put(unsigned char* buf, short max, short len)
{
 short n=0;
 short i;
 char str[256]; //utf8 string
 int first;
 int last;

 //parse parameters of GET request
 webr_req=0;
 if(!len) return 0;
 if(buf[0]!=0x47) return 0; //not get
 if(len<6) webr_req=REQ_G;
 else if(buf[5]=='P') webr_req=REQ_R;
 else if(buf[5]=='K') webr_req=REQ_V;
 else if(buf[5]=='C') webr_req=REQ_G;
 if((len>=8)&&(buf[6]=='='))
 {
  webr_first=atoi((char*)buf+7)-1;
  if(webr_first<0) webr_first=0;
 }
 else webr_first=0;

 if(webr_req==REQ_G)
 {
  webr_cnt=12;  //show candidates
  webr_first=0;
 }
 else webr_cnt=LINESR_PER_PAGE; //show voters


 first=webr_first+1;
 last=first+19;

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

 if(webr_req==REQ_R)
 {
  win2utf(str, "Участники голосования", sizeof(str));

  snprintf((char*)buf+n, max-n, "<TITLE>%s</TITLE></HEAD><BODY>", str);
  n=strlen((char*)buf);

  snprintf((char*)buf+n, max-n, "<FONT SIZE=4><P>%s %d - %d :</P></FONT><TABLE BORDER CELLSPACING=1>", str, first, last);
  n=strlen((char*)buf);
 }
 else if(webr_req==REQ_V)
 {
  win2utf(str, "Результаты голосования", sizeof(str));

  snprintf((char*)buf+n, max-n, "<TITLE>%s</TITLE></HEAD><BODY>", str);
  n=strlen((char*)buf);

  //snprintf(buf+n, max-n, "<FONT SIZE=4><P>%s %d - %d :</P></FONT><TABLE BORDER CELLSPACING=1>", str, first, last);
  snprintf((char*)buf+n, max-n, "<FONT SIZE=4><P>%s %d - %d :</P><TABLE BORDER CELLSPACING=1>", str, first, last);
  n=strlen((char*)buf);
 }
 else //general
 {
  win2utf(str, "Сервер тайного голосования", sizeof(str));
  snprintf((char*)buf+n, max-n, "<TITLE>%s</TITLE></HEAD><BODY>", str);
  n=strlen((char*)buf);

  win2utf(str, "Приветствуем на сервере тайного голосования!", sizeof(str));
  snprintf((char*)buf+n, max-n, "<FONT SIZE=4><P>%s</P></FONT><hr>", str);
  n=strlen((char*)buf);

  win2utf(str, "Разработчик: студентка MHTУ им. академика Ю.Бугая Виктория Малеванченко, Полтава - 2020<hr>", sizeof(str));
  snprintf((char*)buf+n, max-n, "<P>%s</P>", str);
  n=strlen((char*)buf);

  i=srv_statr_get();
  i+=srv_statv_get(0);

  win2utf(str, "<P>Регистратор: избирателей-%d, приглашений-%d, регистраций-%d<P>", sizeof(str));
  snprintf((char*)buf+n, max-n, str, rstat->idn, rstat->psw, rstat->regs);
  n=strlen((char*)buf);
  win2utf(str, "<P>Учетчик: бюлетней-%d, голосовало-%d, раскрыло-%d<P><TABLE BORDER CELLSPACING=1>", sizeof(str));
  snprintf((char*)buf+n, max-n, str, vstat->key, vstat->vot, vstat->opn);
  n=strlen((char*)buf);

 }
 return n;
}

//get data after tcp_request will be answered 
//returns length of data or 0 if no data
short webrsrv_get(unsigned char* buf, int max)
{
 char str[256]; //utf8 string
 int n;
 short i;

 if(!webr_cnt) return 0;

 buf[0]=0; n=0;

 if(webr_req==REQ_R) //Registrator's DB
 {

  while(webr_cnt) //read up to specified number of entries
  {
   webr_cnt--; //count entries
   webr_first++; //set next entry

   i=srv_webr_get(webr_first); //read entry from DB
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
 else if(webr_req==REQ_V)  //Voter's DB
 {
  while(webr_cnt) //read up to specified number of entries
  {
   webr_cnt--; //count entries
   webr_first++; //set next entry

   i=srv_webv_get(webr_first); //read entry from DB
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
  while(webr_cnt)
  {
   webr_cnt--;
   webr_first++;
   if(webr_first>12) break;
   i=srv_statv_get(webr_first);
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
short webrsrv_end(unsigned char* buf, int max)
{
 short n=0;
 char str[256]; //utf8 string
 int first;
 int last;
  buf[0]=0;

 first=webr_first-39;
 if(first<1) first=1;
 last=webr_first+1;

 if(webr_req==REQ_R)
 {
  win2utf(str, "Назад", sizeof(str));
  snprintf((char*)buf+n,max-n, "</TABLE><P><A HREF=\"http://%s/P=%d\"><U><FONT SIZE=4 COLOR=\"#0000ff\">%s</U></FONT></A>\r\n", vkeys->onion, first, str);
  n=strlen((char*)buf);
  win2utf(str, "In English", sizeof(str));
  snprintf((char*)buf+n, max-n, "<A HREF=\"http://%s/R=%d\"><U><FONT SIZE=4 COLOR=\"#0000ff\">%s</U></FONT></A>\r\n", vkeys->onion, first+20, str);
  n=strlen((char*)buf);
  win2utf(str, "Вперед", sizeof(str));
  snprintf((char*)buf+n, max-n, "<A HREF=\"http://%s/P=%d\"><U><FONT SIZE=4 COLOR=\"#0000ff\">%s</U></FONT></A></P></BODY></HTML>\r\n", vkeys->onion, last, str);
  n=strlen((char*)buf);

 }
 else if(webr_req==REQ_V)
 {
  win2utf(str, "Назад", sizeof(str));
  //snprintf((char*)buf+n,max-n, "</TABLE><P><A HREF=\"http://%s/V=%d\"><U><FONT SIZE=4 COLOR=\"#0000ff\">%s</U></FONT></A>\r\n", vkeys->onion, first, str);
  snprintf((char*)buf+n,max-n, "</TABLE></FONT><P><A HREF=\"http://%s/K=%d\"><U><FONT SIZE=4 COLOR=\"#0000ff\">%s</U></FONT></A>\r\n", vkeys->onion, first, str);
  n=strlen((char*)buf);
  win2utf(str, "In English", sizeof(str));
  snprintf((char*)buf+n, max-n, "<A HREF=\"http://%s/V=%d\"><U><FONT SIZE=4 COLOR=\"#0000ff\">%s</U></FONT></A>\r\n", vkeys->onion, first+20, str);
  n=strlen((char*)buf);
  win2utf(str, "Вперед", sizeof(str));
  snprintf((char*)buf+n, max-n, "<A HREF=\"http://%s/K=%d\"><U><FONT SIZE=4 COLOR=\"#0000ff\">%s</U></FONT></A></P></BODY></HTML>\r\n", vkeys->onion, last, str);
  n=strlen((char*)buf);
 }
 else
 {
  win2utf(str, "</TABLE><hr>Вы можете посмотреть записи Регистратора: ФИО и идентификационный код участников голосования.", sizeof(str));
  snprintf((char*)buf+n, max-n, "<P>%s</P>", str);
  n=strlen((char*)buf);

  win2utf(str, "Флаги состояния: X - заблокирован, Q - выдано приглашение, I - идентифицирован, R - зарегистрирован.", sizeof(str));
  snprintf((char*)buf+n, max-n, "<P>%s</P>", str);
  n=strlen((char*)buf);

  win2utf(str, "Также вы можете посмотреть данные Учетчика: результаты голосования по анонимному номеру бюлетня.", sizeof(str));
  snprintf((char*)buf+n, max-n, "<P>%s</P>", str);
  n=strlen((char*)buf);

  win2utf(str, "Флаги состояния: X - ошибка, B - бюллетень получен, V - проголосовал, O - открыл свой голос.", sizeof(str));
  snprintf((char*)buf+n, max-n, "<P>%s</P>", str);
  n=strlen((char*)buf);

  win2utf(str, "Для доступа к записям Регистратора и Учетчика голосов перейдите по ссылкам:", sizeof(str));
  snprintf((char*)buf+n, max-n, "<P>%s</P>", str);
  n=strlen((char*)buf);

  win2utf(str, "Регистритор", sizeof(str));
  snprintf((char*)buf+n,max-n, "<P><A HREF=\"http://%s/P=0\"><U><FONT SIZE=4 COLOR=\"#0000ff\">%s</U></FONT></A>\r\n", vkeys->onion, str);
  n=strlen((char*)buf);
  win2utf(str, "Учетчик голосов", sizeof(str));
  snprintf((char*)buf+n, max-n, "<A HREF=\"http://%s/K=0\"><U><FONT SIZE=4 COLOR=\"#0000ff\">%s</U></FONT></A>\r\n", vkeys->onion, str);
  n=strlen((char*)buf);
  win2utf(str, "In English", sizeof(str));
  snprintf((char*)buf+n, max-n, "<A HREF=\"http://%s/G\"><U><FONT SIZE=4 COLOR=\"#0000ff\">%s</U></FONT></A></P></BODY></HTML>\r\n", vkeys->onion, str);
  n=strlen((char*)buf);
 }


 webr_req=0;
 webr_first=0;
 webr_cnt=0;
 return n;
}
