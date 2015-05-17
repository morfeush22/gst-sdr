/*
 * ring_buffer.cc
 *
 *  @date Created on: May 13, 2015
 *	@author: hipek <pawel_szulc@onet.pl>
 */

#include "ring_buffer.h"

RingBuffer::RingBuffer(size_t size){
	buffer = new float[size];
	length = size;
	last_op_write = false;
	head = 0;
	tail = 0;
}

RingBuffer::~RingBuffer(){
	delete[] buffer;
}

float RingBuffer::ReadNext (){
	float f=*(buffer+tail);
	tail=(tail+1)%length;
	last_op_write = false;
	return f;
}

void RingBuffer::WriteNext (float f){
	*(buffer+head)=f;
	head=(head+1)%length;
	last_op_write = true;
}

float RingBuffer::sReadNext (){
	if (DataStored())
		return ReadNext();
	else
		return NULL;
}

bool RingBuffer::sWriteNext (float f){
	if (FreeSpace()){
		WriteNext(f);
		return true;
	} else {
		return false;
	}
}

size_t RingBuffer::ReadFrom (float *dest_buffer, size_t number_to_write){
//	if (number_to_write>DataStored())
//		number_to_write=DataStored();
	last_op_write = false;
	size_t number_written=number_to_write;

	while (number_to_write>TailToRightEnd()){
		memcpy(dest_buffer,buffer+tail,TailToRightEnd()*sizeof(float));
		number_to_write-=TailToRightEnd();
		tail=0;
	}
	memcpy(dest_buffer,buffer+tail,number_to_write*sizeof(float));
	tail+=number_to_write;
	return number_written;

}

size_t RingBuffer::WriteInto (float *source_buffer, size_t number_to_write){
	size_t number_written = number_to_write;
	last_op_write = true;

	while (number_to_write>HeadToRightEnd()){
		memcpy(buffer+head,source_buffer,HeadToRightEnd()*sizeof(float));
		number_to_write-=HeadToRightEnd();
		head=0;
	}
	memcpy(buffer+head,source_buffer,number_to_write*sizeof(float));
	head+=number_to_write;

	return number_written;
}

size_t RingBuffer::sReadFrom (float *dest_buffer, size_t number_to_write){
		if (number_to_write>DataStored())
			number_to_write=DataStored();
		return ReadFrom(dest_buffer,number_to_write);
}

size_t RingBuffer::sWriteInto (float *source_buffer, size_t number_to_write){
	if (number_to_write>FreeSpace())
			number_to_write=FreeSpace();
	return WriteInto(source_buffer,number_to_write);
}

size_t RingBuffer::DataStored(){
	if (head>tail)
		return head - tail;
	else if (head<tail)
		return length - tail + head;
	else if (last_op_write)
		return length;
	else
		return 0;
}

size_t RingBuffer::FreeSpace(){
	return length-DataStored();
}

size_t RingBuffer::HeadToRightEnd(){
	return length-head;
}

size_t RingBuffer::TailToRightEnd(){
	return length-tail;
}

