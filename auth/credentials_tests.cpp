// Copyright (C) 2019 The Xaya developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "credentials.hpp"

#include <xayautil/base64.hpp>

#include <gtest/gtest.h>

namespace xid
{
namespace
{

/* ************************************************************************** */

using CredentialsPasswordTests = testing::Test;

TEST_F (CredentialsPasswordTests, Roundtrip)
{
  Credentials c("domob", "app");
  const std::string sgn = xaya::EncodeBase64 ("signature");
  c.SetSignature (sgn);
  c.SetExpiry (1234);
  c.AddExtra ("foo", "bar");
  c.AddExtra ("abc", "def");

  Credentials cc("domob", "app");
  ASSERT_TRUE (cc.FromPassword (c.ToPassword ()));
  EXPECT_EQ (cc.GetSignature (), sgn);
  EXPECT_TRUE (cc.HasExpiry ());
  EXPECT_EQ (cc.GetExpiry (), 1234);
  EXPECT_EQ (cc.GetExtra (), Credentials::ExtraMap ({
    {"abc", "def"},
    {"foo", "bar"},
  }));
}

TEST_F (CredentialsPasswordTests, DataLengths)
{
  for (int i = 0; i < 100; ++i)
    {
      Credentials c("domob", "app");

      /* Add an extra value with varying length.  With the string length taking
         values from 0 to 99, this ensures that we will also get serialised data
         of various lengths.  In particular, this tests that the base64 padding
         works fine together with proto serialisation.  */
      c.AddExtra ("key", std::string (i, 'x'));
      const auto expected = c.GetExtra ();
      ASSERT_EQ (expected.size (), 1);

      const std::string pwd = c.ToPassword ();
      ASSERT_TRUE (c.FromPassword (pwd));
      EXPECT_EQ (c.GetExtra (), expected);
    }
}

TEST_F (CredentialsPasswordTests, ClearsExistingData)
{
  Credentials c("domob", "app");
  c.SetExpiry (1234);
  ASSERT_TRUE (c.FromPassword (""));
  EXPECT_FALSE (c.HasExpiry ());
}

TEST_F (CredentialsPasswordTests, InvalidBase64)
{
  Credentials c("domob", "app");
  EXPECT_FALSE (c.FromPassword ("abc"));
}

TEST_F (CredentialsPasswordTests, InvalidProto)
{
  Credentials c("domob", "app");
  EXPECT_FALSE (c.FromPassword ("AQ=="));
}

/* ************************************************************************** */

using CredentialsValidateFormatTests = testing::Test;

TEST_F (CredentialsValidateFormatTests, Valid)
{
  Credentials c(u8"äöü foobar", "chat.xaya.io/Service123");
  c.AddExtra ("My.Key.1", "My.Value.1");
  c.AddExtra ("My.Key.2", "My.Value.2");
  EXPECT_TRUE (c.ValidateFormat ());
}

TEST_F (CredentialsValidateFormatTests, InvalidUsername)
{
  Credentials c("do\nmob", "app");
  EXPECT_FALSE (c.ValidateFormat ());
}

TEST_F (CredentialsValidateFormatTests, InvalidApplication)
{
  Credentials c("domob", "app-foo bar");
  EXPECT_FALSE (c.ValidateFormat ());
}

TEST_F (CredentialsValidateFormatTests, InvalidExtraKey)
{
  Credentials c("domob", "app");
  c.AddExtra ("invalid key", "foo");
  EXPECT_FALSE (c.ValidateFormat ());
}

TEST_F (CredentialsValidateFormatTests, InvalidExtraValue)
{
  Credentials c("domob", "app");
  c.AddExtra ("key", "invalid value");
  EXPECT_FALSE (c.ValidateFormat ());
}

/* ************************************************************************** */

using CredentialsAuthMessageTests = testing::Test;

TEST_F (CredentialsAuthMessageTests, Basic)
{
  Credentials c(u8"äöü foobar", "app");
  c.SetSignature (xaya::EncodeBase64 ("signature"));
  EXPECT_EQ (c.GetAuthMessage (),
u8R"(Xid login
äöü foobar
at: app
expires: never
extra:
)");
}

TEST_F (CredentialsAuthMessageTests, WithExpiry)
{
  Credentials c("domob", "app");
  c.SetExpiry (1234);
  EXPECT_EQ (c.GetAuthMessage (),
u8R"(Xid login
domob
at: app
expires: 1234
extra:
)");
}

TEST_F (CredentialsAuthMessageTests, ExtraData)
{
  Credentials c("domob", "app");
  c.AddExtra ("foo", "bar");
  c.AddExtra ("abc", "def");
  EXPECT_EQ (c.GetAuthMessage (),
u8R"(Xid login
domob
at: app
expires: never
extra:
abc=def
foo=bar
)");
}

/* ************************************************************************** */

using CredentialsExpirationTests = testing::Test;

TEST_F (CredentialsExpirationTests, AtTimestamp)
{
  Credentials c("domob", "app");
  EXPECT_FALSE (c.IsExpired (0));
  c.SetExpiry (100);
  EXPECT_FALSE (c.IsExpired (0));
  EXPECT_FALSE (c.IsExpired (100));
  EXPECT_TRUE (c.IsExpired (101));
}

TEST_F (CredentialsExpirationTests, CurrentTime)
{
  /* Since the current time when running the test is not known, this just
     verifies basic behaviour under a very mild assumption of when the test
     is executed.  */

  Credentials c("domob", "app");
  EXPECT_FALSE (c.IsExpired ());

  c.SetExpiry (1000);
  EXPECT_TRUE (c.IsExpired ());

  c.SetExpiry (5000000000);
  EXPECT_FALSE (c.IsExpired ());
}

/* ************************************************************************** */

using CredentialsAccessorsTests = testing::Test;

TEST_F (CredentialsAccessorsTests, Signature)
{
  Credentials c("domob", "app");
  EXPECT_EQ (c.GetSignature (), "");

  const std::string sgn1 = xaya::EncodeBase64 ("foo");
  c.SetSignature (sgn1);
  EXPECT_EQ (c.GetSignature (), sgn1);

  const std::string sgn2 = xaya::EncodeBase64 ("bar");
  c.SetSignature (sgn2);
  EXPECT_EQ (c.GetSignature (), sgn2);
}

TEST_F (CredentialsAccessorsTests, Expiry)
{
  Credentials c("domob", "app");
  EXPECT_FALSE (c.HasExpiry ());
  c.SetExpiry (0);
  EXPECT_TRUE (c.HasExpiry ());
  EXPECT_EQ (c.GetExpiry (), 0);
  c.SetExpiry (1000);
  EXPECT_TRUE (c.HasExpiry ());
  EXPECT_EQ (c.GetExpiry (), 1000);
}

TEST_F (CredentialsAccessorsTests, Extra)
{
  Credentials c("domob", "app");
  EXPECT_TRUE (c.GetExtra ().empty ());

  c.AddExtra ("foo", "bar");
  EXPECT_DEATH (c.AddExtra ("foo", "baz"), "count \\(key\\) == 0");

  c.AddExtra ("abc", "def");
  EXPECT_EQ (c.GetExtra (), Credentials::ExtraMap ({
    {"abc", "def"},
    {"foo", "bar"},
  }));
}

/* ************************************************************************** */

} // anonymous namespace
} // namespace xid
