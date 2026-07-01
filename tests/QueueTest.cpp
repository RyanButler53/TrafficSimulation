#include "dataStructures/threadsafeQueue.hpp"
#include <gtest/gtest.h>
#include <chrono>
#include <thread>
#include <print>
#include <future>

std::vector<int> pop(ThreadsafeQueue<int>& q){
    std::vector<int> values;
    while (!q.empty() || values.size() < 10){
        auto integer = q.try_pop();
        if (integer){
            values.push_back(*integer);
        }
    }
    return values;
}

TEST(QueueTest, FullQueue){
    ThreadsafeQueue<int> q(5);
    for (int i = 0; i < 5; ++i){
        q.wait_and_push(i);
    }

    std::future<std::vector<int>> f = std::async(pop, std::ref(q));
    for (int i = 5; i < 10; ++i){
        q.wait_and_push(i);
    }

    std::vector<int> values = f.get();
    ASSERT_EQ(values.size(), 10);
    for (int i = 0; i < 10; ++i){
        EXPECT_EQ(i, values[i]);
    }
}
