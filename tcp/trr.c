 //**************************************************************************
//ORFone project
//Core of transport module
//**************************************************************************

#include <limits.h>
#include <stdio.h>

#ifdef _WIN32

#include <stddef.h>
#include <stdlib.h>
#include <basetsd.h>
#include <stdint.h>
#include <winsock2.h>
#include <windows.h>
#include <time.h>

#ifndef __BORLANDC__
 #include <ws2tcpip.h>
 #include <sys/time.h>
#endif

#define ioctl ioctlsocket
#define close closesocket
#define EWOULDBLOCK WSAEWOULDBLOCK  //no data for assync polling
#define ENOTCONN WSAENOTCONN        //wait for connection for assinc polling
#define ECONNRESET WSAECONNRESET    //no remote udp interface in local network



char sock_buf[32768];   //WSA sockets buffer
//------------------------------------------------------------------
//some Windows compilators not have gettimeofday and require own realization
//------------------------------------------------------------------
#ifndef gettimeofday
 int gettimeofday(struct timeval *tv, void* tz)
{

  FILETIME ft;
  const __int64 DELTA_EPOCH_IN_MICROSECS= 11644473600000000;
  unsigned __int64 tmpres = 0;
  unsigned __int64 tmpres_h = 0;
  //static int tzflag;

  if (NULL != tv)
  {
    GetSystemTimeAsFileTime(&ft);

    tmpres |= ft.dwHighDateTime;
    tmpres <<= 32;
    tmpres |= ft.dwLowDateTime;

    //converting file time to unix epoch
    tmpres /= 10;  //convert into microseconds
    tmpres -= DELTA_EPOCH_IN_MICROSECS;

    tmpres_h=tmpres / 1000000UL; //sec
    tv->tv_sec = (long)(tmpres_h);
    tv->tv_usec = (long)(tmpres % 1000000UL);
  }
  return 0;

}
#endif


#else //linux
#include <stdio.h>
#include <assert.h>
#include <ctype.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <netinet/tcp.h>
#include <netdb.h>

#ifdef LINUX_FPU_FIX
#include <fpu_control.h>
#endif

#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "string.h"
#include <linux/sockios.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <ifaddrs.h>

#endif


#ifndef INVALID_SOCKET
 #define INVALID_SOCKET -1
#endif

#ifndef SOCKET_ERROR
 #define SOCKET_ERROR -1
#endif

#include "amath.h"
#include "shake.h"
#include "trr.h"



unsigned char tcp_buf[TCP_MAXLEN];
short tcp_ptr=0; //bytes in buffer
short tcp_req=0; //bytes to be readed
unsigned int tcp_timenow=0; //last timestamp
unsigned int tcp_timecnt=TCP_TIMECNT; //counter for renew timestamp
unsigned int tcp_timeout=0;  //time for close socket (0 - not read)

int tcp_list=INVALID_SOCKET; //listening socket
int tcp_sock=INVALID_SOCKET; //incoming socket
short tcp_job=0; 

#define TR_DBG 0

//suspend excuting of the thread  for paus msec
void psleep(int paus)
{
 #ifdef _WIN32
    Sleep(paus);
 #else
    usleep(paus*1000);
 #endif
}
 
 //close incoming connecting
 void tcp_close(void)
{
 //if(!invalid) close, set invalid
 if(tcp_sock!=INVALID_SOCKET) 
 {
  close(tcp_sock);
  tcp_sock=INVALID_SOCKET; 
 }
  tcp_ptr=0; //clear receiving buffer
  tcp_req=0;
  tcp_timeout=0;
  memclr(tcp_buf, sizeof(tcp_buf));
  if(TR_DBG) printf("TCP was closed\r\n");
}

 //init tcp engine
//returns 0 if OK or negative error code 
 short tcp_init(unsigned short port, short wan)
 { 
 
   struct timeval tt1; 
   struct sockaddr_in saddr;  //work address structure
   unsigned long opt = 1; //for ioctl
   int flag=1; //for setsockopt
   
   
   
   //check for first start and inialize WinSocket (windows only)

  #ifdef _WIN32
    //Initializing WinSocks
   if (WSAStartup(0x202, (WSADATA *)&sock_buf[0]))
   {
    if(TR_DBG) printf("ti_init: WSAStartup error: %d\n", WSAGetLastError());
   }
  #endif


  //open listener
  //CHECK LISTENER ALREADY EXIST AND CLOSE IT OR INITIALISE SOCKETS FIRST
  //create tcp listener if not exist: only once on application start
  if(tcp_list!=INVALID_SOCKET) //check for tcp listener already exist
  {
   close(tcp_list);   //close listener
   tcp_list=INVALID_SOCKET;   //set socket invalid
  }
  
  //CREATE TCP LISTENER AND BIND TO SPECIFIED INTERFACE

   //create tcp listener
   if((tcp_list = socket(AF_INET, SOCK_STREAM, 0)) <0)
   {
    perror("Listener");
    tcp_list=INVALID_SOCKET;
    return -1;
   }

    //set reuseable adressing
   if(setsockopt(tcp_list, SOL_SOCKET, SO_REUSEADDR, (char*)&flag, sizeof(flag)))
   {
    perror("Listener SO_REUSEADDR");
    close(tcp_list);
    tcp_list=INVALID_SOCKET;
    return -2;
   }
  
   //set address structure to listener interface
   memset(&saddr, 0, sizeof(saddr));
   saddr.sin_family = AF_INET;
   saddr.sin_port = htons(port);
   if(wan) saddr.sin_addr.s_addr=INADDR_ANY;
   else saddr.sin_addr.s_addr=INADDR_LOCAL;
  
    //bind listener
   if (bind(tcp_list, (struct sockaddr*)&saddr, sizeof(saddr)) < 0)
   {
    perror("Listener bind");
    close(tcp_list);
    tcp_list=INVALID_SOCKET;
    return -3;
   }
  
    //unblock listener socket
   opt=1;
   ioctl(tcp_list, FIONBIO, &opt);

   //set lestening mode for waiting 2 incoming at a time
   listen(tcp_list, 2);
  
   tcp_close();
  
   gettimeofday(&tt1, NULL); //get timestamp
   tcp_timenow=(unsigned int) tt1.tv_sec; //set global value
   tcp_timecnt=TCP_TIMECNT; //init timeloop

   printf("Server listen TCP port %d\r\n", port);

   return 0;
} 
  

