#include <testsuite_hooks.h>

namespace __gnu_test {
  // This is a moveable class which copies how many times it is copied.
  // This is mainly of use in the containers, where the an element inserted
  // into a container has to be copied once to get there, but we want to check
  // nothing else is copied.
  struct copycounter
  {
    static int copycount;
    int val;
    bool valid;

    copycounter() : val(0), valid(true)
    { }

    copycounter(int inval) : val(inval), valid(true)
    { }

    copycounter(const copycounter& in) : val(in.val), valid(true)
    {
      VERIFY( in.valid == true );
      ++copycount;
    }

    copycounter(copycounter&& in) noexcept
    {
      VERIFY( in.valid == true );
      val = in.val;
      in.valid = false;
      valid = true;
    }

    copycounter&
    operator=(int newval)
    {
      val = newval;
      valid = true;
      return *this;
    }

    bool
    operator=(const copycounter& in)
    {
      VERIFY( in.valid == true );
      ++copycount;
      val = in.val;
      valid = true;
      return true;
    }

    copycounter&
    operator=(copycounter&& in)
    {
      VERIFY(in.valid == true);
      val = in.val;
      in.valid = false;
      valid = true;
      return *this;
    }

    ~copycounter() noexcept
    { valid = false; }
  };

  inline bool
  operator==(const copycounter& lhs, const copycounter& rhs)
  { return lhs.val == rhs.val; }

  inline bool
  operator<(const copycounter& lhs, const copycounter& rhs)
  { return lhs.val < rhs.val; }

  inline void
  swap(copycounter& lhs, copycounter& rhs)
  {
    VERIFY( lhs.valid && rhs.valid );
    int temp = lhs.val;
    lhs.val = rhs.val;
    rhs.val = temp;
  }
}