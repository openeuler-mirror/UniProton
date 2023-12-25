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

static char cstr[] = "/tmp/";
static const std::filesystem::path filename = cstr;

static void
test01()
{
  std::filebuf fb;
  fb.open(filename, std::ios::in);
  VERIFY( fb.is_open() );
}

static void
test02() // compile-only
{
  std::filebuf fb;
  fb.open(cstr, std::ios::in);	// PR libstdc++/83025
}

int
test_27_io_basic_filebuf_open_char_path()
{
  test01();
  return 0;
}
