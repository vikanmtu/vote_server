#include <string.h>
#include "convert.h"



static const char *conv_Dos2Koi="áâ÷çäåöúéêëìíîïğòóôõæèãşûıÿùøüàñÁÂ×ÇÄÅÖÚÉÊËÌÍÎÏĞ°±²³´µ¶·¸¹º»¼½¾¿ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏĞÑÒÓÔÕÖ×ØÙÚÛÜİŞßÒÓÔÕÆÈÃŞÛİßÙØÜÀÑ³£òóôõö÷øùúûüışÿ";
static const char *conv_Dos2Win="ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏĞÑÒÓÔÕÖ×ØÙÚÛÜİŞßàáâãäåæçèéêëìíîï°±²³´µ¶·¸¹º»¼½¾¿ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏĞÑÒÓÔÕÖ×ØÙÚÛÜİŞßğñòóôõö÷øùúûüışÿ¨¸òóôõö÷øùúûüışÿ";
static const char *conv_Koi2Dos="€‚ƒ„…†‡ˆ‰Š‹Œ‘’“”•–—˜™š›œŸ ¡¢ñ¤¥¦§¨©ª«¬­®¯°±²ğ´µ¶·¸¹º»¼½¾¿î ¡æ¤¥ä£å¨©ª«¬­®¯ïàáâã¦¢ìë§èíéçê€–„…”ƒ•ˆ‰Š‹ŒŸ‘’“†‚œ›‡˜™—š";
static const char *conv_Koi2Win="€‚ƒ„…†‡ˆ‰Š‹Œ‘’“”•–—˜™š›œŸ ¡¢¸¤¥¦§¨©ª«¬­®¯°±²¨´µ¶·¸¹º»¼½¾¿şàáöäåôãõèéêëìíîïÿğñòóæâüûçøıù÷úŞÀÁÖÄÅÔÃÕÈÉÊËÌÍÎÏßĞÑÒÓÆÂÜÛÇØİÙ×Ú";
static const char *conv_Win2Dos="€‚ƒ„…†‡ˆ‰Š‹Œ‘’“”•–—˜™š›œŸ ¡¢£¤¥¦§ğ©ª«¬­®¯°±²³´µ¶·ñ¹º»¼½¾¿€‚ƒ„…†‡ˆ‰Š‹Œ‘’“”•–—˜™š›œŸ ¡¢£¤¥¦§¨©ª«¬­®¯àáâãäåæçèéêëìíîï";
static const char *conv_Win2Koi="€‚ƒ„…†‡ˆ‰Š‹Œ‘’“”•–—˜™š›œŸ ¡¢£¤¥¦§³©ª«¬­®¯°±²³´µ¶·£¹º»¼½¾¿áâ÷çäåöúéêëìíîïğòóôõæèãşûıÿùøüàñÁÂ×ÇÄÅÖÚÉÊËÌÍÎÏĞÒÓÔÕÆÈÃŞÛİßÙØÜÀÑ";
static const char tbl_win[72]={
     -32, -31, -30, -29, -28, -27, -26, -25,
     -24, -23, -22, -21, -20, -19, -18, -17,
     -16, -15, -14, -13, -12, -11, -10, -9,
     -8, -7, -4, -5, -6, -3, -2, -1,
     -64, -63, -62, -61, -60, -59, -58, -57,
     -56, -55, -54, -53, -52, -51, -50, -49,
     -48, -47, -46, -45, -44, -43, -42, -41,
     -40, -39, -36, -37, -38, -35, -34, -33,
     -78, -77, -81, -65, -86, -70, -88, -72
 };
static const char* tbl_trn="abvgdezzijklmnoprstufhccss'y'ejjABVGDEZZIIKLMNOPRSTUFHCCSS'I'EJJIiYyYyYy";
static const unsigned short unescape[64] = {
      0x0402, 0x0403, 0x201A, 0x0453, 0x201E, 0x2026, 0x2020, 0x2021, 0x20AC, 0x2030, 0x0409, 0x2039, 0x040A, 0x040C, 0x040B, 0x040F,
      0x0452, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014, 0x0000, 0x2122, 0x0459, 0x203A, 0x045A, 0x045C, 0x045B, 0x045F,
      0x00A0, 0x040E, 0x045E, 0x0408, 0x00A4, 0x0490, 0x00A6, 0x00A7, 0x0401, 0x00A9, 0x0404, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x0407,
      0x00B0, 0x00B1, 0x0406, 0x0456, 0x0491, 0x00B5, 0x00B6, 0x00B7, 0x0451, 0x2116, 0x0454, 0x00BB, 0x0458, 0x0405, 0x0455, 0x0457,
	  };



