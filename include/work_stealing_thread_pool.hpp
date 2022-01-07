#ifndef WORKSTEALINGTHREADPOOL_WORK_STEALING_THREAD_POOL_HPP
#define WORKSTEALINGTHREADPOOL_WORK_STEALING_THREAD_POOL_HPP

#include <atomic>
#include <future>
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
        static thread_local threadsafe_queue<task_type>* local_work_queue;
        static thread_local unsigned index;

        void worker_thread(unsigned index_){
            index=index_;
            local_work_queue=queues[index].get();
            while(!done){
                run_pending_task();
            }
        }

        bool pop_task_from_local_queue(task_type& task){
            return local_work_queue && local_work_queue->try_pop(task);
        }

        bool pop_task_from_pool_queue(task_type& task){
            return pool_work_queue.try_pop(task);
        }

        bool pop_task_from_other_thread(task_type& task){
            for(unsigned  i=0; i<queues.size(); ++i){
                unsigned const idx=(index+i+1)%queues.size();
                if(queues[idx]->try_pop(task)){
                    return true;
                }
            }
            return false;
        }

        void run_pending_task(){
            task_type task;
            if(pop_task_from_local_queue(task) ||
               pop_task_from_pool_queue(task) ||
               pop_task_from_other_thread(task)){
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
            for(auto& thread : threads){
                thread.join();
            }
            done=true;
        }

        template<typename FunctionType, typename... Args>
        std::future<typename std::result_of<FunctionType(Args...)>::type> submit(FunctionType f, Args... args){
            typedef typename std::result_of<FunctionType(Args...)>::type result_type;
            std::packaged_task<result_type()> task(std::bind(std::forward<FunctionType>(f), std::forward<Args>(args) ... ));
            std::future<result_type> res(task.get_future());
            if(local_work_queue){
                local_work_queue->push(std::move(task));
            }
            else{
                pool_work_queue.push(std::move(task));
            }
            return res;
        }
    };
}

#endif //WORKSTEALINGTHREADPOOL_WORK_STEALING_THREAD_POOL_HPP
