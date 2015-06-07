#include "AudioDecoder/src/player.h"
#include "AudioDecoder/src/fake_sink.h"
#include "AudioDecoder/src/file_src.h"
#include "AudioDecoder/src/ring_src.h"
#include "AudioDecoder/src/pulse_sink.h"
#include "AudioDecoder/src/ogg_sink.h"
#include "audio_decoder.h"
#include "gtest/gtest.h"
#include <fstream>
#include <string>

using namespace std;

struct Data {
	void *player;
	void *sink;
};

static void SaveTags(const std::map<const std::string, std::string> *tags_map, void *other_data) {
	map<const string, string> *other_tags_map = reinterpret_cast<map<const string, string> *>(other_data);
	*other_tags_map = *tags_map;
}

static gboolean RemoveSink(gpointer data) {
	Player *player = reinterpret_cast<Player *>(reinterpret_cast<Data *>(data)->player);
	PulseSink *sink = reinterpret_cast<PulseSink *>(reinterpret_cast<Data *>(data)->sink);
	player->RemoveSink(sink);

	return FALSE;
}

TEST(PlayerTestBytesTest, number_of_processed_bytes) {
	remove("./test/testdata/player_unittest_file.raw");
	system("gst-launch-1.0 filesrc location=./test/testdata/player_unittest_file.aac \
			! id3demux \
			! aacparse \
			! faad \
			! audioconvert \
			! pitch \
			! filesink location=./test/testdata/player_unittest_file.raw \
			1>/dev/null");

	uint32_t size;
	ifstream in_file("./test/testdata/player_unittest_file.raw", ifstream::binary);
	ASSERT_TRUE(in_file.good()) << "TESTING CODE FAILED... could not load data";

	in_file.seekg(0, ios::end);
	size = in_file.tellg();

	in_file.close();

	FileSrc *src = new FileSrc("./test/testdata/player_unittest_file.aac");
	FakeSink *sink = new FakeSink();

	Player *player = new Player(src);
	player->AddSink(sink);
	player->Process();

	EXPECT_EQ(size, sink->bytes_returned());

	delete player;
	delete sink;
	delete src;
}

TEST(PlayerTestOggTest, save_to_ogg_test) {
	remove("./test/testdata/player_unittest_file_expected_results.ogg");
	system("gst-launch-1.0 filesrc location=./test/testdata/player_unittest_file.aac \
			! id3demux \
			! aacparse \
			! faad \
			! audioconvert \
			! pitch \
			! vorbisenc \
			! oggmux \
			! filesink location=./test/testdata/player_unittest_file_expected_results.ogg \
			1>/dev/null");

	uint32_t size_er;
	uint32_t size_r;

	FileSrc *src = new FileSrc("./test/testdata/player_unittest_file.aac");
	OggSink *sink = new OggSink("./test/testdata/player_unittest_file.ogg");
	remove("./test/testdata/player_unittest_file.ogg");

	Player *player = new Player(src);
	player->AddSink(sink);
	player->Process();

	ifstream in_file_er("./test/testdata/player_unittest_file_expected_results.ogg", ifstream::binary);
	ASSERT_TRUE(in_file_er.good()) << "TESTING CODE FAILED... could not load expected data";

	ifstream in_file_r("./test/testdata/player_unittest_file.ogg", ifstream::binary);
	ASSERT_TRUE(in_file_r.good()) << "TESTING CODE FAILED... could not load data";

	in_file_er.seekg(0, ios::end);
	size_er = in_file_er.tellg();
	in_file_er.close();

	in_file_r.seekg(0, ios::end);
	size_r = in_file_r.tellg();
	in_file_r.close();

	ASSERT_EQ(size_er, size_r) << "TESTING CODE FAILED... file size mismatch";

	delete player;
	delete sink;
	delete src;
}

TEST(PlayerTest10sPlay, play_10s_of_audio) {
	FileSrc *src = new FileSrc("./test/testdata/player_unittest_file.aac");
	NullSink *null_sink = new NullSink();
	PulseSink *sink = new PulseSink();

	Player *player = new Player(src);

	Data data;
	data.player = player;
	data.sink = sink;

	player->AddSink(null_sink);
	player->AddSink(sink);
	g_timeout_add_seconds(10, RemoveSink, &data);

	player->Process();

	delete player;
	delete sink;
	delete null_sink;
	delete src;
}

