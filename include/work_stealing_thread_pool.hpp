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

        void worker_thread(unsigned index_){
            unsigned index=index_;
            auto local_work_queue= queues[index].get();
            while(!done){
                run_pending_task(index, local_work_queue);
            }
        }

        bool pop_task_from_local_queue(task_type& task, thread_safe_queue<task_type>* local_work_queue){
            return local_work_queue->try_pop(task);
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

        void run_pending_task(unsigned& index, thread_safe_queue<task_type>* local_work_queue){
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

        void join_threads(){
            for(auto& thread : threads){
                if(thread.joinable()){
                    thread.join();
                }
            }
        }

        template<typename NumericType, typename Generator = std::mt19937>
        NumericType random_number(NumericType lower, NumericType upper)
        {
            thread_local static Generator gen(std::random_device{}());
            using distType = typename std::conditional<std::is_integral<NumericType>::value, std::uniform_int_distribution<NumericType>, std::uniform_real_distribution<NumericType>>::type;
            thread_local static distType dist;
            return dist(gen, typename distType::param_type{lower, upper});
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

            auto queue_index = random_number<unsigned>(0, queues.size());
            auto local_queue = queues[queue_index].get();
            if(local_queue){
                local_queue->push(std::move(task_));
            }
            else{
                pool_work_queue.push(std::move(task_));
            }
            return res;
        }
    };
}

#endif //WORK_STEALING_THREAD_POOL_HPP
