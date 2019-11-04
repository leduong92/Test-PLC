
#include "windows.h"
#include "getnow.h"

int UruuCheck(int year);

void get_nowtime( char *buf, short mode, short plus )
{
	time_t 	ltime;
	struct	tm	*nowtime;
    char dummy[20];
	TMSTRCT *outTime;

    memset( dummy, 0 , sizeof(dummy));

	time(&ltime);
	ltime += (time_t)(plus)*24*60*60;
	nowtime=localtime(&ltime);

    if(nowtime->tm_year >= 95){
       nowtime->tm_year += 1900;
    }
    else{
       nowtime->tm_year += 2000;
    }

	sprintf(dummy,"%04d%02d%02d%02d%02d%02d", nowtime->tm_year,
                      nowtime->tm_mon+1,nowtime->tm_mday,nowtime->tm_hour,
                                   nowtime->tm_min,nowtime->tm_sec	);

    switch( mode){
       case HDATEL:
          memcpy( buf, dummy, HDATEL );
          break;

       case LDATEL:
          memcpy( buf, dummy, LDATEL );
          break;

       case MDATEL:
          memcpy( buf, dummy, MDATEL );
          break;

       case SDATEL:
          memcpy( buf, dummy, SDATEL );
          break;

       case HOST_L:
		  if( memcmp( &dummy[2],"93", 2) > 0 )
				buf[0]='0';
		  else
				buf[0]='1';
		  memcpy( &buf[1],&dummy[2],HOST_L);
		  break;

		case TIMEST:
			outTime = (TMSTRCT*)buf;
			outTime->year = nowtime->tm_year;
			outTime->month = nowtime->tm_mon+1;
			outTime->day = nowtime->tm_mday;
			outTime->hour = nowtime->tm_hour;
			outTime->min = nowtime->tm_min;
			outTime->sec = nowtime->tm_sec;
          break;
	   default:
          break;
   }

}

void Calctime( char *buf,char *date, short plus )
{
	time_t 	ltime=0;
	struct	tm	nowtime,*newtime;
    char dummy[20];

	memset( &nowtime,0,sizeof(nowtime));

    memset( dummy, 0 , sizeof(dummy));
	memcpy(dummy,date,4);
	nowtime.tm_year = atol(dummy)-1900;

    memset( dummy, 0 , sizeof(dummy));
	memcpy(dummy,&date[4],2);
	nowtime.tm_mon = atol(dummy)-1;

    memset( dummy, 0 , sizeof(dummy));
	memcpy(dummy,&date[6],2);
	nowtime.tm_mday=atol(dummy);

	ltime = mktime(&nowtime);
	ltime += (time_t)(plus)*24*60*60;

	newtime=localtime(&ltime);
    if(newtime->tm_year >= 95){
       newtime->tm_year += 1900;
    }
    else{
       newtime->tm_year += 2000;
    }

	sprintf(dummy,"%04d%02d%02d%02d%02d%02d", newtime->tm_year,
                      newtime->tm_mon+1,newtime->tm_mday,newtime->tm_hour,
                                   newtime->tm_min,newtime->tm_sec	);

	memcpy( buf, dummy, 14 );

}

void get_nowtime2( char *buf, short mode, long plus ,int tani)
{
	time_t 	ltime;
	struct	tm	*nowtime;
    char dummy[20];

    memset( dummy, 0 , sizeof(dummy));

	time(&ltime);

	switch( tani ){
		case MD_SEC:
			ltime += (time_t)plus;
			break;
		case MD_MIN :
			ltime += (time_t)(plus)*60;
			break;
		case MD_HOUR:
			ltime += (time_t)(plus)*60*60;
			break;
		case MD_DAY:
			ltime += (time_t)(plus)*24*60*60;
			break;
		default:
			break;
		
	}

	nowtime=localtime(&ltime);

    if(nowtime->tm_year >= 95){
       nowtime->tm_year += 1900;
    }
    else{
       nowtime->tm_year += 2000;
    }

	sprintf(dummy,"%04d%02d%02d%02d%02d%02d", nowtime->tm_year,
                      nowtime->tm_mon+1,nowtime->tm_mday,nowtime->tm_hour,
                                   nowtime->tm_min,nowtime->tm_sec	);

    switch( mode){
       case HDATEL:
          memcpy( buf, dummy, HDATEL );
          break;

       case LDATEL:
          memcpy( buf, dummy, LDATEL );
          break;

       case MDATEL:
          memcpy( buf, dummy, MDATEL );
          break;

       case SDATEL:
          memcpy( buf, dummy, SDATEL );
          break;

       case HOST_L:
		  if( memcmp( &dummy[2],"93", 2) > 0 )
				buf[0]='0';
		  else
				buf[0]='1';
		  memcpy( &buf[1],&dummy[2],HOST_L);
		  break;

	   default:
          break;
   }

}

