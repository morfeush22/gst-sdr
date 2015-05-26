#include "file_wrapper.h"

#include <cmath>
/// @cond
#include <cstdlib>
#include <cstddef>
/// @endcond

#define BUFF_COUNT 5

void *FileWrapperHelpers::ReadFromFileHelper(void *context) {
	return ((FileWrapper *)context)->ReadFromFile();
}

FileWrapper::FileWrapper(const char* file, size_t buffer_size):
input_file_(file, std::ifstream::binary),
buffSize(buffer_size),
count_(0),
finished_(false) {
	if(input_file_.good()) {
		input_file_.seekg(0, std::ios::end);
		file_size_ = input_file_.tellg();
		input_file_.seekg(0, std::ios::beg);

		to_read_ = file_size_;
		to_announce_ = file_size_;

		size_t size = BUFF_COUNT*buffSize;
		buffer_ = new char[size];
		current_chunk_pointer_ = buffer_;
		current_read_pointer_ = buffer_;

		pthread_mutex_init(&count_mutex_, NULL);
		pthread_cond_init(&count_condition_not_empty_, NULL);
		pthread_cond_init(&count_condition_not_full_, NULL);

		pthread_attr_init(&attr_);
		pthread_attr_setdetachstate(&attr_, PTHREAD_CREATE_DETACHED);

		pthread_create(&read_thread_, &attr_, FileWrapperHelpers::ReadFromFileHelper, this);
	}
	else {
		current_chunk_pointer_ = NULL;
	}
}

FileWrapper::~FileWrapper() {
	if(current_chunk_pointer_ != NULL) {
		if(!finished_)
			pthread_cancel(read_thread_);

		input_file_.close();
		delete[] buffer_;

		pthread_attr_destroy(&attr_);
		pthread_mutex_destroy(&count_mutex_);
		pthread_cond_destroy(&count_condition_not_empty_);
		pthread_cond_destroy(&count_condition_not_full_);
	}
}

void *FileWrapper::ReadFromFile() {
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,  NULL);
	while(1) {
		pthread_mutex_lock(&count_mutex_);

		while(count_ >= BUFF_COUNT-1) {	/* delay to allow read from output buffer */
			pthread_cond_wait(&count_condition_not_full_, &count_mutex_);
		}

		size_t size = buffSize;
		char *ptr = current_read_pointer_;

		if(to_read_ == file_size_) {	/* is first call? */
			size *= BUFF_COUNT;
		}

		if(to_read_ > 0) {	/* something to read */
			if(to_read_ >= size) {	/* is remainder? */
				if(size >= file_size_) {	/* read entire file */
					input_file_.read(ptr, file_size_);
					count_ += ceil(size/static_cast<float>(file_size_)/BUFF_COUNT);
				}
				else {	/* read 'size' */
					input_file_.read(ptr, size);
					count_ += size/buffSize;
				}
			}
			else {	/* remainder chunk, read rest */
				input_file_.read(ptr, to_read_);
				count_ += 1;
			}

			current_read_pointer_ += size;
			MoveInRing(current_read_pointer_);

			if(to_read_ >= size)
				to_read_ -= size;
			else
				to_read_ = 0;

			pthread_cond_signal(&count_condition_not_empty_);
			pthread_mutex_unlock(&count_mutex_);
		}
		else {
			finished_ = true;
			pthread_mutex_unlock(&count_mutex_);
			pthread_exit(NULL);
		}
	}

	return NULL;
}

uint32_t FileWrapper::GetNextChunk() {
	pthread_mutex_lock(&count_mutex_);

	int32_t old_announce = to_announce_;

	if(count_ > 0) {
		if(to_announce_ != file_size_) {	/* is first call? advance pointer */
			current_chunk_pointer_ += buffSize;
			MoveInRing(current_chunk_pointer_);
		}

		if(buffSize >= to_announce_) {	/* fits entire buffer? */
			to_announce_ = 0;
		}
		else {
			to_announce_ -= buffSize;
			old_announce = buffSize;
		}

		count_ -= 1;

		pthread_cond_signal(&count_condition_not_full_);
		pthread_mutex_unlock(&count_mutex_);

		return old_announce;
	}

	if(to_announce_ > 0) {
		pthread_cond_signal(&count_condition_not_full_);

		while(!count_) {
			pthread_cond_wait(&count_condition_not_empty_, &count_mutex_);
		}

		pthread_mutex_unlock(&count_mutex_);
		return GetNextChunk();
	}

	return 0;	/* entire read */
}

char const *const *FileWrapper::GetCurrentChunkPointer() {
	return &current_chunk_pointer_;
}

void FileWrapper::MoveInRing(char *&ref_to_ptr) {
	if(ref_to_ptr >= buffer_+(buffSize*BUFF_COUNT)-1)	/* move pointer in ring */
		ref_to_ptr = buffer_;
}
