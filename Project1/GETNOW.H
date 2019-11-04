#ifndef GETNOW_H
#define GETNOW_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define MD_MIN 0
#define MD_HOUR 1
#define MD_SEC 2
#define MD_DAY 3

#ifndef A_MONTH
#define A_MONTH 30
#endif
#ifndef A_YEAR
#define A_YEAR  12
#endif
#ifndef HDATEL
#define HDATEL	14
#endif
#ifndef LDATEL
#define LDATEL	12
#endif
#ifndef MDATEL
#define MDATEL	10
#endif
#ifndef SDATEL
#define SDATEL	8
#endif
#ifndef HOST_L
#define HOST_L 13
#endif

#ifndef CVMODE_TIME
#define CVMODE_TIME 6
#endif
#ifndef CVMODE_DATE
#define CVMODE_DATE 8
#endif
#ifndef CVMODE_DATETIME
#define CVMODE_DATETIME 14
#endif

#define TIMEST	0		
typedef struct{
	short year;
	short month;
	short day;
	short hour;
	short min;
	short sec;
} TMSTRCT;


void get_nowtime( char *buf,short mode,short plus);
void get_nowtime2( char *buf, short mode, long plus ,int tani);
void NglogAdd(char *prgname, char *msg, short code,char* dir=NULL);
void Calctime( char *buf,char *date, short plus );
void Calctime( char *buf,char *date, int plus ,short tani);
void CvDateTime(char *src,char *dest,int mode);		
int get_yobi(char *date);
int check_date(char *date);		

static char    YOBI_TBLJP[][4]= { "ì˙","åé","âŒ","êÖ","ñÿ","ã‡","ìy" };
static char    YOBI_TBLEN[][4]= { "SUN","MON","TUE","WED","THU","FRI","SAT" };

int SubDays(int *days,const char* date1,const char* date2);
int SubSeconds( long* seconds, const char* date1, const char* date2 );

#endif
