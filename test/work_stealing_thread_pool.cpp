#define BOOST_TEST_MODULE WorkStealingThreadPoolTest
#include <boost/test/unit_test.hpp>
#include <future>
#include <memory>
#include <chrono>
#include <thread>

#include "work_stealing_thread_pool.hpp"

using namespace WorkStealingThreadPool;

int test_function(int a, int b){
    std::this_thread::sleep_for(std::chrono::seconds(2));
    return a+b;
}

BOOST_AUTO_TEST_CASE(WSTP)
{
    auto wstp = work_stealing_thread_pool();
    std::vector<std::future<int>> results;
    unsigned calls = 32;
    for(unsigned i=0; i<calls; ++i){
        results.push_back(wstp.submit(test_function, 1, 2));
    }

    for(auto& fut : results){
        fut.wait();
    }

    for(auto& fut : results){
        auto f = fut.get();
        assert(f == 3);
    }
}

