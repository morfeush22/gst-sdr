/*
 * audio_decoder.h
 *
 *  Created on: May 23, 2015
 *      Author: morfeush22
 */

#ifndef AUDIO_DECODER_H_
#define AUDIO_DECODER_H_

#include "AudioDecoder/src/abstract_sink.h"
#include "AudioDecoder/src/player.h"
#include "AudioDecoder/src/fake_sink.h"
#include "AudioDecoder/src/ring_src.h"

class AudioDecoder {
public:
	AudioDecoder(float, size_t);
	virtual ~AudioDecoder();

	void RemoveSink(AbstractSink *);
	AbstractSink *AddSink(AbstractSink *);

	void Write(uint8_t *, size_t);
	void LastFrame();

	void RegisterTagsMapCallback(TagsMapCallback, void *);

	void Process();

private:
	RingSrc *src_;
	FakeSink *sink_;
	Player *player_;

};

#endif /* AUDIO_DECODER_H_ */
