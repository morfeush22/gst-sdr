/*
 * data_dumper.h
 *
 *  Created on: May 20, 2015
 *      Author: hipek
 */


#include "file_wrapper.h"
#include <ctime>

// something from this might be useful sometime
#include <cstddef>
#include <cstdlib>
#include <cstring>
//#include <cmath>
//#include <complex>
//#include <errno.h>
//#include <stdexcept>
//#include <signal.h>
#include <cstdio>
#include <stdio.h>

#include <iostream>
#include <unistd.h>
#include <sys/types.h>

#ifndef DATA_DUMPER_H_
#define DATA_DUMPER_H_

class DataDumper {
public:

	DataDumper (size_t, size_t, const char*);

	virtual ~DataDumper();

	void SetInterval(size_t nanoseconds);
	size_t GetInterval();
	void StartDumping(void *buffer);
	//bool SyncDump(void* buffer);
	bool IsRunning();

private:
	size_t bytes_to_dump;
	size_t interval;
	timespec last_time;
	FileWrapper *file_wrapper;
	bool running;

};



#endif /* DATA_DUMPER_H_ */
