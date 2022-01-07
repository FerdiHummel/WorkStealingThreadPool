#define BOOST_TEST_MODULE WorkStealingThreadPoolTest
#include <boost/test/unit_test.hpp>
#include <future>
#include <memory>

#include "work_stealing_thread_pool.hpp"

using namespace WorkStealingThreadPool;

int test_function(int a, int b){
    return a+b;
}

BOOST_AUTO_TEST_CASE(WSTP)
{
    auto wstp = work_stealing_thread_pool();
    auto res = wstp.submit(test_function, 1, 2);
    res.wait();
    auto r = res.get();
    std::cout<< r << std::endl;
}