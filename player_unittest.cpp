#include "src/player.h"

#include "gtest/gtest.h"
#include <fstream>
#include <string>
#include "src/fake_sink.h"
#include "src/file_src.h"
#include "src/ring_src.h"

using namespace std;

TEST(PlayerTestBytesTest, number_of_processed_bytes) {
	uint32_t size;
	ifstream in_file("./player_unittest_file.raw", ifstream::binary);
	ASSERT_TRUE(in_file.good()) << "TESTING CODE FAILED... could not load data";

	in_file.seekg(0, ios::end);
	size = in_file.tellg();

	in_file.close();

	FileSrc *src = new FileSrc("./player_unittest_file.aac");
	FakeSink *sink = new FakeSink();

	Player player(src);
	player.AddSink(sink);
	player.Process();

	EXPECT_EQ(size, sink->GetBytesReturned());

	delete sink;
	delete src;
}

TEST(PlayerTestTagsTest, tags_returned) {
	ifstream in_file("./player_unittest_tags_expected.txt");
	ASSERT_TRUE(in_file.good()) << "TESTING CODE FAILED... could not load data";

	FileSrc *src = new FileSrc("./player_unittest_file.aac");
	FakeSink *sink = new FakeSink();

	Player player(src);
	player.AddSink(sink);
	player.Process();

	std::map<const char *, char *, PlayerHelpers::CmpStr>::const_iterator it;
	string line;

	while(getline(in_file, line)) {
		it = player.GetTagsMap()->find(line.c_str());
		ASSERT_TRUE(it != player.GetTagsMap()->end());
		getline(in_file, line);
		EXPECT_FALSE(strcmp(it->second, line.c_str()));
	}

	delete sink;
	delete src;
}

