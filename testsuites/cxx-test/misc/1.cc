#include<stdio.h>
#include "locale.h"
#include <iostream>
#include <stdlib.h>
#include <sys/time.h>
#include <bits/alltypes.h>
using namespace std;

class cs
{
public:
    explicit cs(int i) :i_(i)
    { 
        printf("cs constructor: %d\n",  i);
    }
    ~cs()
    { 
        printf("cs destructor:i %d\n", i_);
    }
private:
    int i_;
};

static void test_func3()
{
    cs c(33);
    cs c2(332);

    throw 3;

    cs c3(333);
    printf("test func3\n");
}

static void test_func3_2()
{
    cs c(32);
    cs c2(322);

    test_func3();

    cs c3(323);

    test_func3();
}

static void test_func2()
{
    cs c(22);

    printf("test func2\n");
    try {
        test_func3_2();
        cs c2(222);
    } catch (int) {
        printf("catch 2\n");
    }
}

static void test_except2()
{
    printf("test func1\n");
    try {
        test_func2();
    } catch (...) {
        printf("catch 1\n");
    }
}

static void test_except1()
{
    try {
        throw 1;
    } catch(int x) {
        printf("caught except\n");
    }

}

class testnew
{
private:
    static int cnt;
public:
    testnew();
    ~testnew();

    int getCnt();
};

int testnew::cnt = 0;

testnew::testnew()
{
    cnt++;
    printf("testnew\n");
}

testnew::~testnew()
{
    printf("~testnew\n");
}

int testnew::getCnt()
{
    return cnt;
}

static testnew __testnew;

static void test_static()
{
    if (__testnew.getCnt() == 1) {
        printf("test_static success \n");
    } else {
        printf("test_static failed, cnt:%d\n", __testnew.getCnt());
    }
}

static void test_cout()
{
    std::cout << "TESTCOUT:" << 123 << std::endl;
}

int test_misc_1(void)
{
    test_except1();
    test_except2();
    test_static();
    test_cout();
    return 0;
}