void dos2koi(char* st)
{
 int n, i;
 unsigned char* str = (unsigned char*)st;
 n = strlen(st);
 for(i=0;i<n;i++) if(str[i]>=0x80) str[i]=(unsigned char)conv_Dos2Koi[str[i]-0x80];

}

void dos2win(char* st)
{
 int n, i;
 unsigned char* str = (unsigned char*)st;
 n = strlen(st);
 for(i=0;i<n;i++) if(str[i]>=0x80) str[i]=(unsigned char)conv_Dos2Win[str[i]-0x80];
}

void koi2dos(char* st)
{
 int n, i;
 unsigned char* str = (unsigned char*)st;
 n = strlen(st);
 for(i=0;i<n;i++) if(str[i]>=0x80) str[i]=(unsigned char)conv_Koi2Dos[str[i]-0x80];
}

void koi2win(char* st)
{
 int n, i;
 unsigned char* str = (unsigned char*)st;
 n = strlen(st);
 for(i=0;i<n;i++) if(str[i]>=0x80) str[i]=(unsigned char)conv_Koi2Win[str[i]-0x80];
}

void win2dos(char* st)
{
 int n, i;
 unsigned char* str = (unsigned char*)st;
 n = strlen(st);
 for(i=0;i<n;i++) if(str[i]>=0x80) str[i]=(unsigned char)conv_Win2Dos[str[i]-0x80];
}

void win2koi(char* st)
{
 int n, i;
 unsigned char* str = (unsigned char*)st;
 n = strlen(st);
 for(i=0;i<n;i++) if(str[i]>=0x80) str[i]=(unsigned char)conv_Win2Koi[str[i]-0x80];
}


//Transcode cp866(ru) to latin
void r2tru(char* in, char* out)
{
 int i,j;
 char*p=out;

 for(i=0;i<1+(int)strlen(in); i++)
 {
  if(in[i]>=0) (*p++)=in[i];
  else
  {
   for(j=0; j<72; j++)
   {
    if(in[i]==tbl_win[j])
    {
     (*p++)=tbl_trn[j];
     if((j==6)||(j==23)||(j==24)||(j==38)||(j==55)||(j==56)) (*p++)='h'; //zh, ch, sh
     if((j==25)||(j==57)) (*p++)='c'; //sc
     if((j==30)||(j==62)) (*p++)='u'; //ju
     if((j==31)||(j==63)) (*p++)='a'; //sc
     if((j==66)||(j==67)) (*p++)='i'; //yi
     if((j==68)||(j==69)) (*p++)='e'; //ye
     if((j==70)||(j==71)) (*p++)='o'; //yo
     break;
    }
    if(j==72) (*p++)='?';
   }
  }
 }
}

void win2wc(wchar_t* wc, const char* str, int maxlen)
 {
  int i, j;
  unsigned char c;
  unsigned int u;
  void* pp = (void*)wc;
  unsigned char* p = (unsigned char*)pp;

  for(i=0;i<(maxlen-1);i++)
  {
   c=(unsigned char)str[i];
   if(c<' ') break;
   else if(c >= 0xC0) u = (unsigned int)c - 0xC0 + 0x0410;
   else if(c >= 0x80) u = (unsigned int)unescape[c - 0x80];
   else u = (unsigned int)c;

   for(j=0;j<sizeof(wchar_t);j++)
   {
    (*p++) = u&0xFF;
    u>>=8;
   }
  }
  memset(p, 0, 4);
 }
 


