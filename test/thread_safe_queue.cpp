#include <future>
#include <memory>
#include <chrono>
#include <thread>

#include "gtest/gtest.h"

#include "thread_safe_queue.hpp"

using namespace WorkStealingThreadPool;

void test_function(int pop_threads = 1,
                   int push_threads = 1,
                   int queue_size = 0,
                   std::optional<std::function<void(thread_safe_queue<int>&)>> check_queue = {})
{
    thread_safe_queue<int> q{};
    for(int i=0; i < queue_size; ++i){
        q.push(i);
    }
    std::vector<int> results;
    for(int i=0; i<pop_threads; ++i) {
        results.push_back(i);
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
                                               q.push(100);
                                           }));
        }

        for(int i=0; i < pop_ready.size(); ++i){
            pop_done.push_back(std::async(std::launch::async,
                                          [&q, ready, &pop_ready, &results, i]()
                                          {
                                              pop_ready[i].set_value();
                                              ready.wait();
                                              return q.try_pop(results[i]);
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

    if(check_queue.has_value()){
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        auto f = check_queue.value();
        f(q);
    }
}

TEST(ThreadSafeQueueTest, MoveConstructor)
{
    thread_safe_queue<int> q1{};
    q1.push(1);
    thread_safe_queue<int> q2(std::move(q1));
    assert(q1.empty());
    assert(!q2.empty());
    auto result = 0;
    auto success = q2.try_pop(result);
    assert(result == 1);
    assert(success);
}

TEST(ThreadSafeQueueTest, MoveAssign)
{
    thread_safe_queue<int> q1{};
    q1.push(1);
    thread_safe_queue<int> q2 = std::move(q1);
    assert(q1.empty());
    assert(!q2.empty());
    auto result = 0;
    auto success = q2.try_pop(result);
    assert(result == 1);
    assert(success);
}


TEST(ThreadSafeQueueTest, SingleThreadPushEmptyQueue)
{
    auto f = [&](thread_safe_queue<int>& q){ assert(!q.empty());};
    test_function(0, 1, 0, f);
}

TEST(ThreadSafeQueueTest, SingleThreadPushFullQueue)
{
    auto f = [&](thread_safe_queue<int>& q){
        for(unsigned  i=0; i<6; ++i){
            int val = -1;
            auto success = q.try_pop(val);
            assert(success);
            assert(val != -1);
        }};
    test_function(0, 1, 5, f);
}

TEST(ThreadSafeQueueTest, SingleThreadEmpty)
{
    auto queue = new thread_safe_queue<int>();
    assert(queue->empty());
}

TEST(ThreadSafeQueueTest, SingleThreadPopEmptyQueue)
{
    test_function(1, 0, 0);
}

TEST(ThreadSafeQueueTest, SingleThreadPopFullQueue)
{
    auto pop_threads = 1;
    auto queue_size = 100;
    auto f = [&](thread_safe_queue<int>& q){
        for(unsigned  i=0; i<queue_size-pop_threads; ++i){
            int val = -1;
            auto success = q.try_pop(val);
            assert(success);
            assert(val != -1);
        }};
    test_function(pop_threads, 0, queue_size, f);
}

TEST(ThreadSafeQueueTest, ConcurrentPushAndPopOnEmptyQueue)
{
    test_function();
}

TEST(ThreadSafeQueueTest, ConcurrentPushAndPopOnFullQueue)
{
    test_function(1,1, 1000);
}

TEST(ThreadSafeQueueTest, ConcurrentPushOnEmptyQueue)
{
    auto num_threads = std::thread::hardware_concurrency();
    auto f = [&](thread_safe_queue<int>& q){
        for(unsigned  i=0; i<num_threads; ++i){
            int val = -1;
            auto success = q.try_pop(val);
            assert(success);
            assert(val != -1);
        }};
    test_function(0, num_threads, 0, f);
}

TEST(ThreadSafeQueueTest, ConcurrentPushOnFullQueue)
{
    auto num_threads = std::thread::hardware_concurrency();
    auto queue_size = 100;
    auto f = [&](thread_safe_queue<int>& q){
        for(unsigned  i=0; i<num_threads+queue_size; ++i){
            int val = -1;
            auto success = q.try_pop(val);
            assert(success);
            assert(val != -1);
        }};
    test_function(0, num_threads, queue_size, f);
}

TEST(ThreadSafeQueueTest, ConcurrentPophOnEmptyQueue)
{
    test_function(std::thread::hardware_concurrency(), 0);
}

TEST(ThreadSafeQueueTest, ConcurrentPopOnFullQueue)
{
    auto num_threads = std::thread::hardware_concurrency();
    auto queue_size = 100;
    auto f = [&](thread_safe_queue<int>& q){
        for(unsigned  i=0; i<queue_size - num_threads; ++i){
            int val = -1;
            auto success = q.try_pop(val);
            assert(success);
            assert(val != -1);
        }};
    test_function(num_threads, 0, queue_size);
}


TEST(ThreadSafeQueueTest, ConcurrentPopOnPartiallyFullQueue)
{
    auto num_threads = std::thread::hardware_concurrency();
    auto queue_size = num_threads/2;
    auto f = [&](thread_safe_queue<int>& q){ assert(q.empty());};
    test_function(num_threads, 0, queue_size, f);
}

TEST(ThreadSafeQueueTest, ConcurrentPushAndSinglePopOnEmptyQueue)
{
    auto num_threads = std::thread::hardware_concurrency();
    test_function(1, num_threads-1, 0);
}

TEST(ThreadSafeQueueTest, ConcurrentPushAndSinglePopOnFullQueue)
{
    auto num_threads = std::thread::hardware_concurrency();
    test_function(1, num_threads-1,  1000);
}

TEST(ThreadSafeQueueTest, ConcurrentPushAndPophOnEmptyQueue)
{
    auto num_threads = std::thread::hardware_concurrency();
    test_function(num_threads/2, num_threads/2);
}

TEST(ThreadSafeQueueTest, ConcurrentPushAndPophOnFullQueue)
{
    auto num_threads = std::thread::hardware_concurrency();
    test_function(num_threads/2, num_threads/2, 1000);
}