/*
 * RingSrc.h
 *
 *  Created on: May 17, 2015
 *      Author: morfeush22
 */

#ifndef SRC_RINGSRC_H_
#define SRC_RINGSRC_H_

#include "AbstractSrc.h"
#include <stdint.h>
#include <gst/gst.h>

class RingSrc: public AbstractSrc {
public:
	RingSrc(const char *);
	virtual ~RingSrc();

	void InitSrc(void *);
	const char *GetName();

	uint32_t GetSize();
	void SetSize(uint32_t);

	char *GetBuff();
	void SetBuff(uint32_t);

	uint32_t DecrementPlaybackSpeed(GstElement *, uint32_t);
	uint32_t IncrementPlaybackSpeed(GstElement *, uint32_t);

	guint &GetSourceId();

private:
	guint source_id_;
	char *buff_;
	uint32_t size_;
};

#endif /* SRC_RINGSRC_H_ */
