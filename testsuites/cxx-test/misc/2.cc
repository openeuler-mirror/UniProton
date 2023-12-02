#include <boost/circular_buffer.hpp>
#include <stdio.h>

#define BOOST_CHECK(fn)                                     \
    do {                                                    \
    if (!(fn)) {                                            \
        printf("%s:%d: %s: Assertion '%s' failed.\n",       \
            __FILE__, __LINE__, __PRETTY_FUNCTION__, #fn);  \
        }                                                   \
    } while (false)

using namespace boost;
static void iterator_increment_test() {

    circular_buffer<int> cb(10, 1);
    cb.push_back(2);
    circular_buffer<int>::iterator it1 = cb.begin();
    circular_buffer<int>::iterator it2 = cb.begin() + 5;
    circular_buffer<int>::iterator it3 = cb.begin() + 9;
    it1++;
    it2++;
    ++it3;

    BOOST_CHECK(it1 == cb.begin() + 1);
    BOOST_CHECK(it2 == cb.begin() + 6);
    BOOST_CHECK(it3 == cb.end());
}

static void iterator_decrement_test() {

    circular_buffer<int> cb(10, 1);
    cb.push_back(2);
    circular_buffer<int>::iterator it1= cb.end();
    circular_buffer<int>::iterator it2= cb.end() - 5;
    circular_buffer<int>::iterator it3= cb.end() - 9;
    --it1;
    it2--;
    --it3;

    BOOST_CHECK(it1 == cb.end() - 1);
    BOOST_CHECK(it2 == cb.end() - 6);
    BOOST_CHECK(it3 == cb.begin());
}
int test_misc_2()
{
    iterator_increment_test();
    iterator_decrement_test();
    printf("\rtest_misc_2 finish\n");
    return 0;
}
