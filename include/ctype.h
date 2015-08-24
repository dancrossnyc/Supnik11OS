#define _CTUPPER        (1<<0)
#define _CTLOWER        (1<<1)
#define _CTDIGIT        (1<<2)
#define _CTXDIGIT       (1<<3)
#define _CTWSPACE       (1<<4)
#define _CTPUNCT        (1<<5)
#define _CTCTRL         (1<<6)
#define _CTBLANK        (1<<7)

#define _CTALPHA        (_CTUPPER | _CTLOWER)
#define _CTALNUM        (_CTALPHA | _CTDIGIT)
#define _CTSPACE        (_CTBLANK | _CTWSPACE)
#define _CTGRAPH        (_CTALNUM | _CTPUNCT)
#define _CTPRINT        (_CTGRAPH | _CTBLANK)
#define _CTANYDIGIT     (_CTDIGIT | _CTXDIGIT)
#define _CTABCDEF       (_CTUPPER | _CTXDIGIT)
#define _CTabcdef       (_CTLOWER | _CTXDIGIT)

extern unsigned char _ctable[256];

#define isalpha(c)  (_ctable[(unsigned char)(c)] & _CTALPHA)
#define islower(c)  (_ctable[(unsigned char)(c)] & _CTLOWER)
#define isupper(c)  (_ctable[(unsigned char)(c)] & _CTUPPER)
#define isdigit(c)  (_ctable[(unsigned char)(c)] & _CTDIGIT)
#define isxdigit(c) (_ctable[(unsigned char)(c)] & _CTXDIGIT)
#define isspace(c)  (_ctable[(unsigned char)(c)] & _CTSPACE)
#define ispunct(c)  (_ctable[(unsigned char)(c)] & _CTPUNCT)
#define isalnum(c)  (_ctable[(unsigned char)(c)] & _CTALNUM)
#define isprint(c)  (_ctable[(unsigned char)(c)] & _CTPRINT)
#define isgraph(c)  (_ctable[(unsigned char)(c)] & _CTGRAPH)
#define iscntrl(c)  (_ctable[(unsigned char)(c)] & _CTCTRL)
#define isascii(c)  ((unsigned char)(c) <= 0x7F)

#define toascii(c)  ((c) & 0x7F)
extern int tolower(int c);
extern int toupper(int c);
