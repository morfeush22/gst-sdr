/*
 * abstract_src.h
 *
 *  Created on: May 24, 2015
 *      Author: Kacper Patro patro.kacper@gmail.com
 */

#ifndef SRC_ABSTRACT_SRC_H_
#define SRC_ABSTRACT_SRC_H_

#define ABSTRACT_SRC_CAST(X) (reinterpret_cast<AbstractSrc *>(X))
#define ABSTRACT_SRC_DATA_CAST(X) (reinterpret_cast<AbstractSrcHelpers::Data *>(X))

namespace AbstractSrcHelpers {

/**
 * @struct Data
 * @biref This struct contains pointers to data passed when calling GStreamer functions
 */
struct Data {
	void *src_data;	/**< Source data pointer */
	void *other_data;	/**< Other data pointer */
};

}

/**
 * @class AbstractSrc
 * @brief Abstract class for sources
 *
 * @author Kacper Patro patro.kacper@gmail.com
 * @copyright Public domain
 * @pre
 */
class AbstractSrc {
public:
	AbstractSrc();
	virtual ~AbstractSrc();

	/**
	 * Sets source properties, before linking
	 * @param other_data Other data pointer, which should be assigned to other_data in AbstractSinkHelpers::Data structure
	 * @pure
	 */
	virtual void SetSrc(void *other_data) = 0;

	/**
	 * Links source element
	 * @pure
	 */
	virtual void LinkSrc() = 0;

	/**
	 * Returns element name
	 * @return Element name
	 * @pure
	 */
	virtual const char *name() const = 0;

};

#endif /* SRC_ABSTRACT_SRC_H_ */
