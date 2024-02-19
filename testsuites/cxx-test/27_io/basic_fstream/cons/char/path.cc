// Copyright (C) 2017-2020 Free Software Foundation, Inc.
//
// This file is part of the GNU ISO C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the
// Free Software Foundation; either version 3, or (at your option)
// any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along
// with this library; see the file COPYING3.  If not see
// <http://www.gnu.org/licenses/>.

// { dg-options "-std=gnu++17 -lstdc++fs" }
// { dg-do run { target c++17 } }
// { dg-require-fileio "" }
// { dg-require-filesystem-ts "" }

#include <fstream>
#include <filesystem>
#include <testsuite_hooks.h>
namespace {
const std::filesystem::path filename = "/tmp/basic_fstream_cons_char_path";

void
test01()
{
  std::fstream f(filename);
  VERIFY( f.is_open() );
}

void
test02()
{
  std::fstream f(filename, std::ios::out);
  VERIFY( f.is_open() );
}

void
prepare()
{
  std::fstream f(filename, std::ios::out);
  f.close();
}
using std::is_constructible_v;
// PR libstdc++/83025
static_assert(is_constructible_v<std::fstream, char*>);
static_assert(is_constructible_v<std::fstream, char*, std::ios::openmode>);
};

int
test_27_io_basic_fstream_cons_char_path()
{
  prepare();
  test01();
  test02();
  return 0;
}
