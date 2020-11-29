 #include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "trr.h"
#include "srv_inir.h"
#include "srv_iniv.h"
#include "webtcp.h"
#include "websrv.h"

#define SRV_DBG 0

#ifdef _WIN32
#include <dirent.h>
#include <direct.h>
#include <windows.h>
#endif

#include "trr.h"

unsigned char srv_pkt[TCP_MAXLEN];  //work buffer
unsigned char srv_mode=0;           //mode flags
unsigned char web_buf[1400];   //web bufer



short tor_run(void)
{
 #define TOR_TCP_PORT 6543
 #define TOR_WEB_PORT 8000
 #define TOR_SOC_PORT 9056
 #define TOR_SERVER

 char path[512]={0,};
 char torcmd[1024];
 int i,j;
 int l;
 int pathlen;
 FILE * pFile;

  //kill old Tor process
  printf("Stop Tor...");
#ifdef _WIN32
  strcpy(torcmd, (char*)"taskkill /IM wt_tor.exe /F 2>NUL");
  psleep(1000);
#else
  //strcpy(torcmd, (char*)"killall -9 vt_tor > /dev/null");
  strcpy(torcmd, (char*)"pkill -f \"vt_tor\" > /dev/null");
#endif
  printf("\r\n----\r\n");
  system(torcmd);
  printf("\r\nRun Tor");

  for(i=0;i<5;i++)
  {
   printf(".");
   psleep(1000);
  }

 //get path
  i=wai_getExecutablePath(NULL, 0, NULL);
  if(i<(sizeof(path)-64)) wai_getExecutablePath(path, i, &l);
  if(!l) path[0]=0;
  else for(i=l; i>=0; i--) if((path[i]==92)||(path[i]==47)) break;
  if(i) path[i]=0;
  pathlen=strlen(path);

  //create registrator's data directory
  strncpy(path+pathlen, (char*)"/tor", sizeof(path)-pathlen); //set directory path
  #ifdef _WIN32
  mkdir(path);
  #else
  mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
  #endif


  strncpy(path+pathlen, (char*)"/tor/torrc", sizeof(path)-pathlen); //set directory path
  pFile = fopen(path, "r" );
  if(pFile)
  {
   fclose(pFile);
   pFile=0;
  }
  else
  {
    pFile = fopen(path, "w" ); //try write to torrc
    if(pFile)
    {
     strcpy(torcmd, (char*)"RunAsDaemon 1"); //demonize Tor after run
 #ifndef _WIN32
	fprintf(pFile, "%s\r\n", torcmd); //this options only for Linux
 #endif
     strcpy(torcmd, (char*)"DataDirectory "); //Work directory will be created by Tor
#ifdef _WIN32
     strncpy(path+pathlen, (char*)"\\tor\\tor_data", sizeof(path)-pathlen);
#else
     strncpy(path+pathlen, (char*)"/tor/tor_data", sizeof(path)-pathlen);
#endif
     strcpy(torcmd+strlen(torcmd), path);
     fprintf(pFile, "%s\r\n", torcmd);

#ifdef TOR_SERVER
      strcpy(torcmd, (char*)"HiddenServiceDir "); //Hidden service directore will be create by Tor
#ifdef _WIN32
      strncpy(path+pathlen, (char*)"\\tor\\hidden_service", sizeof(path)-pathlen);
#else
      strncpy(path+pathlen, (char*)"/tor/hidden_service", sizeof(path)-pathlen);
#endif
      strcpy(torcmd+strlen(torcmd), path);
      fprintf(pFile, "%s\r\n", torcmd);

      strcpy(torcmd, (char*)"HiddenServiceVersion 2"); //Version 2 of HS: short onion address with RCA key
      fprintf(pFile, "%s\r\n", torcmd);

      sprintf(torcmd, (char*)"HiddenServicePort %d", TOR_TCP_PORT);
      fprintf(pFile, "%s\r\n", torcmd);

      sprintf(torcmd, (char*)"HiddenServicePort 80 %d", TOR_WEB_PORT);
      fprintf(pFile, "%s\r\n", torcmd);    
 #endif
      sprintf(torcmd, (char*)"SocksPort %d", TOR_SOC_PORT);
      fprintf(pFile, "%s\r\n", torcmd);
      
     fclose (pFile);
     pFile=0;
    }
  }

  //set path to Tor executable
#ifdef _WIN32
   strncpy(path+pathlen, (char*)"\\tor\\wt_tor.exe -f ", sizeof(path)-pathlen);
#else
   strncpy(path+pathlen, (char*)"/tor/vt_tor -f ", sizeof(path)-pathlen);
#endif
   strcpy(torcmd, path);
   //add path to torrc as parameter
#ifdef _WIN32
   strncpy(path+pathlen, (char*)"\\tor\\torrc", sizeof(path)-pathlen);
#else
   strncpy(path+pathlen, (char*)"/tor/torrc", sizeof(path)-pathlen);
#endif
   strncpy(torcmd+strlen(torcmd), path, sizeof(torcmd)-strlen(torcmd));




    //run Tor in separated thread
#ifdef _WIN32
if(1) //use CreateProcess and wait 2 sec for Tor will be stable before next steps
{
   STARTUPINFO si;
   PROCESS_INFORMATION pi;
   memset(&si, 0, sizeof(si));
   si.cb = sizeof(si);
   //si.wShowWindow=SW_HIDE; 
   si.wShowWindow=SW_SHOWNORMAL;
   memset(&pi, 0, sizeof(pi));
   if( !CreateProcess( NULL,   // No module name (use command line)
        torcmd,        // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        0,              // Set handle inheritance to FALSE
        CREATE_NEW_CONSOLE,              // Creation flags
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory 
        &si,            // Pointer to STARTUPINFO structure
        &pi )           // Pointer to PROCESS_INFORMATION structure
    ) 
    printf( "CreateProcess failed (%d)\r\n", (int)GetLastError() ); 
    else WaitForSingleObject( pi.hProcess, 2000 );
  
}
#else   
   
    system(torcmd); //Linux: start Tor, wait for demonize

#endif

 //load onion address
 #ifdef _WIN32
 strncpy(path+pathlen, (char*)"\\tor\\hidden_service\\hostname", sizeof(path)-pathlen);
 #else
 strncpy(path+pathlen, (char*)"/tor/hidden_service/hostname", sizeof(path)-pathlen);
 #endif
 for(i=0;i<5;i++)
 {
  vkeys->onion[0]=0;
  l=0;
  pFile = fopen(path, "rt");
  if(pFile)
  {
   fgets(vkeys->onion, sizeof(vkeys->onion), pFile);
   fclose (pFile);
   pFile=0;
   l=strlen(vkeys->onion);
   for(j=0;j<l;j++) if(vkeys->onion[j]<' ') vkeys->onion[j]=0; //skip unprintables
   l=strlen(vkeys->onion);
   if(l==22) break;
   printf(".");
   psleep(1000); //1 sec delay
  }
 }
 if(l) printf(" DarkNet address: %s\r\n", vkeys->onion);
 else
 {
  vkeys->onion[0]=0;
  printf("Tor Error!\r\n");
 }
 printf("\r\n==========================Server runs==========================\r\n");
 return 0;
}


 //startup initialization