void utf2wchar(char* swt, char* sut, int max)
{

        unsigned char* wt = (unsigned char*) swt;
        unsigned char* ut = (unsigned char*) sut;
	unsigned   int ch1 = 0, ch2 = 0, ch3 = 0, ch4 = 0;
	unsigned   int code = 0;
	unsigned   int codeh1 = 0, codeh2 = 0;
	unsigned   int c = 0, b = 0, b1 = 0, b2 = 0, a1 = 0, a2 = 0;

	ch1 = *ut++;
	while (ch1) {

		if ((ch1 >> 7) == 0x0) //0b000)
                { // We are in 1 bytes to take normall code form Utf-8. -->0xxxxxxx
			ch1 = ch1 << 1;
			ch1 >>= 1;
			code = ch1;

		}
		else if ((ch1 >> 5) == 0x5) //0b110)
                { // We are in 2 bytes to take normall code form Utf-8. --> 110xxxxx,10xxxxxx
			ch1 <<= 27;
			ch1 >>= 27;
			ch2 = *ut++;
			ch2 <<= 26;
			ch2 >>= 26;
			code = (ch1 << 6) | ch2;

		}
		else if ((ch1 >> 4) == 0xE) //0b1110)
                { // We are in 3 bytes to take normall code form Utf-8. --> 1110xxxx,10xxxxxx,10xxxxxx
			ch1 <<= 28;
			ch1 >>= 28;
			ch2 = *ut++;
			ch2 <<= 26;
			ch2 >>= 26;
			ch3 = *ut++;
			ch3 <<= 26;
			ch3 >>= 26;
			code = (ch1 << 12) | (ch2 << 6) | ch3;

		}
		else if ((ch1 >> 3) == 0x1E) //0b11110)
                { // We are in 4 bytes to take normall code form Utf-8. --> 11110xxx,10xxxxxx,10xxxxxx,10xxxxxx
			ch1 <<= 29;
			ch1 >>= 29;
			ch2 = *ut++;
			ch2 <<= 26;
			ch2 >>= 26;
			ch3 = *ut++;
			ch3 <<= 26;
			ch3 >>= 26;
			ch4 = *ut++;
			ch4 <<= 26;
			ch4 >>= 26;
			code = (ch1 << 18) | (ch2 << 12) | (ch3 << 6) | ch4;

		}
		else { // we are not given right Utf-8 code.
			//printf("ERROR\n");
                         *wt++ =0;
                         *wt++ =0;
			return;
		}

                if(!code) break;
		if ((code > 0x0000 && code <= 0xD7FF) || (code >= 0xE000 && code <= 0xFFFF)) { // We have a code up tp 2 bytes to make it  in Utf-16.
                        if(max < 4) break;
                        max-=2;
			if (code <= 0x007F) { // We have  up to 7 digit code.
				codeh1 = 0;
				*wt++ = (codeh1);
				codeh2 = code;
				*wt++ = (codeh2);
			}

			else if (code >= 0x0080 && code <= 0x07FF) { // We have up to 11 digit code.
				codeh1 = code >> 8;
				*wt++ = (codeh1);
				codeh2 = code << 24;
				codeh2 >>= 24;
				*wt++ = (codeh2);

			}
			else if (code >= 0x0800 && code <= 0xFFFF) { // We have up to 16 digit code.
				codeh1 = code >> 8;
				*wt++ = (codeh1);
				codeh2 = code << 24;
				codeh2 >>= 24;
				*wt++ = (codeh2);
			}
		}
		else { // We have a code 4 bytes to make it  in Utf-16.
                        if(max < 6) break;
                        max-=4;
			c = code;
			b = c - 0x010000;
			b1 = b >> 10;
			b2 = b << 22;
			b2 >>= 22;
			a1 = b1 + 0xD800;
			a2 = b2 + 0xDC00;
			codeh1 = a1 >> 8;
			*wt++ = (codeh1);
			codeh2 = a1 << 24;
			codeh2 >>= 24;
			*wt++ = (codeh2);
			codeh1 = a2 >> 8;
			*wt++ = (codeh1);
			codeh2 = a2 << 24;
			codeh2 >>= 24;
			*wt++ = (codeh2);

		}
		ch1 = *ut++; //For our next loop.
	} // We have finished file convert  if not return 1;
            *wt++ =0;
            *wt++ =0;
	return;
}


 

