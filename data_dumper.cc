/*
 * data_dumper.cc
 *
 *  Created on: May 20, 2015
 *      Author: hipek
 */

#include "data_dumper.h"

DataDumper::DataDumper(size_t number_of_bytes, size_t ns_interval, const char* file, RingSrc *src) {
	interval = ns_interval;
	bytes_to_dump = number_of_bytes;
	file_wrapper = new FileWrapper(file,number_of_bytes);
	clock_gettime(CLOCK_REALTIME, &last_time);
	running = false;

	buffer_ = new char[bytes_to_dump];
	float_buffer_ = new float[bytes_to_dump];
	src_ = src;
}

DataDumper::~DataDumper(){
  delete file_wrapper;
  delete[] buffer_;
  delete[] float_buffer_;
}

void DataDumper::SetInterval(size_t nanoseconds) {
	interval = nanoseconds;
}

size_t DataDumper::GetInterval() {
	return interval;
}

void *DataDumper::StartDumping() {
	running = true;
	timespec this_time;

	uint32_t returned = file_wrapper->GetNextChunk();
	const char * const* start;

	while(1){
		usleep(1);
		long int calculated_ns = last_time.tv_nsec + last_time.tv_sec*1000000000;
		clock_gettime(CLOCK_REALTIME, &this_time);
		if (this_time.tv_nsec + this_time.tv_sec*1000000000 >= calculated_ns+interval){
			if (returned>=bytes_to_dump){

				last_time = this_time;
				const char * const* start = file_wrapper->GetCurrentChunkPointer();
				memcpy(buffer_,start,bytes_to_dump);
				ConvertToFloat();
				src_->Write(float_buffer_, bytes_to_dump);
				returned = file_wrapper->GetNextChunk();
				std::cout << returned <<std::endl;
			} else {
				running = false;
				return NULL;
			}
		}

	}
}
/*
bool DataDumper::SyncDump(void* buffer) {
	timespec this_time;
	uint32_t returned = file_wrapper->GetNextChunk();

	while(1){
		usleep(1);
		long int calculated_ns = last_time.tv_nsec + last_time.tv_sec*1000000;
		clock_gettime(CLOCK_REALTIME, &this_time);
		if (calculated_ns+interval>=this_time.tv_nsec + this_time.tv_sec*1000000){
			if (returned>=bytes_to_dump){
				last_time = this_time;
				const char * const* start = file_wrapper->GetCurrentChunkPointer();
				memcpy(buffer,start,bytes_to_dump);
				return true;
			} else {
				return false;
			}
		}

	}
	return false;
}*/

bool DataDumper::IsRunning(){
	return running;
}
;

void DataDumper::ConvertToFloat() {
	for(int i=0; i<bytes_to_dump; i++) {
		float_buffer_[i] = (float)buffer_[i];
	}
}