void NglogAdd(char *prgname, char *msg, short code,char* dir)
{
	char filename[MAX_PATH+1];
	char now[HDATEL+1];
	char buf[24];
	FILE *fp;
	WIN32_FIND_DATA fd;
	HANDLE hFind;

   memset(now,      0 ,sizeof(now));
   get_nowtime( now,SDATEL,0);

	if( dir==NULL){
		strcpy( filename,".\\log\0\0");
		memset( &fd, 0, sizeof(fd) );
		fd.dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
		hFind = FindFirstFile(filename,&fd);
		if( hFind == INVALID_HANDLE_VALUE){
			if(!CreateDirectory(filename,NULL)){
				FindClose(hFind);
				return;
			}
		}

		FindClose(hFind);
		sprintf(filename, ".\\log\\%s.err",now);
	}
	else{
		sprintf(filename, "%s\\%s.err",dir,now);
	}
   if( (fp = fopen( filename,"ab")) == NULL ){
      return;
   }

   memset(now, 0 ,sizeof(now));       
   memset(buf, 0 ,sizeof(buf));
   get_nowtime( now,HDATEL,0);
   memcpy(buf,&now[2],2);
   strcat(buf,"/");
   memcpy(&buf[3],&now[4],2);
   strcat(buf,"/");
   memcpy(&buf[6],&now[6],2);
   strcat(buf," ");
   memcpy(&buf[9],&now[8],2);
   strcat(buf,":");
   memcpy(&buf[12],&now[10],2);
   strcat(buf,":");
   memcpy(&buf[15],&now[12],2);
   fprintf(fp, "%s:",buf);

   fprintf(fp, "%s:",prgname);        
   fprintf(fp, "%s",msg);
   if(code)
	   fprintf(fp,":err=%d\r\n",code);	
   else
	   fprintf(fp, "\r\n");				

   fclose(fp);
}

void Calctime( char *buf,char *date, int plus ,short tani)
{
	time_t 	ltime=0;
	struct	tm	nowtime,*newtime;
    char dummy[20];

	memset( &nowtime,0,sizeof(nowtime));

    memset( dummy, 0 , sizeof(dummy));
	memcpy(dummy,date,4);
	nowtime.tm_year = atol(dummy)-1900;

	memcpy(dummy,&date[4],2);
	dummy[2] = 0;
	nowtime.tm_mon = atol(dummy)-1;

	memcpy(dummy,&date[6],2);
	dummy[2] = 0;
	nowtime.tm_mday=atol(dummy);

	memcpy(dummy,&date[8],2);
	dummy[2] = 0;
	nowtime.tm_hour=atol(dummy);

	memcpy(dummy,&date[10],2);
	dummy[2] = 0;
	nowtime.tm_min=atol(dummy);

	memcpy(dummy,&date[12],2);
	dummy[2] = 0;
	nowtime.tm_sec=atol(dummy);

	ltime = mktime(&nowtime);

	switch( tani ){
		case MD_SEC:
			ltime += (time_t)plus;
			break;
		case MD_MIN :
			ltime += (time_t)(plus)*60;
			break;
		case MD_HOUR:
			ltime += (time_t)(plus)*60*60;
			break;
		case MD_DAY:
			ltime += (time_t)(plus)*24*60*60;
			break;
		default:
			break;
		
	}

	newtime=localtime(&ltime);
    if(newtime->tm_year >= 95){
       newtime->tm_year += 1900;
    }
    else{
       newtime->tm_year += 2000;
    }

	sprintf(dummy,"%04d%02d%02d%02d%02d%02d", newtime->tm_year,
                      newtime->tm_mon+1,newtime->tm_mday,newtime->tm_hour,
                                   newtime->tm_min,newtime->tm_sec	);

	memcpy( buf, dummy, 14 );

}

void CvDateTime(char *src,char *dest,int mode)
{
	if( mode == CVMODE_TIME ){
		if( strlen(src) < 6 ) return;
		memcpy(dest,src,2);
		memcpy(dest+2,":",1);
		memcpy(dest+3,src+2,2);
		memcpy(dest+5,":",1);
		memcpy(dest+6,src+4,2);
	}

	if( mode == CVMODE_DATE ){
		if( strlen(src) < 8 ) return;
		memcpy(dest,src,4);
		memcpy(dest+4,"/",1);
		memcpy(dest+5,src+4,2);
		memcpy(dest+7,"/",1);
		memcpy(dest+8,src+6,2);
	}
	
	if( mode == CVMODE_DATETIME ){
		if( strlen(src) < 14 ) return;
		memcpy(dest,src,4);
		memcpy(dest+4,"/",1);
		memcpy(dest+5,src+4,2);
		memcpy(dest+7,"/",1);
		memcpy(dest+8,src+6,2);
		memcpy(dest+10," ",1);
		memcpy(dest+11,src+8,2);
		memcpy(dest+13,":",1);
		memcpy(dest+14,src+10,2);
		memcpy(dest+16,":",1);
		memcpy(dest+17,src+12,2);
	}
}

