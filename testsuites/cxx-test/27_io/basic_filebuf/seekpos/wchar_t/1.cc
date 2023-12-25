// Copyright (C) 2003-2020 Free Software Foundation, Inc.
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

// 27.8.1.4 Overridden virtual functions

#include <locale>
#include <fstream>
#include <testsuite_hooks.h>

static void test01()
{
  using namespace std;

  const char* name = "/tmp/basic_filebuf_seekpos_wchar_t_1";

  wfilebuf fb;

  fb.open(name, ios_base::out);
  streampos pos = fb.pubseekoff(0, ios_base::beg);
  fb.sputc(0xf001);

  try
    {
      fb.pubseekpos(pos);
      VERIFY( false );
    }
  catch (std::exception&)
    {
    }
}

int test_27_io_basic_filebuf_seekpos_wchar_t_1()
{
  test01();
  return 0;
}
