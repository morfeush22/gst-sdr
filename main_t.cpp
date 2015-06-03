#include "AudioDecoder/src/file_src.h"
#include "AudioDecoder/src/player.h"
#include "AudioDecoder/src/ogg_sink.h"

using namespace std;

int main() {
	FileSrc *src = new FileSrc("./test/testdata/player_unittest_file.aac");
	OggSink *sink = new OggSink("./test/testdata/player_unittest_file.ogg");
	Player *player = new Player(src);

	player->AddSink(sink);
	player->Process();

	delete player;
	delete src;
	delete sink;
}
