#include "src/ring_buffer.h"
#include "src/fake_sink.h"
#include "src/file_sink.h"
#include "src/file_src.h"
#include "src/player.h"
#include "src/pulse_sink.h"
#include "src/ring_src.h"
#include "src/blocking_ring_buffer.h"
#include "file_wrapper.h"
#include <iostream>
#include <unistd.h>

using namespace std;

struct Data {
	void *player;
	void *sink;
};

static gboolean AddCb(gpointer data) {
	cout << "###################adding sink###################" << endl;

	Player *player = (Player *)((Data *)data)->player;
	FileSink *sink = (FileSink *)((Data *)data)->sink;
	player->AddSink(sink);

	cout << "###################sink added###################" << endl;
	return FALSE;
}

static gboolean RemCb(gpointer data) {
	cout << "###################removing sink###################" << endl;

	Player *player = (Player *)((Data *)data)->player;
	FileSink *sink = (FileSink *)((Data *)data)->sink;
	player->RemoveSink(sink);

	cout << "###################sink removed###################" << endl;

	return FALSE;
}

#define BYTES 96000	//96kB

static FileWrapper *file_wrapper = new FileWrapper("./player_unittest_file.aac", BYTES);
static const char *const *start = file_wrapper->GetCurrentChunkPointer();

static void *ReadingThread(void *data) {
	while(1) {
		RingSrc *src = (RingSrc *)data;

		int size = file_wrapper->GetNextChunk();
		char *curr_ptr = const_cast<char *>(*start);

		src->Write(curr_ptr, size);

		sleep(1);
	}
	return NULL;
}

uint16_t FakeSink::count_ = 0;

int main() {
	pthread_t thread;
	pthread_attr_t attr;

	RingSrc *src = new RingSrc(0.2);
	PulseSink *sink = new PulseSink();
	FileSink *new_sink = new FileSink("./test.raw");

	pthread_attr_init(&attr) ;
	pthread_create(&thread, &attr, ReadingThread, src) ;

	Player player(src);
	player.AddSink(sink);

	Data data;
	data.player = &player;
	data.sink = new_sink;

	g_timeout_add_seconds(1, AddCb, &data);
	g_timeout_add_seconds(5, RemCb, &data);

	player.Process();

	delete file_wrapper;
	delete sink;
	delete src;
}
