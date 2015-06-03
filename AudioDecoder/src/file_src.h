/*
 * file_src.h
 *
 *  Created on: May 13, 2015
 *      Author: Kacper Patro patro.kacper@gmail.com
 */

#ifndef SRC_FILE_SRC_H_
#define SRC_FILE_SRC_H_

#define FILE_SRC_CAST(X) (reinterpret_cast<FileSrc *>(X))
#define FILE_SRC_DATA_CAST(X) (reinterpret_cast<FileSrcHelpers::Data *>(X))

#include "abstract_src.h"
#include "player.h"

namespace FileSrcHelpers {

/**
 * @struct Data
 * @biref This struct contains specific for FileSrc class elements
 */
struct Data {
	void *abstract_src;	/**< Pointer to "this" src element */

	GstElement *src;	/**< Src element for GStreamer */
};

}

/**
 * @class FileSrc
 * @brief Class used to read from audio files
 *
 * @author Kacper Patro patro.kacper@gmail.com
 * @copyright Public domain
 * @pre
 */
class FileSrc: public AbstractSrc {
public:
	/**
	 * Constructor of FileSrc
	 * @param path Path to input file
	 */
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