int get_yobi(char *date)
{
	int y,m,d,yobi;
	long count;
	char buf[20];
	int     MTBL[12]=  { 0,31,59,90,120,151,181,212,243,273,304,334 };

	if( strlen(date) < 8 )return -1;
	
	memcpy(buf,date,4);buf[4]=0;
	y=atoi(buf);
	memcpy(buf,date+4,2);buf[2]=0;
	m=atoi(buf);
	memcpy(buf,date+6,2);buf[2]=0;
	d=atoi(buf);

    count=  (y-1)*365L;        
    count+= (y/4L - y/100L + y/400L + MTBL[m-1] + d);
    if( m<3 && UruuCheck(y) )  count--;

	yobi= count%7L;
	return yobi;
}

int UruuCheck(int year)
{
	if (year%400L==0L)  return(1);
	if(year%100L==0L)  return(0);
	if(year%4L==0L)    return(1);
	return(0);
}

int check_date(char* date)
{
	char buf[50];

	if( strlen(date) < 8 ) return -1;

	int y,m,d;
	memcpy(buf,date,4);buf[4]=0;
	y = atoi(buf);
	memcpy(buf,date+4,2);buf[2]=0;
	m = atoi(buf);
	memcpy(buf,date+6,2);buf[2]=0;
	d = atoi(buf);

	if( m < 1 || m > 12 ) return -2;

	int nUruu = 0;
	int nMaxDay;

	nUruu = UruuCheck(y);
	switch(m) {
		case 2:nMaxDay = 28+nUruu;
			break;
		case 4:
		case 6:
		case 9:
		case 11: nMaxDay = 30; break;
		default: nMaxDay = 31; break;
	}
	if( d > nMaxDay ) return -3;	

	return 0;	
}


int SubDays(int *days,const char* date1,const char* date2)
{
	int y,m,d;
	long count1,count2;
	char buf[20];
	int     MTBL[12]=  { 0,31,59,90,120,151,181,212,243,273,304,334 };

	if( strlen(date1) < 8 || strlen(date2) < 8 ) return(-1);

	memcpy(buf,date1,4);buf[4]=0;
	y=atoi(buf);
	memcpy(buf,date1+4,2);buf[2]=0;
	m=atoi(buf);
	memcpy(buf,date1+6,2);buf[2]=0;
	d=atoi(buf);
    count1=  (y-1)*365L;           
    count1+= (y/4L - y/100L + y/400L + MTBL[m-1] + d);
    if( m<3 && UruuCheck(y) )  count1--;

	memcpy(buf,date2,4);buf[4]=0;
	y=atoi(buf);
	memcpy(buf,date2+4,2);buf[2]=0;
	m=atoi(buf);
	memcpy(buf,date2+6,2);buf[2]=0;
	d=atoi(buf);
    count2=  (y-1)*365L;           
    count2+= (y/4L - y/100L + y/400L + MTBL[m-1] + d);
    if( m<3 && UruuCheck(y) )  count2--;
	*days=(int)(count1-count2);

	return(0);
}

int SubSeconds(long* seconds,const char* date1,const char* date2)
{
	int y,m,d;
	int hour, minit, second;
	long day1, day2, days;
	long time1, time2;
	char buf[20];
	int     MTBL[12]=  { 0,31,59,90,120,151,181,212,243,273,304,334 };

	if( strlen(date1) < 14 || strlen(date2) < 14 ) 
		return(-1);

	memcpy(buf,date1,4);buf[4]=0;
	y=atoi(buf);
	memcpy(buf,date1+4,2);buf[2]=0;
	m=atoi(buf);
	memcpy(buf,date1+6,2);buf[2]=0;
	d=atoi(buf);
    day1=  (y-1)*365L;           
    day1+= (y/4L - y/100L + y/400L + MTBL[m-1] + d);
    if( m<3 && UruuCheck(y) )  
		day1--;

	memcpy(buf,date2,4);buf[4]=0;
	y=atoi(buf);
	memcpy(buf,date2+4,2);buf[2]=0;
	m=atoi(buf);
	memcpy(buf,date2+6,2);buf[2]=0;
	d=atoi(buf);
    day2=  (y-1)*365L;          
    day2+= (y/4L - y/100L + y/400L + MTBL[m-1] + d);
    if( m<3 && UruuCheck(y) )  
		day2--;

	days = day1 - day2;

	memcpy(buf,date1+8,2);buf[2]=0;
	hour=atoi(buf);
	memcpy(buf,date1+10,2);buf[2]=0;
	minit=atoi(buf);
	memcpy(buf,date1+12,2);buf[2]=0;
	second=atoi(buf);
	time1 = hour*60*60 + minit*60 + second;

	memcpy(buf,date2+8,2);buf[2]=0;
	hour=atoi(buf);
	memcpy(buf,date2+10,2);buf[2]=0;
	minit=atoi(buf);
	memcpy(buf,date2+12,2);buf[2]=0;
	second=atoi(buf);
	time2 = hour*3600 + minit*60 + second;

	*seconds = days*24*60*60 +(time1 - time2);

	return(0);
}
