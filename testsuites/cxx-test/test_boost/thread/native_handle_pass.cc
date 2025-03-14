//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
// Copyright (C) 2011 Vicente J. Botet Escriba
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// <boost/thread/thread.hpp>

// class thread

// native_handle_type native_handle();

#include <boost/thread/thread_only.hpp>
#include <new>
#include <cstdlib>
#include <cassert>
#include <boost/detail/lightweight_test.hpp>

class G
{
  int alive_;
public:
  static int n_alive;
  static bool op_run;

  G() :
    alive_(1)
  {
    ++n_alive;
  }
  G(const G& g) :
    alive_(g.alive_)
  {
    ++n_alive;
  }
  ~G()
  {
    alive_ = 0;
    --n_alive;
  }

  void operator()()
  {
    BOOST_TEST(alive_ == 1);
    //BOOST_TEST(n_alive == 1);
    op_run = true;
  }
};

int G::n_alive = 0;
bool G::op_run = false;

int test_test_boost_thread_native_handle_pass()
{
  {
    boost::thread t0( (G()));
    // boost::thread::native_handle_type hdl =
    (void)t0.native_handle();
    t0.join();
  }

  return boost::report_errors();
}