short srv_init(int argc, char* argv[])
{
  short i, j;
  short len;
  char* p;

  //initialize verbose info
  srv_verb_ini();

  //check command line parameters for allow or deny server's modes:
  if(argc>1)  //check parameters are specified in command line
  {
   srv_mode=0; //clear allowing flags
   for(i=1;i<argc;i++)  //parse all parameters passing over command line
   {
    p=argv[i]; //next parameter string
    len=strlen(p); //it's length
    for(j=0;j<len;j++) //parse all chars and set flags
    {
     if((p[j]=='r')||(p[j]=='R')) srv_mode |= SRV_ALLOW_REGS;
     else if((p[j]=='j')||(p[j]=='J')) srv_mode |= SRV_ALLOW_JOIN;
     else if((p[j]=='v')||(p[j]=='V')) srv_mode |= SRV_ALLOW_VOTE;
     else if((p[j]=='o')||(p[j]=='O')) srv_mode |= SRV_ALLOW_OPEN;
     else if((p[j]=='w')||(p[j]=='W')) srv_mode |= SRV_ALLOW_WEBS;
    }
   }
  }
  else srv_mode = SRV_ALLOW_REGS + SRV_ALLOW_JOIN + SRV_ALLOW_VOTE + SRV_ALLOW_OPEN + SRV_ALLOW_WEBS;

  //output working mode result
  printf("===============================================================\r\n");
  printf("Run in modes:\r\n-present public keys\r\n");
  if(srv_mode & SRV_ALLOW_REGS) printf("-named registration\r\n");
  if(srv_mode & SRV_ALLOW_JOIN) printf("-anonimity joining\r\n");
  if(srv_mode & SRV_ALLOW_VOTE) printf("-accept votes\r\n");
  if(srv_mode & SRV_ALLOW_OPEN) printf("-open votes\r\n");
  if(srv_mode & SRV_ALLOW_WEBS) printf("-web interface\r\n");
  printf("===============================================================\r\n\r\n");

  //initialize registrator's part
  i=srv_inir();
  if(i) return 0;

  //initialize voter's part
  i=srv_iniv();
  if(i) return 0;

  //registrator's part load voter's pubkey
  i=srv_get_voter_pubkey();
  if(i) return 0;

  //voter's part load registrator's pubkey
  i=srv_get_reg_pubkey();
  if(i) return 0;

  //initialize tcp engine (listening port)
  i=tcp_init(TCP_PORT, TCP_WAN);
  if(i) return 0;

  i=web_tcp_init(TOR_WEB_PORT, 1);
  if(i) return 0;


  //test: add 3 record's to registartor's database
 //i=srv_set_test();//!!!!!!
 return 1; //success
}



