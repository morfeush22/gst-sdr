/*
 * file_src.h
 *
 *  Created on: May 13, 2015
 *      Author: morfeush22
 */

#ifndef SRC_FILE_SRC_H_
#define SRC_FILE_SRC_H_

#include "abstract_src.h"

class FileSrc: public AbstractSrc {
public:
	FileSrc(const char *);
	virtual ~FileSrc();

	void InitSrc(void *);
	const char *get_name();

private:
	AbstractSrcHelpers::Data *data_;

	const char *path_;

};

#endif /* SRC_FILE_SRC_H_ */
