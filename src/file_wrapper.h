/**
 * @class FileWrapper
 * @brief Wrapper for file reading
 *
 * Provides fast access to files.
 *
 * @author Kacper Patro patro.kacper@gmail.com
 * @version 0.1
 * @copyright Public domain
 * @pre
 */

#ifndef FILEWRAPPER_H_
#define FILEWRAPPER_H_

#include <pthread.h>
/// @cond
#include <stdint.h>
/// @endcond
#include <fstream>

namespace FileWrapperHelpers {
	extern "C" void *ReadFromFileHelper(void *);
}

class FileWrapper {
public:
	/**
	 * Wrapp file
	 * @param file path to file
	 * @param buffer_size size of internal buffer
	 */
	FileWrapper(const char*, size_t);
	virtual ~FileWrapper();

	/**
	 * @return constant pointer to current chunk
	 */
	const char *const *GetCurrentChunkPointer();

	/**
	 * @return number of items that could be read from internal buffer
	 */
	uint32_t GetNextChunk();
	friend void *FileWrapperHelpers::ReadFromFileHelper(void *);

private:
	pthread_attr_t attr_;
	pthread_mutex_t count_mutex_;
	pthread_cond_t count_condition_not_empty_;
	pthread_cond_t count_condition_not_full_;
	pthread_t read_thread_;
	uint32_t count_;

	std::ifstream input_file_;
	const size_t buffSize;
	size_t file_size_;
	int32_t to_read_;
	int32_t to_announce_;
	char *input_buffer_;
	char *current_chunk_pointer_;
	char *current_read_pointer_;
	void *ReadFromFile();
};

#endif /* FILEWRAPPER_H_ */
