#include <chrono>
#include <future>
#include <memory>
#include <thread>

#include "gtest/gtest.h"

#include "work_stealing_thread_pool.hpp"

using namespace WorkStealingThreadPool;


int heavy_work_load(int a, int b, int seconds){
    std::this_thread::sleep_for(std::chrono::seconds(seconds));
    return a+b;
}

int light_work_load(int a, int b, int milliseconds){
    std::this_thread::sleep_for(std::chrono::milliseconds (milliseconds));
    return a+b;
}

void test_function(unsigned num_threads = 1,
                   unsigned num_jobs = 1,
                   int duration = 1,
                   bool heavy_workload = false){
    work_stealing_thread_pool wstp(num_threads);
    std::vector<std::future<int>> results;
    for(unsigned i=0; i<num_jobs; ++i){
        if(heavy_workload){
            results.push_back(wstp.submit(heavy_work_load, 1, 2, duration));
        }
        else{
            results.push_back(wstp.submit(light_work_load, 1, 2, duration));
        }
    }
    for(auto& future : results){
        assert(future.valid());
        future.wait();
    }

    for(auto& future : results){
        auto f = future.get();
        assert(f == 3);
    }
}


TEST(WorkStealingThreadPoolTest, SingleThreadHeavyWorkload)
{
    test_function(1, 10, 1, true);
}

TEST(WorkStealingThreadPoolTest, MultipleThreadsHeavyWorkloadPartialJobs)
{
    auto num_threads = std::thread::hardware_concurrency();
    auto jobs = num_threads / 2;
    test_function(num_threads, jobs, 1, true);
}

TEST(WorkStealingThreadPoolTest, MultipleThreadsHeavyWorkloadEnoughJobs)
{
    auto num_threads = std::thread::hardware_concurrency();
    auto jobs = num_threads * 2;
    test_function(num_threads, jobs, 1, true);
}

TEST(WorkStealingThreadPoolTest, SingleThreadLightWorkload)
{
    test_function(1, 10, 1, false);
}

TEST(WorkStealingThreadPoolTest, MultipleThreadsLightWorkloadPartialJobs)
{
    auto num_threads = std::thread::hardware_concurrency();
    auto jobs = num_threads / 2;
    test_function(num_threads, jobs, 1, false);
}

TEST(WorkStealingThreadPoolTest, MultipleThreadsLightWorkloadEnoughJobs)
{
    auto num_threads = std::thread::hardware_concurrency();
    auto jobs = num_threads * 2;
    test_function(num_threads, jobs, 1, false);
}