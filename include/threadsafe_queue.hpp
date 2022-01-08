#ifndef THREADSAFE_QUEUE_HPP
#define THREADSAFE_QUEUE_HPP

#include <memory>
#include <mutex>

namespace WorkStealingThreadPool{

    template <typename T>
    class threadsafe_queue{
    private:
        struct node
        {
            std::shared_ptr<T> data;
            std::unique_ptr<node> next;
        };
        std::mutex head_mutex;
        std::unique_ptr<node> head;
        std::mutex tail_mutex;
        node* tail;

        node* get_tail(){
          std::lock_guard<std::mutex> tail_lock(tail_mutex);
          return tail;
        }

        std::unique_ptr<node> pop_head(){
            std::unique_ptr<node> old_head=std::move(head);
            head=std::move(old_head->next);
            return old_head;
        }

        std::unique_ptr<node> try_pop_head(T& value){
            std::lock_guard<std::mutex> head_lock(head_mutex);
            if(head.get()==get_tail()){
                return std::unique_ptr<node>();
            }
            value=std::move(*head->data);
            return pop_head();
        };

    public:
        threadsafe_queue(): head(new node), tail(head.get()){};
        threadsafe_queue(const threadsafe_queue& other) = delete;
        threadsafe_queue& operator=(const threadsafe_queue& other) = delete;

        bool try_pop(T& value){
            std::unique_ptr<node> const old_head=try_pop_head(value);
            return old_head ? true : false;
        }

        void push(T new_value){
            std::shared_ptr<T> new_data(std::make_shared<T>(std::move(new_value)));
            std::unique_ptr<node> p(new node);
            {
                std::lock_guard<std::mutex> tail_lock(tail_mutex);
                tail->data=new_data;
                node* const new_tail=p.get();
                tail->next=std::move(p);
                tail=new_tail;
            }
        }

        bool empty(){
            std::lock_guard<std::mutex> head_lock(head_mutex);
            return (head.get()==get_tail());
        }
    };
}

#endif //THREADSAFE_QUEUE_HPP


