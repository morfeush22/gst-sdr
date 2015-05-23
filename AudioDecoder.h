/*
 * AudioDecoder.h
 *
 *  Created on: May 23, 2015
 *      Author: morfeush22
 */

#ifndef AUDIODECODER_H_
#define AUDIODECODER_H_

#include "src/AbstractSink.h"
#include "src/RingSrc.h"
#include "src/PulseSink.h"
#include "src/Player.h"

class AudioDecoder {
public:
	AudioDecoder();
	virtual ~AudioDecoder();

	void RemoveSink(AbstractSink *);
	AbstractSink *AddSink(AbstractSink *);

	void Write(float *, size_t);

private:
	RingSrc *src_;
	PulseSink *sink_;
	Player *player_;

};

#endif /* AUDIODECODER_H_ */