//poll for tcp packet
//returns positive length on packet is received
short tcp_read(unsigned char* pkt)
{
 struct timeval tt1;
 struct sockaddr_in saddr;  //work address structure
 unsigned long opt=1;
 int sTemp=INVALID_SOCKET; //accepted socket
 int flag=1; 
 int ll=sizeof(saddr);

 short len;
 short ret=0;
 
 //sleep on no job	  
	if(!tcp_job) psleep(TCP_SLEEP);
        tcp_job=0;

 if(tcp_list==INVALID_SOCKET) return -1;
 
 //check no incoming socket
 if(tcp_sock==INVALID_SOCKET) //no active incoming connecting: listen port
 {
     //try accept
   sTemp  = accept(tcp_list, (struct sockaddr *) &saddr, (socklen_t*)&ll);
   if(sTemp<=0) return 0; //no incoming connections accepted
   tcp_job=1;

   if(TR_DBG) printf("new TCP was accepted\r\n");

   //disable nagle on sTemp ->Err
  if (setsockopt(sTemp, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag)) < 0)
  {
   perror( "TCPin TCP_NODELAY" );
   close(sTemp);
   return -2;
  }

   //unblock socket
  opt=1;
  ioctl(sTemp, FIONBIO, &opt); 
  tcp_sock=sTemp;
    
  //set timeout
  gettimeofday(&tt1, NULL); //get timestamp
  tcp_timeout=TCP_TOUT+(unsigned int) tt1.tv_sec; //set global value
  
  
  tcp_ptr=0; //clear receiving buffer
  tcp_req=TCP_HDR_LEN; //set length of header for receive  
 }
 
 else  //incoming connecting active
 {
    if(tcp_timecnt) tcp_timecnt--;
	else
	{
	 gettimeofday(&tt1, NULL); //get timestamp
         tcp_timenow=(unsigned int) tt1.tv_sec; //set global value
	}
	
	//check tout: close, set invalid
	if(tcp_timeout && (tcp_timenow > tcp_timeout))
        {
         if(TR_DBG) printf("TCP timeout\r\n");
         tcp_close();
        }
	//if !req break
	if(!tcp_req) return 0;
	  
    len=recv(tcp_sock, (char*)tcp_buf+tcp_ptr, tcp_req, 0);  //try read to buf
    if(len<0) return 0;	//<0 break
    if((!len)||(len>tcp_req)) //0 close
    {
         if(TR_DBG) printf("TCP closed remotely\r\n");
	 tcp_close();
	 return 0;
    }
	
    tcp_job=1;

	//req-=len, ptr+=len
    tcp_ptr+=len;
    tcp_req-=len;		
	//req!=0  - break
    if(tcp_req) return 0;
		//check ptr==4
    if(tcp_ptr==TCP_HDR_LEN)
    {	

         if(TR_DBG) printf("TCP rcvd header\r\n");
          //get pktlen
		  len=mtos(tcp_buf);
		  //check pktlen
		  if((len<=TCP_HDR_LEN)||(len>=TCP_MAXLEN))
		  {
                   if(TR_DBG) printf("bad TCP header length=%d\r\n", len);
		   tcp_close();
	           return 0;
		  }
		  //set req=pktlen-4
		  tcp_req=len-TCP_HDR_LEN;
    }
    else //(ptr>4)
    {
      if(TR_DBG) printf("TCP rcvd pkt=%d\r\n", tcp_ptr);

      //memcpy data, ret=ptr
	  memcpy(pkt, tcp_buf, tcp_ptr);
	  ret=tcp_ptr;
    }	  
 }		  
	

	return ret;
}



//send tcp packet
//returns data length will be sended or error
short tcp_write(unsigned char* pkt)
{
 short len, ret;
 //check for socket
 if(tcp_sock==INVALID_SOCKET)
 {
  if(TR_DBG) printf("TCP send nosoc\r\n");
  tcp_close();
  return -1;
 }
 //check len
 len=mtos(pkt);
 if((len<=TCP_HDR_LEN)||(len>=TCP_MAXLEN))
 {
  if(TR_DBG) printf("TCP send badlen=%d\r\n", len);
  tcp_close();
  return -2;
 }
 //send data
 ret=send(tcp_sock, (char*)pkt, len, MSG_NOSIGNAL); //send connectin event
 if(ret!=len) 
 {
  if(TR_DBG) printf("TCP send fail=%d-%d\r\n", len, ret);
  tcp_close();
  ret=-3;
 }
 if(TR_DBG) printf("TCP send OK=%d\r\n", ret);
 return ret;
} 

 void tcp_setjob(void)
 {
  tcp_job=1;
 }




