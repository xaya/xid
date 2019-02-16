// Copyright (C) 2019 The Xaya developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "time.hpp"

#include <gtest/gtest.h>

#include <cstdint>
#include <ctime>

namespace xid
{
namespace
{

TEST (TimeTests, UnixTimestamps)
{
  struct TestCase
  {
    uint64_t timestamp;

    int year;
    int month;
    int day;

    int hour;
    int min;
    int sec;
  };

  const TestCase tests[] = {
    {0, 1970, 1, 1, 0, 0, 0},
    {1000000000, 2001, 9, 9, 1, 46, 40},
    {5000000000, 2128, 6, 11, 8, 53, 20},
  };

  for (const auto& t : tests)
    {
      const std::time_t time = TimeFromUnix (t.timestamp);
      EXPECT_EQ (TimeToUnix (time), t.timestamp);

      const auto& cal = *std::gmtime (&time);
      EXPECT_EQ (1900 + cal.tm_year, t.year);
      EXPECT_EQ (1 + cal.tm_mon, t.month);
      EXPECT_EQ (cal.tm_mday, t.day);
      EXPECT_EQ (cal.tm_hour, t.hour);
      EXPECT_EQ (cal.tm_min, t.min);
      EXPECT_EQ (cal.tm_sec, t.sec);
    }
}

} // anonymous namespace
} // namespace xid
