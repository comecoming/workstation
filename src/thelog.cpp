#include <stdarg.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>

#include "thelog.h"

theLog::theLog()
{
	logfd = NULL;
}

theLog::~theLog()
{
	theLog::close();
}

int theLog::itoa(int num, char *str)
{
	if (num == 0) return 0;
	
	int ret;
	ret = itoa(num/10, str);
	*(str+ret) = num%10+'0';

	return ++ret;

}

void theLog::mk_filename(char filename[])
{
	struct timeval tv;
	struct tm *m_tm;
	char filename1[10] = {0};
	char filename2[10] = {0};
	int timename1;
	int timename2;
	int i;
	
	gettimeofday(&tv, NULL);
	m_tm = localtime(&tv.tv_sec);

	timename1 = (m_tm->tm_year+1900)*100*100 + \
				(m_tm->tm_mon+1)*100 + \
				(m_tm->tm_mday);
	timename2 = (m_tm->tm_hour*100*100) + \
				(m_tm->tm_min)*100 + \
				(m_tm->tm_sec);

				itoa(timename1, filename1);
				itoa(timename2, filename2);
				strcat(strcat(strcat(strcat(filename, "/"), filename1), filename2), ".log");
	
}

void theLog::create(const char *path)
{
	char file[100];
	strcpy(file, path);
	mk_filename(file);
	theLog::close();
	logfd = fopen(file, "w+");

}

void theLog::printf(const char *format, ...)
{
	if (logfd == NULL) return;
	va_list valist;
	va_start(valist, format);

	
	struct timeval tv;
	gettimeofday(&tv, NULL);
	
	mutex_log.lock();
	fprintf(logfd, "[%lu.%lu] ", tv.tv_sec, tv.tv_usec);
	vfprintf(logfd, format, valist);
	mutex_log.unlock();
	fflush(logfd);
}

void theLog::close()
{
	if (logfd != NULL) {
		fflush(logfd);
		fclose(logfd);
		logfd = NULL;
	}

}




