#ifndef THREAD_SAFE_QUEUE_HPP
#define THREAD_SAFE_QUEUE_HPP

#include <memory>
#include <mutex>

namespace WorkStealingThreadPool{

    template <typename T>
    class thread_safe_queue{
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

        void move_queue(thread_safe_queue&& other){
            std::lock_guard<std::mutex> tail_lock(tail_mutex);
            std::lock_guard<std::mutex> other_tail_lock(other.tail_mutex);
            tail=std::move(other.tail);
            std::lock_guard<std::mutex> head_lock(head_mutex);
            std::lock_guard<std::mutex> other_head_lock(other.head_mutex);
            head=std::move(other.head);
            other.head = std::make_unique<node>();
            other.tail = other.head.get();
        }

    public:
        thread_safe_queue(): head(new node), tail(head.get()){};
        thread_safe_queue(const thread_safe_queue& other) = delete;
        thread_safe_queue(thread_safe_queue&& other) noexcept : head(new node), tail(head.get()){
            move_queue(std::forward<thread_safe_queue>(other));
        };
        thread_safe_queue& operator=(thread_safe_queue& other) = delete;
        thread_safe_queue& operator=(thread_safe_queue&& other){
            move_queue(std::forward<thread_safe_queue>(other));
            return *this;
        };

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

#endif //THREAD_SAFE_QUEUE_HPP


