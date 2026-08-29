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
    /**
     * @brief Constructs an unbounded threadsafe queue.
     */
    ThreadsafeQueue() = default;

    /**
     * @brief Constructs a threadsafe queue with a bounded capacity.
     * @param maxSize The maximum number of elements the queue may hold.
     */
    ThreadsafeQueue(size_t maxSize);

    /**
     * @brief Checks whether the queue is currently empty.
     * @return true if the queue contains no elements, false otherwise.
     * @note Result may be stale immediately after return in a concurrent context;
     *       do not use as a precondition for pop without proper synchronization.
     */
    bool empty() const;

    /**
     * @brief Returns the current number of elements in the queue.
     * @return The number of elements currently held by the queue.
     * @note Result may be stale immediately after return in a concurrent context.
     */
    size_t size() const;

    /**
     * @brief Attempts to push a value without blocking.
     * @param new_value The value to push into the queue.
     * @return true if the value was successfully pushed, false if the queue was full.
     */
    bool try_push(T new_value);

    /**
     * @brief Pushes a value, blocking if necessary until space is available.
     * @param new_value The value to push into the queue.
     */
    void wait_and_push(T new_value);

    /**
     * @brief Attempts to pop a value without blocking, via output parameter.
     * @param value Reference to be populated with the popped value on success.
     * @return true if a value was popped, false if the queue was empty.
     */
    bool try_pop(T &value);

    /**
     * @brief Blocks until a value is available, then pops it via output parameter.
     * @param value Reference to be populated with the popped value.
     */
    void wait_and_pop(T &value);

    /**
     * @brief Attempts to pop a value without blocking.
     * @return A shared_ptr to the popped value, or nullptr if the queue was empty.
     */
    std::shared_ptr<T> try_pop();

    /**
     * @brief Blocks indefinitely until a value is available, then pops it.
     * @return A shared_ptr to the popped value.
     */
    std::shared_ptr<T> wait_and_pop();

    /**
     * @brief Blocks until a value is available or a timeout elapses, then pops if possible.
     * @param ms Maximum time to wait, in milliseconds.
     * @return A shared_ptr to the popped value, or nullptr if the timeout elapsed
     *         before a value became available.
     */
    std::shared_ptr<T> wait_and_pop(size_t ms);
};

#include "threadsafeQueue-private.hpp"
