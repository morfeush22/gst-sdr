#include "src/Player.h"
#include "src/FakeSink.h"
#include "src/FileSrc.h"
#include "src/RingSrc.h"
#include "src/PulseSink.h"

int main() {
	RingSrc *src = new RingSrc("./player_unittest_file.aac");
	PulseSink *sink = new PulseSink();

	Player player(src, sink, 44100);
	player.Process();

	delete sink;
	delete src;
}
