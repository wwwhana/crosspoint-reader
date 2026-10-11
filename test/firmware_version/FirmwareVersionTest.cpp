#include <FirmwareVersion.h>
#include <gtest/gtest.h>

using firmware_version::isNewer;

TEST(FirmwareVersion, UpstreamSegmentsCompareNumerically) {
  EXPECT_TRUE(isNewer("1.6.6", "1.6.5"));
  EXPECT_TRUE(isNewer("1.10.0", "1.9.9"));
  EXPECT_TRUE(isNewer("2.0.0", "1.6.5-x3"));
  EXPECT_FALSE(isNewer("1.6.4-x9", "1.6.5"));
}

TEST(FirmwareVersion, ModNumberOrdersBuildsOfTheSameUpstreamVersion) {
  EXPECT_TRUE(isNewer("1.6.5-x2", "1.6.5-x1"));
  EXPECT_TRUE(isNewer("1.6.5-x10", "1.6.5-x9"));
  EXPECT_TRUE(isNewer("1.6.5-x1", "1.6.5"));
  EXPECT_FALSE(isNewer("1.6.5-x1", "1.6.5-x2"));
  EXPECT_FALSE(isNewer("1.6.5", "1.6.5-x1"));
  EXPECT_FALSE(isNewer("1.6.5-x1", "1.6.5-x1"));
}

TEST(FirmwareVersion, ReleaseCandidateTakesTheFinalBuildOfTheSameVersion) {
  EXPECT_TRUE(isNewer("1.6.5-x1", "1.6.5-x1-rc+71c2cb8"));
  EXPECT_TRUE(isNewer("1.6.5", "1.6.5-rc+71c2cb8"));
  EXPECT_FALSE(isNewer("1.6.5-x1", "1.6.5-x2-rc+71c2cb8"));
}

TEST(FirmwareVersion, BoardSuffixesAreNotModNumbers) {
  EXPECT_FALSE(isNewer("1.6.5-x4pro", "1.6.5"));
  EXPECT_FALSE(isNewer("1.6.5-x4c", "1.6.5"));
  EXPECT_TRUE(isNewer("1.6.5-x2", "1.6.5-x1-x4pro"));
  EXPECT_FALSE(isNewer("1.6.5-x1", "1.6.5-x1-x4c"));
}

TEST(FirmwareVersion, UnparsableVersionsReadAsZero) {
  EXPECT_FALSE(isNewer("", "1.6.5"));
  EXPECT_TRUE(isNewer("1.6.5", "garbage"));
  EXPECT_FALSE(isNewer(nullptr, nullptr));
}
