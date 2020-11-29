#include <wchar.h>
void dos2koi(char* st); //cp866 to koi8ru
void dos2win(char* st); //cp866 to win1251
void koi2dos(char* st); //koi8ru to cp866
void koi2win(char* st); //koi8ro to win1251
void win2dos(char* st); //win1251 to cp866
void win2koi(char* st); //win1251 to koi8ru

void r2tru(char* in, char* out); //convert win1251 to latin (transliterate)
void win2wc(wchar_t* wc, const char* str, int maxlen); //convert win1251 to widechar (system depends len)
void utf2wchar(char* swt, char* sut, int max); //convert utf8 to utf16
void wchar2utf(char* sut, char* swt, int max); //convert utf16 to utf8
void win2utf(char* sut, char* sst, int max); //convert win1251 to utf8
       
