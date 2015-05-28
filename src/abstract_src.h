/*
 * abstract_src.h
 *
 *  Created on: May 13, 2015
 *      Author: morfeush22
 */

#ifndef SRC_ABSTRACT_SRC_H_
#define SRC_ABSTRACT_SRC_H_

#define ABSTRACT_SRC_CAST(X) (reinterpret_cast<AbstractSrc *>(X))
#define ABSTRACT_SRC_DATA_CAST(X) (reinterpret_cast<AbstractSrcHelpers::Data *>(X))

namespace AbstractSrcHelpers {

struct Data {
	void *src_data;
	void *other_data;
};

}

class AbstractSrc {
public:
	AbstractSrc();
	virtual ~AbstractSrc();

	virtual void InitSrc(void *other_data) = 0;
	virtual const char *get_name() = 0;

};

#endif /* SRC_ABSTRACT_SRC_H_ */
