#ifndef WORK_STEALING_THREAD_POOL_HPP
#define WORK_STEALING_THREAD_POOL_HPP

#include <atomic>
#include <future>
#include <stdlib.h>
#include <functional>
#include <memory>
#include <vector>

#include "function_wrapper.hpp"
#include "threadsafe_queue.hpp"

namespace WorkStealingThreadPool{

    class work_stealing_thread_pool{
        typedef function_wrapper task_type;
        std::atomic_bool done;
        threadsafe_queue<task_type> pool_work_queue;
        std::vector<std::unique_ptr<threadsafe_queue<task_type>>> queues;
        std::vector<std::thread> threads;

        void worker_thread(unsigned index_){
            unsigned index=index_;
            auto local_work_queue= queues[index].get();
            while(!done){
                run_pending_task(index, local_work_queue);
            }
        }

        bool pop_task_from_local_queue(task_type& task, threadsafe_queue<task_type>* local_work_queue){
            return local_work_queue && local_work_queue->try_pop(task);
        }

        bool pop_task_from_pool_queue(task_type& task){
            return pool_work_queue.try_pop(task);
        }

        bool pop_task_from_other_thread(task_type& task, unsigned&  index){
            for(unsigned  i=0; i<queues.size(); ++i){
                unsigned const idx=(index+i+1)%queues.size();
                if(queues[idx]->try_pop(task)){
                    return true;
                }
            }
            return false;
        }

        void run_pending_task(unsigned& index, threadsafe_queue<task_type>* local_work_queue){
            task_type task;
            if(pop_task_from_local_queue(task, local_work_queue) ||
               pop_task_from_pool_queue(task) ||
               pop_task_from_other_thread(task, index)){
                task();
            }
            else{
                std::this_thread::yield();
            }
        }

    public:
        explicit work_stealing_thread_pool(unsigned const thread_count = std::thread::hardware_concurrency()): done(false){
            try{
                for(unsigned i=0;i<thread_count;++i){
                    queues.push_back(std::make_unique<threadsafe_queue<task_type>>());
                }
                for(unsigned i=0;i<thread_count;++i){
                    threads.push_back(std::thread(&work_stealing_thread_pool::worker_thread, this, i));
                }
            }
            catch (...){
                done=true;
                throw;
            }
        };

        ~work_stealing_thread_pool(){
            done=true;
            for(auto& thread : threads){
                auto future = std::async(std::launch::async, &std::thread::join, &thread);
                if (future.wait_for(std::chrono::seconds(5)) == std::future_status::timeout) {
                    // TODO terminate thread
                }
            }
        }

        template<typename FunctionType, typename... Args>
        std::future<typename std::result_of<FunctionType(Args...)>::type> submit(FunctionType f, Args... args){
            typedef typename std::result_of<FunctionType(Args...)>::type result_type;
            std::packaged_task<result_type()> task(std::bind(std::forward<FunctionType>(f), std::forward<Args>(args) ... ));
            std::future<result_type> res(task.get_future());
            unsigned queue_index = rand() % queues.size();
            auto local_queue = queues[queue_index].get();
            if(local_queue){
                local_queue->push(std::move(task));
            }
            else{
                pool_work_queue.push(std::move(task));
            }
            return res;
        }
    };
}

#endif //WORK_STEALING_THREAD_POOL_HPP
