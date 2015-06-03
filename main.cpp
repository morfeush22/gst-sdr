#include "audio_decoder.h"
#include "file_wrapper.h"
#include "AudioDecoder/src/pulse_sink.h"
#include <iostream>
#include <unistd.h>

using namespace std;

#define BYTES 2000*10	//20kB, dlugosc bufora dla FileWrappera

static FileWrapper *file_wrapper = new FileWrapper("./test/testdata/player_unittest_file.aac", BYTES);
static const char *const *start = file_wrapper->GetCurrentChunkPointer();

static void *ReadingThread(void *data) {
	while(1) {
		AudioDecoder *ad = reinterpret_cast<AudioDecoder *>(data);

		//czytamy z FileWrappera
		int size = file_wrapper->GetNextChunk();
		//FileWrapper czyta w charach, wiec konieczne sa casty
		uint8_t *curr_ptr = (uint8_t *)(*start);

		//piszemy do AudioDecodera
		ad->Write(curr_ptr, size);

		cout << "########## WRITE ##########" << endl;

		if(!size)	//jesli ostatnie dane, informujemy o tym AudioDecoder, by mogl wykonac procedury czyszczace
			ad->LastFrame();

		usleep(700000);	//0.7s 'snu'
	}
	return NULL;
}

struct Data {
	void *player;
	void *sink;
};

static gboolean AddCb(gpointer data) {
	cout << "###################adding sink###################" << endl;

	AudioDecoder *ad = (AudioDecoder *)((Data *)data)->player;
	PulseSink *sink = (PulseSink *)((Data *)data)->sink;
	ad->AddSink(sink);

	cout << "###################sink added###################" << endl;
	return FALSE;
}

static gboolean RemCb(gpointer data) {
	cout << "###################removing sink###################" << endl;

	AudioDecoder *ad = (AudioDecoder *)((Data *)data)->player;
	PulseSink *sink = (PulseSink *)((Data *)data)->sink;
	ad->RemoveSink(sink);

	cout << "###################sink removed###################" << endl;

	return FALSE;
}

int main() {
	//zmienne dla watku piszacego do AudioDecodera
	pthread_t thread;
	pthread_attr_t attr;

	AudioDecoder audio_decoder(0.1, 50*10000);	//pierwszy parametr to threshold, powinien zawierac sie w (0, 0.5), stanowi prog, dla ktorego odbywal sie bedzie resampling
												//drugi to wielkosc wewnetrznego bufora kolowego, moze nie dzialac, gdy bedzie za mala;
												//w pliku ring_src.cpp jest pare parametrow do konfiguracji, w tym przypadku dlugosc bufora powinna byc podzielna przez 250 i 10000

	//uruchamiamy watek piszacy
	pthread_attr_init(&attr);
	pthread_create(&thread, &attr, ReadingThread, &audio_decoder);

	PulseSink *new_sink = new PulseSink();

	Data data;
	data.player = &audio_decoder;
	data.sink = new_sink;

	g_timeout_add_seconds(10, AddCb, &data);
	g_timeout_add_seconds(30, RemCb, &data);
	//audio_decoder.AddSink(new_sink);

	//uruchamiamy processing audio
	audio_decoder.Process();

	//czyscimy, po zakonczeniu odtwarzania
	delete new_sink;
	delete file_wrapper;

	cout << "all done..." << endl;

	return 0;
}
