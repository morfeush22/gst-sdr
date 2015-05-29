/*
 * audio_decoder.h
 *
 *  Created on: May 23, 2015
 *      Author: morfeush22
 */

#ifndef AUDIO_DECODER_H_
#define AUDIO_DECODER_H_

#include "src/abstract_sink.h"
#include "src/player.h"
#include "src/pulse_sink.h"
#include "src/ring_src.h"

class AudioDecoder {
public:
	AudioDecoder(float);
	virtual ~AudioDecoder();

	void RemoveSink(AbstractSink *);
	AbstractSink *AddSink(AbstractSink *);

	void Write(float *, size_t);
	void LastFrame();

	void Process();

private:
	RingSrc *src_;
	PulseSink *sink_;
	Player *player_;

};

#endif /* AUDIO_DECODER_H_ */
