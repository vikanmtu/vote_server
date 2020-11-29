#include "srv_iniv.h"

//returns answer length
short srv_gets(unsigned char* pkt)
{
 unsigned int crc=mtoi(pkt+GETS_RC); //packets crc
 unsigned char e=0; //error flag
 unsigned char r=0; //replay flag




 while(1) //loop for use break instead return
 {
  //check packet's type
  if(GETS!=pkt[2])
  {
   e=ERR_GETS_TYPE;
   break;
  }

 //check packet's length
  if(GETS_RL!=mtos(pkt))
  {
   e=ERR_GETS_LEN;
   break;
  }


 //check crc
  if(crc!=crc32_le(pkt, GETS_RC))
  {
   e=ERR_GETS_CRC;
   break; //no send any answer
  }

  break;
 }


   //clear input
   memclr(pkt, GETS_RL); //clear data

  //check there are no fatal errors
  if(!e)
  {
   memclr(pkt, GETS_AL); //clear data
   stom(pkt, GETS_AL);  //packet's len
   pkt[2]=GETS; //type
   pkt[3]=r; //note

   memcpy(pkt+GETS_AS, vkeys->rps, GETS_AS_LEN); //add signature
   memcpy(pkt+GETS_AV, &vkeys->XX, GETS_AV_LEN); //add voter's pubkey
  }

  if(e)//check for fatal error
  {
   memclr(pkt, GETS_AL); //clear data
   stom(pkt, GETS_AL);  //packet's len
   pkt[2]=GETS; //type
   pkt[3]=e+SRV_FATAL_ERR; //server's fatal error
  }

  //compute CRC
  crc=crc32_le(pkt, GETS_AC); //compute CRC
  itom(pkt+GETS_AC, crc);  //output
  #ifdef SRV_VLOG
  printf("Client request keys: %s  %s\r\n", srv_note(r), srv_verb(e));
  #else
  if(e) printf("Server: key request error %d \r\n", e);
  else printf("Server: anyone anonimously request keys\r\n");
  #endif

   //clear data
  crc=0;
  e=0;
  r=0;


  return GETS_AL;  //returns length of output
  
}
