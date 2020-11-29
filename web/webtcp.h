#define GET_LEN 18


 #define TCP_MAXLEN 256 //maximal length of packet
  #define TCP_WAN 1      //acess server from WAN or only from Onion
  #define TCP_PORT 6543  //Listening TCP port
  #define TCP_TOUT 3     //in sec
  #define TCP_HDR_LEN 4  //bytes in packet's header
  #define TCP_SLEEP 10  //sleep time for tcp task in iddle in ms
  #define TCP_TIMECNT 200  //200 ms


#ifdef _WIN32

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <basetsd.h>
#include <stdint.h>
#include <winsock2.h>
#include <windows.h>
#include <time.h>

#define ioctl ioctlsocket
#ifndef close
//#define close closesocket
#endif
#define EWOULDBLOCK WSAEWOULDBLOCK //no data for assync polling
#define ENOTCONN WSAENOTCONN //wait for connection for assinc polling
#define ECONNRESET WSAECONNRESET //no remote udp interface in local network


#else //linux

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>

#include <stdint.h>
#include <stdarg.h>
#include <termios.h>
#include <unistd.h>
#include <sys/stat.h>

#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <linux/sockios.h>
#include <sys/ioctl.h>
#include <fcntl.h>
//#include <ifaddrs.h>

#endif


//some socket definitions
#ifndef INVALID_SOCKET
 #define INVALID_SOCKET -1
#endif

#ifndef SOCKET_ERROR
 #define SOCKET_ERROR -1
#endif

#ifndef INADDR_LOCAL
 #define INADDR_LOCAL 0x0100007F
#endif

#ifndef socklen_t
 #define socklen_t int
#endif

#ifndef MSG_NOSIGNAL
 #define MSG_NOSIGNAL 0
#endif

//initialize tcp listener on specified port
//return 0 if OK or eeror code
short web_tcp_init(unsigned short port, short wan);

//read incoming data in loop
//return 0 if no data or -1 if accepted or data length
short webtcp_read(unsigned char* buf, short max);

//send answer len bytes or close connecting if len==0
//return len if OK or negative error code
short webtcp_send(unsigned char* buf, short len);

