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

// { dg-require-fileio "" }

// 27.8.1.3 filebuf member functions

#include <locale>
#include <fstream>
#include <testsuite_hooks.h>
namespace {
class CloseCvt : public std::codecvt<char, char, std::mbstate_t>
{
public:
  mutable bool unshift_called;

  CloseCvt()
  : unshift_called(false)
  { }

protected:
  bool
  do_always_noconv() const throw()
  { return false; }

  int
  do_encoding() const throw()
  { return -1; }

  std::codecvt_base::result
  do_unshift(std::mbstate_t&, char* to, char*, char*& to_next) const
  {
    unshift_called = true;
    to_next = to;
    return std::codecvt_base::ok;
  }
};

// libstdc++/12790
// basic_filebuf::close() should call codecvt::unshift()
void test01()
{
  using namespace std;

  const char* name = "/tmp/basic_filebuf_close_char_12790_2";

  CloseCvt* cvt = new CloseCvt;
  locale loc(locale::classic(), cvt);

  filebuf fb;
  fb.pubsetbuf(0, 0);
  fb.pubimbue(loc);

  fb.open(name, ios_base::out);
  fb.sputc('a');
  fb.in_avail(); // showmanyc() should have no effect on close().

  VERIFY( !cvt->unshift_called );
  fb.close();
  VERIFY( cvt->unshift_called );
}
}
int test_27_io_basic_filebuf_close_char_12790_2()
{
  test01();
  return 0;
}
