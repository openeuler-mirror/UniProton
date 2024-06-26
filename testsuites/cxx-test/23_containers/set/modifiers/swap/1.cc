// Copyright (C) 2004-2020 Free Software Foundation, Inc.
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

#include <set>
#include <testsuite_hooks.h>
 
struct T { int i; };

// T must be LessThanComparable to pass concept-checks
static bool operator<(T l, T r) { return l.i < r.i; }

static int swap_calls;

namespace std
{
  template<> 
    void 
    set<T>::swap(set<T>&) 
    { ++swap_calls; }
}

// Should use set specialization for swap.
static void test01()
{
  std::set<T> A;
  std::set<T> B;
  swap_calls = 0;
  std::swap(A, B);
  VERIFY(1 == swap_calls);
}

// Should use set specialization for swap.
static void test02()
{
  using namespace std;
  set<T> A;
  set<T> B;
  swap_calls = 0;
  swap(A, B);
  VERIFY(1 == swap_calls);
}

// See c++/13658 for background info.
int test_23_containers_set_modifiers_swap_1()
{
  test01();
  test02();
  return 0;
}