TEST(PlayerTestTagsTest, tags_returned) {
	ifstream in_file("./test/testdata/player_unittest_tags_expected.txt");
	ASSERT_TRUE(in_file.good()) << "TESTING CODE FAILED... could not load data";

	FileSrc *src = new FileSrc("./test/testdata/player_unittest_file.aac");
	FakeSink *sink = new FakeSink();

	map<const string, string> temp;
	map<const string, string>::iterator it;

	Player *player = new Player(src);
	player->AddSink(sink);
	player->RegisterTagsMapCallback(SaveTags, &temp);
	player->Process();

	string line;

	while(getline(in_file, line)) {
		it = temp.find(line);
		ASSERT_TRUE(it != temp.end());
		getline(in_file, line);
		EXPECT_FALSE(line.compare(it->second));
	}

	delete player;
	delete sink;
	delete src;
}

TEST(PlayerTestGetSrc, src_getter) {
	FileSrc *src = new FileSrc("./test/testdata/player_unittest_file.aac");
	FakeSink *sink = new FakeSink();

	Player *player = new Player(src);
	player->AddSink(sink);

	EXPECT_EQ(src, player->abstract_src());

	delete player;
	delete sink;
	delete src;
}

TEST(PlayerTestTeeTest, number_of_src_pads) {
	FileSrc *src = new FileSrc("./test/testdata/player_unittest_file.aac");
	FakeSink *sink = new FakeSink();
	uint32_t result = 1;

	Player *player = new Player(src);
	player->AddSink(sink);

	EXPECT_EQ(result, sink->num_src_pads());

	delete player;
	delete sink;
	delete src;
}

TEST(PlayerTestAddSink, incrementation_of_sink_pads) {
	FileSrc *src = new FileSrc("./test/testdata/player_unittest_file.aac");
	FakeSink *sink1 = new FakeSink();
	FakeSink *sink2 = new FakeSink();
	FakeSink *sink3 = new FakeSink();
	uint32_t result = 3;

	Player *player = new Player(src);
	EXPECT_EQ(sink1, player->AddSink(sink1));
	EXPECT_EQ(sink2, player->AddSink(sink2));
	EXPECT_EQ(sink3, player->AddSink(sink3));

	EXPECT_EQ(result, sink1->num_src_pads());

	delete player;
	delete sink1;
	delete sink2;
	delete sink3;
	delete src;
}

TEST(PlayerTestRemoveSink, decrementation_of_sink_pads) {
	FileSrc *src = new FileSrc("./test/testdata/player_unittest_file.aac");
	FakeSink *sink1 = new FakeSink();
	FakeSink *sink2 = new FakeSink();
	FakeSink *sink3 = new FakeSink();
	uint32_t result = 2;

	Player *player = new Player(src);
	player->AddSink(sink1);
	player->AddSink(sink2);
	player->AddSink(sink3);

	player->RemoveSink(sink3);

	EXPECT_EQ(result, sink1->num_src_pads());

	delete player;
	delete sink1;
	delete sink2;
	delete sink3;
	delete src;
}

TEST(PlayerTestPlaybackSpeed, setting_playback_speed) {
	FileSrc *src = new FileSrc("./test/testdata/player_unittest_file.aac");
	FakeSink *sink = new FakeSink();

	Player *player = new Player(src);
	player->AddSink(sink);
	player->set_playback_speed(0.9);

	EXPECT_NEAR(0.9, sink->playback_speed(), 1e-3);

	delete player;
	delete sink;
	delete src;
}

TEST(SinkTestLinkage, test_of_linkage) {
	FileSrc *src = new FileSrc("./test/testdata/player_unittest_file.aac");
	FakeSink *sink = new FakeSink();

	EXPECT_FALSE(sink->linked());

	Player *player = new Player(src);
	player->AddSink(sink);

	EXPECT_TRUE(sink->linked());

	player->RemoveSink(sink);

	EXPECT_FALSE(sink->linked());

	delete player;
	delete sink;
	delete src;
}

TEST(RingSrcTestDecrementRatio, decrementation_of_ratio) {
	RingSrc *src = new RingSrc(0.2, 100);
	FakeSink *sink = new FakeSink();

	Player *player = new Player(src);
	player->AddSink(sink);
	src->DecrementRatio(player);

	EXPECT_LT(sink->playback_speed(), 1.0);

	delete player;
	delete sink;
	delete src;
}

TEST(RingSrcTestIncrementRatio, incrementation_of_ratio) {
	RingSrc *src = new RingSrc(0.2, 100);
	FakeSink *sink = new FakeSink();

	Player *player = new Player(src);
	player->AddSink(sink);
	src->IncrementRatio(player);

	EXPECT_GT(sink->playback_speed(), 1.0);

	delete player;
	delete sink;
	delete src;
}
