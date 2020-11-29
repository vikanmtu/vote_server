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

#include "amath.h"
#include "shake.h"
#include "trr.h"

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

#include "websrv.h"
#include "webtcp.h"

int web_list=INVALID_SOCKET; //listening socket
int web_sock=INVALID_SOCKET; //incoming socket
short web_job=0;

unsigned char web_getbuf[GET_LEN];
short web_getlen=0;

#define TR_DBG 0

//close incoming connecting
void web_close(void)
{
 if(web_sock != INVALID_SOCKET) close(web_sock);
 web_sock=INVALID_SOCKET;
}

//initialize tcp listener on specified port
//return 0 if OK or eeror code
short web_tcp_init(unsigned short port, short wan)
{
   //struct timeval tt1; 
   struct sockaddr_in saddr;  //work address structure
   unsigned long opt = 1; //for ioctl
   int flag=1; //for setsockopt

   //open listener
  //CHECK LISTENER ALREADY EXIST AND CLOSE IT OR INITIALISE SOCKETS FIRST
  //create tcp listener if not exist: only once on application start
  if(web_list!=INVALID_SOCKET) //check for tcp listener already exist
  {
   close(web_list);   //close listener
   web_list=INVALID_SOCKET;   //set socket invalid
  }
  
  //CREATE TCP LISTENER AND BIND TO SPECIFIED INTERFACE

   //create tcp listener
   if((web_list = socket(AF_INET, SOCK_STREAM, 0)) <0)
   {
    perror("Web Listener");
    web_list=INVALID_SOCKET;
    return -1;
   }

    //set reuseable adressing
   if(setsockopt(web_list, SOL_SOCKET, SO_REUSEADDR, (char*)&flag, sizeof(flag)))
   {
    perror("Web Listener SO_REUSEADDR");
    close(web_list);
    web_list=INVALID_SOCKET;
    return -2;
   }
  
   //set address structure to listener interface
   memset(&saddr, 0, sizeof(saddr));
   saddr.sin_family = AF_INET;
   saddr.sin_port = htons(port);
   if(wan) saddr.sin_addr.s_addr=INADDR_ANY;
   else saddr.sin_addr.s_addr=INADDR_LOCAL;
  
    //bind listener
   if (bind(web_list, (struct sockaddr*)&saddr, sizeof(saddr)) < 0)
   {
    perror("Web Listener bind");
    close(web_list);
    web_list=INVALID_SOCKET;
    return -3;
   }
  
    //unblock listener socket
   opt=1;
   ioctl(web_list, FIONBIO, &opt);

   //set lestening mode for waiting 2 incoming at a time
   listen(web_list, 2);
  
   web_close();

   printf("Server listen WEB port %d\r\n", port);
   
 return 0;
}

//send answer len bytes or close connecting if len==0
//return 0 if OK or error code
short webtcp_send(unsigned char* buf, short len)
{
 short ret;
 //check for socket
 if(web_sock==INVALID_SOCKET)
 {
  if(TR_DBG) printf("WEB send nosoc\r\n");
  web_close();
  return -1;
 }

 if(!len)
 {
  ret=0;
  send(web_sock, (char*)&ret, sizeof(ret), MSG_NOSIGNAL);
  web_close();
  return ret;
 }

 //send data
 ret=send(web_sock, (char*)buf, len, MSG_NOSIGNAL); //send connectin event
 if(ret!=len) 
 {
  if(TR_DBG) printf("WEB send fail=%d-%d\r\n", len, ret);
  web_close();
  ret=-3;
 }
 if(TR_DBG) printf("WEB send OK=%d\r\n", ret);

 return ret;
}

//read incoming data in loop
//return 0 if no data or data length
short webtcp_read(unsigned char* buf, short max)
{
 
  struct sockaddr_in saddr;  //work address structure
  unsigned long opt=1;
  int sTemp=INVALID_SOCKET; //accepted socket
  int flag=1;
  int ll=sizeof(saddr);

  short len;


  //----------------------accept------------------------
  //check listen, return -1 if accepted
  if(web_list != INVALID_SOCKET)
  {
   sTemp  = accept(web_list, (struct sockaddr *) &saddr, (socklen_t*)&ll);
   if(sTemp>0) // incoming connections accepted
   {
    //disable nagle on sTemp ->Err


    if (setsockopt(sTemp, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag)) < 0)
    {
     perror( "TCPin TCP_NODELAY" );
     close(sTemp);
     return 0;
    }

    //unblock socket
    opt=1;
    ioctl(sTemp, FIONBIO, &opt);

    //check another client already connected, force disconnect it
    if(web_sock != INVALID_SOCKET)
    {
     len=websrv_busy(buf, max); //compose busy webpage
     if(len>0) len=webtcp_send(buf, len); //send busy webpage
     webtcp_send(buf, 0); //disconnect client
    }

    //set new socket
    web_close();
    web_sock=sTemp;
    web_getlen=0; //ckear received length for new client's GET request
    return -1; //accepted
   }  //end of sTemp>0
  } //(web_list != INVALID_SOCKET)

  //-----------------------rcvd--------------------
  //if websock check read
  if(web_sock != INVALID_SOCKET)
  {
    len=recv(web_sock, (char*)buf, max, 0);

    //connecting closed by remote
    if(!len)
    {
     web_close();
     return 0;
    }

    //some data received
    if(len>0)
    {
     //restrict data length to free space in GET buffer
     if(len > (GET_LEN-web_getlen)) len=GET_LEN-web_getlen;
     //check we have some data for  put to GET buffer
     if((len>0)&&(web_getlen<GET_LEN))
     {
      memcpy(web_getbuf+web_getlen, buf, len); //append GET buffer with received data
      web_getlen+=len; //add length of received data
      if(web_getlen>=GET_LEN) //check GET buffer sufficient
      {
       web_getbuf[GET_LEN-1]=0; //termonate string in GET buffer
       strncpy((char*)buf, (char*)web_getbuf, max); //output GET buffer
       return GET_LEN; //length of data GET buffer
      } //end of web_len>=GET_LEN
     } //end of start of packet
    } //end of some data was received
  }
  
 return 0;
} //end of webtcp_read


