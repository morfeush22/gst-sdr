#include "audio_decoder.h"
#include "file_wrapper.h"
#include <iostream>
#include <unistd.h>

using namespace std;

#define BYTES 2000*10	//20kB, dlugosc bufora dla FileWrappera

static FileWrapper *file_wrapper = new FileWrapper("./player_unittest_file.aac", BYTES);
static const char *const *start = file_wrapper->GetCurrentChunkPointer();

static void *ReadingThread(void *data) {
	while(1) {
		AudioDecoder *ad = reinterpret_cast<AudioDecoder *>(data);

		//czytamy z FileWrappera
		int size = file_wrapper->GetNextChunk();
		//FileWrapper czyta w charach, wiec konieczne sa casty
		float *curr_ptr = reinterpret_cast<float *>(const_cast<char *>(*start));

		//piszemy do AudioDecodera, dzielimy przez sizeof, bo FileWrapper czyta charami
		ad->Write(curr_ptr, size/sizeof(float));

		cout << "########## WRITE ##########" << endl;

		if(!size)	//jesli ostatnie dane, informujemy o tym AudioDecoder, by mogl wykonac procedury czyszczace
			ad->LastFrame();

		usleep(700000);	//0.7s 'snu'
	}
	return NULL;
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

	//uruchamiamy processing audio
	audio_decoder.Process();

	//czyscimy, po zakonczeniu odtwarzania
	delete file_wrapper;

	cout << "all done..." << endl;

	return 0;
}
