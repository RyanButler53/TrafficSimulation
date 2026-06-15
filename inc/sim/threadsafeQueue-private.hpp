
#include "threadsafeQueue.hpp"
#include <iostream>

template <typename T>
ThreadsafeQueue<T>::ThreadsafeQueue(size_t n):maxSize{n}{}

template <typename T>
void ThreadsafeQueue<T>::wait_and_pop(T &value)
{
    std::unique_lock<std::mutex> lk(mut);
    empty_cond.wait(lk,[this]{return !data_queue.empty();});
    value = std::move(*data_queue.front());
    data_queue.pop();
}

template <typename T>
bool ThreadsafeQueue<T>::try_pop(T &value){
    std::lock_guard<std::mutex> lk(mut);    
    if (data_queue.empty())
        return false;
    value = std::move(*data_queue.front());
    data_queue.pop();
    return true;    
}
template <typename T>
std::shared_ptr<T> ThreadsafeQueue<T>::wait_and_pop(){
    std::unique_lock<std::mutex> lk(mut);
    empty_cond.wait(lk,[this]{return !data_queue.empty();});
    std::shared_ptr<T> res = data_queue.front();
    data_queue.pop();
    return res;
}

template <typename T>
std::shared_ptr<T> ThreadsafeQueue<T>::try_pop(){
    std::lock_guard<std::mutex> lk(mut);
    if(data_queue.empty())
        return std::shared_ptr<T>();
    std::shared_ptr<T> res = data_queue.front();
    data_queue.pop();
    return res;
}   

template <typename T>
bool ThreadsafeQueue<T>::empty() const {
        std::lock_guard<std::mutex> lk(mut);
        return data_queue.empty();
}

template <typename T>
bool ThreadsafeQueue<T>::try_push(T new_value){
    std::lock_guard<std::mutex> lk(mut);
    // Can't push to a full queue
    if (data_queue.size() == maxSize){
        return false;
    }
    std::shared_ptr<T> data(std::make_shared<T>(std::move(new_value)));
    data_queue.push(data);
    // Notify thread waiting on empty queue that data is ready
    empty_cond.notify_one();
    return true;
}

template <typename T>
void ThreadsafeQueue<T>::wait_and_push(T new_value){

    std::shared_ptr<T> data(std::make_shared<T>(std::move(new_value)));
    std::unique_lock<std::mutex> lk(mut);
    full_cond.wait(lk,[this]{return data_queue.size() < maxSize;});

    data_queue.push(data);
    empty_cond.notify_one();
}