// sdi1500129
// Editor : Panagiotis Petropoulakis
void wchar2utf(char* sut, char* swt, int max)
{
        unsigned char* ut = (unsigned char*)sut;
        unsigned char* wt = (unsigned char*) swt;
	unsigned  int ch1 = 0 ,ch2 = 0,ch3 = 0,ch4 = 0;
	unsigned  int code;
	unsigned  int codeh1,codeh2;
	unsigned  int c,b ,b1,b2, a1, a2;
	
	
	
		ch1 = *wt++;
		while (1) {
			code = 0;
			codeh1 = 0;
			c = 0;
			b = 0;
			b1 = 0;
			b2 = 0;
			a1 = 0;
			a2 = 0;
			ch2 = *wt++;
			code = (ch1 << 8) | ch2;
                        
                        if(!code) break;
			if (!((code >= 0x0000 && code <= 0xD7FF) | (code >= 0xE000 && code <= 0xFFFF))) {
				code = 0;
				ch3 = *wt++;
				ch4 = *wt++;
				a1 = (ch1 << 8) | ch2;
				a2 = (ch3 << 8) | ch4;
				if ((a1 >= 0xD800 && a1 <= 0xDBFF) & (a2 >= 0xDC00 && a2 <= 0xDFFF)) { // Is a  4 bytes in  Utf-16.
					b1 = a1 - 0xD800;
					b2 = a2 - 0xDC00;
					b = (b1 << 10) | b2;
					c = b + 0x010000;
					code = c;
				}
				else {
				       //	printf("ERROR\n");
				}

			}

			///// now we check code and make it utf-8 

			if (code >= 0x0000 && code <= 0x007F) { // we must make it 1 byte utf-8
				codeh1 |= code;
                                if(max<2) break;
                                max--;
				*ut++ = (codeh1);
			}
			else if (code >= 0x0080 && code <= 0x07FF) { // we must make it 2 byte utf-8
                                if(max<3) break;
                                max-=2;
				codeh1 = (code >> 6) | 0xC0;
				codeh2 = code << 26;
				codeh2 >>= 26;
				codeh2 |= 0x80;
				*ut++ = (codeh1);
				*ut++ = (codeh2);
			}
			else if (code >= 0x0800 && code <= 0xFFFF) {	// we must make it 3 byte utf-8
                                if(max<4) break;
                                max-=3;
				codeh1 = (code >> 12) | 0xE0;
				codeh2 = code << 20;
				codeh2 >>= 26;
				codeh2 |= 0x80;
				*ut++ = (codeh1);
				*ut++ = (codeh2);
				codeh1 = code << 26;
				codeh1 >>= 26;
				codeh1 |= 0x80;
				*ut++ = (codeh1);
			}
			else if (code >= 0x10000 && code <= 0x10FFFF) {	// we must make it 4 byte utf-8		
                                if(max<5) break;
                                max-=4;
				codeh1 = (code >> 11);
				codeh1 |= 0xF0;
				codeh2 = code << 14;
				codeh2 >>= 26;
				codeh2 |= 0x80;
				*ut++ = (codeh1);
				*ut++ = (codeh2);
				codeh1 = code << 20;
				codeh1 >>= 26;
				codeh1 |= 0x80;
				codeh2 = code << 26;
				codeh2 >>= 26;
				codeh2 |= 0x80;
				*ut++ = (codeh1);
				*ut++ = (codeh2);
			}

			ch1 = *wt++;
			}

        *ut++ = 0;
	return;
}


void win2utf(char* sut, char* sst, int max)
{
 unsigned char* ut = (unsigned char*)sut;
 unsigned char* st = (unsigned char*)sst;
 unsigned char c;
 unsigned short code;
 unsigned  int codeh1,codeh2;
 
 while(1)
 {
   //get next win1251 char and check for end of string
   c=*st++;
   if(!c) break;

   //convert to 2-bytes utf-16
   else if(c >= 0xC0) code= (unsigned int)c - 0xC0 + 0x0410;
   else if(c >= 0x80) code= (unsigned int)unescape[c - 0x80];
   else code= (unsigned int)c;

   //convert utf16 to utf8
   if(code >= 0x0000 && code <= 0x007F)
   { // we must make it 1 byte utf-8
     codeh1 = code;
     if(max<2) break;
     max--;
     *ut++ = (codeh1);
   }
   else if (code >= 0x0080 && code <= 0x07FF)
   { // we must make it 2 byte utf-8
     if(max<3) break;
     max-=2;
     codeh1 = (code >> 6) | 0xC0;
     codeh2 = code << 26;
     codeh2 >>= 26;
     codeh2 |= 0x80;
     *ut++ = (codeh1);
     *ut++ = (codeh2);
   }
   else if (code >= 0x0800 && code <= 0xFFFF)
   {	// we must make it 3 byte utf-8
     if(max<4) break;
     max-=3;
     codeh1 = (code >> 12) | 0xE0;
     codeh2 = code << 20;
     codeh2 >>= 26;
     codeh2 |= 0x80;
     *ut++ = (codeh1);
     *ut++ = (codeh2);
     codeh1 = code << 26;
     codeh1 >>= 26;
     codeh1 |= 0x80;
     *ut++ = (codeh1);
   }
 }
 *ut++ = 0; //put end of urf8 string
}
