/*
 * FileSrc.h
 *
 *  Created on: May 13, 2015
 *      Author: morfeush22
 */

#ifndef SRC_FILESRC_H_
#define SRC_FILESRC_H_

#include "AbstractSrc.h"

class FileSrc: public AbstractSrc {
public:
	FileSrc(const char *);
	virtual ~FileSrc();

	void InitSrc(void *);
	const char *GetName();

private:
	const char *path_;

};

#endif /* SRC_FILESRC_H_ */
