#include <gtest/gtest.h>

#include "process/ProcessInfo.h"

using namespace srm;

TEST(ProcessParser, ParsesStatWithParenthesesInName) {
    const std::string text =
        "1234 (my (weird) proc) S 1 1234 1234 0 -1 0 100 0 0 0 10 5 0 0 20 0 100 0 12345";

    const ProcessStat stat = parseProcessStat(text);

    ASSERT_TRUE(stat.valid);
    EXPECT_EQ(stat.pid, 1234);
    EXPECT_EQ(stat.name, "my (weird) proc");
    EXPECT_EQ(stat.state, "S");
    EXPECT_EQ(stat.parentPid, 1);
    EXPECT_EQ(stat.utime, 10u);
    EXPECT_EQ(stat.stime, 5u);
    EXPECT_EQ(stat.threadCount, 100);
    EXPECT_EQ(stat.startTicks, 12345u);
}

TEST(ProcessParser, ParsesSimpleStat) {
    const std::string text =
        "42 (bash) R 1 42 42 34816 0 0 0 0 0 0 12 3 0 0 20 0 1 0 555";

    const ProcessStat stat = parseProcessStat(text);
    ASSERT_TRUE(stat.valid);
    EXPECT_EQ(stat.pid, 42);
    EXPECT_EQ(stat.name, "bash");
    EXPECT_EQ(stat.state, "R");
    EXPECT_EQ(stat.threadCount, 1);
    EXPECT_EQ(stat.startTicks, 555u);
}

TEST(ProcessParser, RejectsMalformedStat) {
    EXPECT_FALSE(parseProcessStat("").valid);
    EXPECT_FALSE(parseProcessStat("1234 (name) S 1 2 3\n").valid);
    EXPECT_FALSE(parseProcessStat("no parentheses here").valid);
}
