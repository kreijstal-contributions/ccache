// Copyright (C) 2010-2020 Joel Rosdahl and other contributors
//
// See doc/AUTHORS.adoc for a complete list of contributors.
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 3 of the License, or (at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
// more details.
//
// You should have received a copy of the GNU General Public License along with
// this program; if not, write to the Free Software Foundation, Inc., 51
// Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

#include "../src/legacy_util.hpp"

#include "third_party/doctest.h"

#define CHECK_STR_EQ_FREE2(a, b)                                               \
  do {                                                                         \
    CHECK(strcmp((a), (b)) == 0);                                              \
    free(b);                                                                   \
  } while (false)

TEST_SUITE_BEGIN("legacy_util");

TEST_CASE("subst_env_in_string")
{
  char* errmsg;

  x_setenv("FOO", "bar");

  CHECK_STR_EQ_FREE2("bar", subst_env_in_string("$FOO", &errmsg));
  CHECK(!errmsg);

  CHECK_STR_EQ_FREE2("$", subst_env_in_string("$", &errmsg));
  CHECK(!errmsg);

  CHECK_STR_EQ_FREE2("bar bar:bar",
                     subst_env_in_string("$FOO $FOO:$FOO", &errmsg));
  CHECK(!errmsg);

  CHECK_STR_EQ_FREE2("xbar", subst_env_in_string("x$FOO", &errmsg));
  CHECK(!errmsg);

  CHECK_STR_EQ_FREE2("barx", subst_env_in_string("${FOO}x", &errmsg));
  CHECK(!errmsg);

  CHECK(!subst_env_in_string("$surelydoesntexist", &errmsg));
  CHECK_STR_EQ_FREE2("environment variable \"surelydoesntexist\" not set",
                     errmsg);

  CHECK(!subst_env_in_string("${FOO", &errmsg));
  CHECK_STR_EQ_FREE2("syntax error: missing '}' after \"FOO\"", errmsg);
}

TEST_CASE("parse_size_with_suffix")
{
  uint64_t size;
  size_t i;
  struct
  {
    const char* size;
    int64_t expected;
  } sizes[] = {
    {"0", 0},
    {"42", (int64_t)42 * 1000 * 1000 * 1000}, // Default suffix: G

    {"78k", 78 * 1000},
    {"78K", 78 * 1000},
    {"1.1 M", (int64_t)(1.1 * 1000 * 1000)},
    {"438.55M", (int64_t)(438.55 * 1000 * 1000)},
    {"1 G", 1 * 1000 * 1000 * 1000},
    {"2T", (int64_t)2 * 1000 * 1000 * 1000 * 1000},

    {"78 Ki", 78 * 1024},
    {"1.1Mi", (int64_t)(1.1 * 1024 * 1024)},
    {"438.55 Mi", (int64_t)(438.55 * 1024 * 1024)},
    {"1Gi", 1 * 1024 * 1024 * 1024},
    {"2 Ti", (int64_t)2 * 1024 * 1024 * 1024 * 1024},
  };

  for (i = 0; i < ARRAY_SIZE(sizes); ++i) {
    CHECK(parse_size_with_suffix(sizes[i].size, &size));
    CHECK(size == sizes[i].expected);
  }
}

TEST_SUITE_END();