//server's infinite task
void srv_process(void)
{
 short i;

 //main loop
 while(1)
 {
  i=webtcp_read(web_buf, sizeof(web_buf)); //try receive data from web port
  if(i>0) //receive get request from webclient 
  {
   tcp_setjob();

   i=websrv_put(web_buf, sizeof(web_buf), i); //process data andd get answer's header
   if(!i) goto web_end; //incorrect GET

   //check web-interface is enabled
   if(!(srv_mode & SRV_ALLOW_WEBS))
   {
    i=websrv_deny(web_buf, sizeof(web_buf)); //interface disabled: get webpage
    if(i) i=webtcp_send(web_buf, i); //send answer
    goto web_end;
   }

   i=webtcp_send(web_buf, i); //send header
   if(!i) goto web_end; //socket error
   while(1)
   {
    i=websrv_get(web_buf, sizeof(web_buf)); //get data table
    if(!i) break; //no tables more
    i=webtcp_send(web_buf, i); //send if
    if(!i) goto web_end; //socket error
   }
   i=websrv_end(web_buf, sizeof(web_buf)); //get finalize data
   if(i) i=webtcp_send(web_buf, i); //send if exist
 web_end:
   webtcp_send(web_buf, 0); //close socket
   i=0;
  }
  else if(i<0) tcp_setjob(); //webclient connected 

  //try get incomonf TCP packet
  i=tcp_read(srv_pkt);
  //packets was received, process it
  if(i>TCP_HDR_LEN)
  {
   i=0;
   //process packet by it's type with checking mode allow
   switch(srv_pkt[2])
   {
    case GETS:
        if(SRV_DBG) printf("SRV process GETS\r\n");
        i=srv_gets(srv_pkt);
        break;
    case IDNT:
        if(!(srv_mode & SRV_ALLOW_REGS)) srv_pkt[2]=DENY;
        if(SRV_DBG) printf("SRV process IDNT\r\n");
        i=srv_idnt(srv_pkt);
        break;
    case REGS:
        if(!(srv_mode & SRV_ALLOW_REGS)) srv_pkt[2]=DENY;
        if(SRV_DBG) printf("SRV process REGS\r\n");
        i=srv_regs(srv_pkt);
        break;
    case JOIN:
        if(!(srv_mode & SRV_ALLOW_JOIN)) srv_pkt[2]=DENY;
        if(SRV_DBG) printf("SRV process JOIN\r\n");
        i=srv_join(srv_pkt);
        break;
    case VOTE:
        if(!(srv_mode & SRV_ALLOW_VOTE)) srv_pkt[2]=DENY;
        if(SRV_DBG) printf("SRV process VOTE\r\n");
        i=srv_vote(srv_pkt);
        break;
    case OPEN:
        if(!(srv_mode & SRV_ALLOW_OPEN)) srv_pkt[2]=DENY;
        if(SRV_DBG) printf("SRV process OPEN\r\n");
        i=srv_open(srv_pkt);
        break;
    default:
        if(SRV_DBG) printf("SRV bad pkt type=%d\r\n", srv_pkt[2]);
   }


   if(i>TCP_HDR_LEN)
   {
    if(SRV_DBG) printf("SRV send answer=%d\r\n", i);
    tcp_write(srv_pkt); //check and send answer to client
   }
   tcp_close(); //close current tcp session (only one request - answer for one session)
  } 



 }
 
}




