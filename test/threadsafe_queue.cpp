#define BOOST_TEST_MODULE ThreadSafeQueueTest
#include <boost/test/unit_test.hpp>
#include <future>
#include <memory>

#include "threadsafe_queue.hpp"

using namespace WorkStealingThreadPool;

void test_function(int pop_threads = 1,
                   int push_threads = 1,
                   int queue_size = 0)
{
    auto q = new threadsafe_queue<int>();
    for(int i=0; i < queue_size; ++i){
        q->push(i);
    }
    std::vector<int> results;
    for(int i=0; i<pop_threads; ++i) {
        results.push_back(0);
    }
    std::vector<std::promise<void>> push_ready, pop_ready;
    std::vector<std::future<void>> push_done;
    std::vector<std::future<bool>> pop_done;
    std::promise<void> go;
    std::shared_future<void> ready(go.get_future());
    try{

        for(int i=0; i<push_threads; ++i) {
            push_ready.push_back(std::promise<void>());
        }

        for(int i=0; i<pop_threads; ++i) {
            pop_ready.push_back(std::promise<void>());
        }

        for(auto& push : push_ready){
            push_done.push_back(std::async(std::launch::async,
                                           [&q, ready, &push]()
                                           {
                                               push.set_value();
                                               ready.wait();
                                               q->push(100);
                                           }));
        }

        for(int i=0; i < pop_ready.size(); ++i){
            pop_done.push_back(std::async(std::launch::async,
                                          [&q, ready, &pop_ready, &results, i]()
                                          {
                                              pop_ready[i].set_value();
                                              ready.wait();
                                              return q->try_pop(results[i]);
                                          }));
        }

        for(auto& push : push_ready){
            push.get_future().wait();
        }

        for(auto& pop: pop_ready){
            pop.get_future().wait();
        }
        go.set_value();
    }
    catch(...){
        go.set_value();
        throw;
    }
}

BOOST_AUTO_TEST_CASE(SingleThreadPushEmptyQueue)
{
    test_function(0);
}

BOOST_AUTO_TEST_CASE(SingleThreadPushFullQueue)
{
    test_function(0, 1, 1000);
}

BOOST_AUTO_TEST_CASE(SingleThreadEmpty)
{
    auto queue = new threadsafe_queue<int>();
    assert(queue->empty());
}

BOOST_AUTO_TEST_CASE(SingleThreadPopEmptyQueue)
{
    test_function(1, 0);
}

BOOST_AUTO_TEST_CASE(SingleThreadPopFullQueue)
{
    test_function(1, 0, 1000);
}

BOOST_AUTO_TEST_CASE(ConcurrentPushAndPopOnEmptyQueue)
{
    test_function();
}

BOOST_AUTO_TEST_CASE(ConcurrentPushAndPopOnFullQueue)
{
    test_function(1,1, 1000);
}

BOOST_AUTO_TEST_CASE(ConcurrentPushOnEmptyQueue)
{
    test_function(0, std::thread::hardware_concurrency());
}

BOOST_AUTO_TEST_CASE(ConcurrentPushOnFullQueue)
{
    test_function(0, std::thread::hardware_concurrency(), 1000);
}

BOOST_AUTO_TEST_CASE(ConcurrentPophOnEmptyQueue)
{
    test_function(std::thread::hardware_concurrency(), 0);
}

BOOST_AUTO_TEST_CASE(ConcurrentPopOnFullQueue)
{
    test_function(std::thread::hardware_concurrency(), 0, 1000);
}


BOOST_AUTO_TEST_CASE(ConcurrentPopOnPartiallyFullQueue)
{
    auto num_threads = std::thread::hardware_concurrency();
    test_function(num_threads, 0, num_threads/2);
}

BOOST_AUTO_TEST_CASE(ConcurrentPushAndSinglePopOnEmptyQueue)
{
    auto num_threads = std::thread::hardware_concurrency();
    test_function(1, num_threads-1, num_threads/2);
}

BOOST_AUTO_TEST_CASE(ConcurrentPushAndSinglePopOnFullQueue)
{
    auto num_threads = std::thread::hardware_concurrency();
    test_function(1, num_threads-1,  1000);
}

BOOST_AUTO_TEST_CASE(ConcurrentPushAndPophOnEmptyQueue)
{
    auto num_threads = std::thread::hardware_concurrency();
    test_function(num_threads/2, num_threads/2);
}

BOOST_AUTO_TEST_CASE(ConcurrentPushAndPophOnFullQueue)
{
    auto num_threads = std::thread::hardware_concurrency();
    test_function(num_threads/2, num_threads/2, 1000);
}