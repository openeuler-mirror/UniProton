#include <bits/c++config.h>
#include <testsuite_counter_type.h>

namespace __gnu_test 
{
  int counter_type::default_count = 0;
  int counter_type::specialize_count = 0;
  int counter_type::copy_count = 0;
  int counter_type::copy_assign_count = 0;
  int counter_type::less_compare_count = 0;

#if __cplusplus >= 201103L
  int counter_type::move_count = 0;
  int counter_type::move_assign_count = 0;
#endif
  int counter_type::destructor_count = 0;
}