#ifndef charset_h
#define charset_h

int unicode_to_gb2312(long int unicode, char *r, char *c);
long int gb2312_to_unicode(char r, char c);
void GB2312ToUTF_8(char *pOut,char *pText, int pLen);
void UTF_8ToGB2312(char *pOut, char *pText, int pLen);

#endif
