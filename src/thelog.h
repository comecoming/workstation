#ifndef __THELOG_H__
#define __THELOG_H__

#include <mutex>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

class theLog {
public:
	theLog();
	~theLog();
	void create(const char *path);
		void printf(const char *format, ...);
		void close();
		

private:
	int itoa(int num, char *str);
		void mk_filename(char filename[]);
	FILE *logfd;
	std::mutex mutex_log;
	
};

#endif
