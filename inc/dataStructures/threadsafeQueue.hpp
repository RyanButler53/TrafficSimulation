#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <numeric>
#include <optional>

template<typename T>
class ThreadsafeQueue
{
private:
    mutable std::mutex mut;
    std::queue<std::shared_ptr<T> > data_queue;
    std::condition_variable empty_cond;
    std::condition_variable full_cond;
    size_t maxSize{std::numeric_limits<size_t>::max()};
public:
    ThreadsafeQueue() = default;

    ThreadsafeQueue(size_t maxSize);

    void wait_and_pop(T &value); // never use

    bool try_pop(T &value); // never use

    std::shared_ptr<T> wait_and_pop(); // always use

    std::shared_ptr<T> try_pop(); // never use

    bool empty() const;

    bool try_push(T new_value); // use between 80% and 90% capacity

    void wait_and_push(T new_value); // Wait at 90% memory and always push it through

    std::shared_ptr<T> wait_and_pop(size_t ms);
    
    size_t size() const;
};

#include "threadsafeQueue-private.hpp"
