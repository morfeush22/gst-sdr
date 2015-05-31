/*
 * file_src.h
 *
 *  Created on: May 13, 2015
 *      Author: morfeush22
 */

#ifndef SRC_FILE_SRC_H_
#define SRC_FILE_SRC_H_

#define FILE_SRC_CAST(X) (reinterpret_cast<FileSrc *>(X))
#define FILE_SRC_DATA_CAST(X) (reinterpret_cast<FileSrcHelpers::Data *>(X))

#include "abstract_src.h"
#include "player.h"

namespace FileSrcHelpers {

struct Data {
	void *abstract_src;

	GstElement *src;
};

}

class FileSrc: public AbstractSrc {
public:
	FileSrc(const char *);
	virtual ~FileSrc();

	void SetSrc(void *);
	void LinkSrc();
	const char *name() const;

private:
	AbstractSrcHelpers::Data *data_;

	const char *path_;

};

#endif /* SRC_FILE_SRC_H_ */
