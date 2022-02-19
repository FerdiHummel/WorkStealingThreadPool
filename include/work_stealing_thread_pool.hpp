#ifndef WORK_STEALING_THREAD_POOL_HPP
#define WORK_STEALING_THREAD_POOL_HPP

#include <atomic>
#include <future>
#include <functional>
#include <memory>
#include <random>
#include <vector>

#include "thread_safe_queue.hpp"

namespace WorkStealingThreadPool{

    class work_stealing_thread_pool{
        typedef std::function<void()> task_type;
        std::atomic_bool done;
        thread_safe_queue<task_type> pool_work_queue;
        std::vector<std::unique_ptr<thread_safe_queue<task_type>>> queues;
        std::vector<std::thread> threads;

        inline static thread_local thread_safe_queue<task_type>* local_work_queue = nullptr;
        inline static thread_local unsigned index = 0;

        void worker_thread(unsigned index_){
            index=index_;
            local_work_queue = queues[index].get();
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

        void join_threads(){
            for(auto& thread : threads){
                if(thread.joinable()){
                    thread.join();
                }
            }
        }

    public:
        explicit work_stealing_thread_pool(unsigned const thread_count = std::thread::hardware_concurrency()): done(false){
            try{
                for(unsigned i=0;i<thread_count;++i){
                    queues.push_back(std::make_unique<thread_safe_queue<task_type>>());
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
            join_threads();
        }

        template<typename F, typename... Args>
        std::future<typename std::result_of<F(Args...)>::type> submit(F f, Args... args){
            typedef typename std::result_of<F(Args...)>::type result_type;
            auto task = std::make_shared<std::packaged_task<result_type()>>(std::bind(std::forward<F>(f), std::forward<Args>(args) ... ));
            auto res(task->get_future());

            auto task_ =
                    [task]()
                    {
                        (*task)();
                    };

            if(local_work_queue){
                local_work_queue->push(std::move(task_));
            }
            else{
                pool_work_queue.push(std::move(task_));
            }
            return res;
        }
    };
}

#endif //WORK_STEALING_THREAD_POOL_HPP
