/*
 * AbstractSrc.h
 *
 *  Created on: May 13, 2015
 *      Author: morfeush22
 */

#ifndef SRC_ABSTRACTSRC_H_
#define SRC_ABSTRACTSRC_H_

class AbstractSrc {
public:
	AbstractSrc();
	virtual ~AbstractSrc();

	virtual void InitSrc(void *data) = 0;
	virtual const char *GetName() = 0;
};

#endif /* SRC_ABSTRACTSRC_H_ */
