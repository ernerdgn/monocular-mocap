#include <catch2/catch_test_macros.hpp>
#include "core/concurrent_queue.hpp"
#include <atomic>
#include <chrono>
#include <thread>
#include <vector>

using namespace mocap;
using namespace std::chrono_literals;

TEST_CASE("ConcurrentQueue: push and pop single item", "[concurrent_queue]")
{
    ConcurrentQueue<int> q(4);
    REQUIRE(q.empty());
    REQUIRE(q.size() == 0);

    bool pushed = q.push(42);
    REQUIRE(pushed);
    REQUIRE(q.size() == 1);
    REQUIRE_FALSE(q.empty());

    auto item = q.pop();
    REQUIRE(item.has_value());
    REQUIRE(item.value() == 42);
    REQUIRE(q.empty());
}

TEST_CASE("ConcurrentQueue: FIFO ordering is preserved", "[concurrent_queue]")
{
    ConcurrentQueue<int> q(8);
    for (int i = 0; i < 5; ++i) q.push(i);
    for (int i = 0; i < 5; ++i) {
        auto item = q.pop();
        REQUIRE(item.has_value());
        REQUIRE(item.value() == i);
    }
}

TEST_CASE("ConcurrentQueue: try_push returns false when full", "[concurrent_queue]")
{
    ConcurrentQueue<int> q(2);
    REQUIRE(q.try_push(1));
    REQUIRE(q.try_push(2));
    REQUIRE_FALSE(q.try_push(3)); 
    REQUIRE(q.size() == 2);
}

TEST_CASE("ConcurrentQueue: try_pop returns nullopt when empty", "[concurrent_queue]")
{
    ConcurrentQueue<int> q(4);
    auto item = q.try_pop();
    REQUIRE_FALSE(item.has_value());
}

TEST_CASE("ConcurrentQueue: close causes pop to return nullopt on empty queue", "[concurrent_queue]")
{
    ConcurrentQueue<int> q(4);
    q.close();
    REQUIRE(q.is_closed());
    auto item = q.pop();
    REQUIRE_FALSE(item.has_value()); 
}

TEST_CASE("ConcurrentQueue: close after push allows drain before returning nullopt", "[concurrent_queue]")
{
    ConcurrentQueue<int> q(4);
    q.push(10);
    q.push(20);
    q.close();

    auto a = q.pop();
    REQUIRE(a.has_value());
    REQUIRE(a.value() == 10);

    auto b = q.pop();
    REQUIRE(b.has_value());
    REQUIRE(b.value() == 20);

    auto c = q.pop();
    REQUIRE_FALSE(c.has_value());
}

TEST_CASE("ConcurrentQueue: push returns false after close", "[concurrent_queue]")
{
    ConcurrentQueue<int> q(4);
    q.close();
    bool result = q.push(99);
    REQUIRE_FALSE(result);
}

TEST_CASE("ConcurrentQueue: concurrent producer and consumer, no data loss", "[concurrent_queue]")
{
    constexpr int k_itemCount = 1000;
    ConcurrentQueue<int> q(16); 
    std::atomic<int> consumedSum{0};

    std::jthread producer([&]()
    {
        for (int i = 0; i < k_itemCount; ++i) q.push(i);
        q.close();
    });

    std::jthread consumer([&]()
    {
        while (true)
        {
            auto item = q.pop();
            if (!item.has_value()) break;
            consumedSum.fetch_add(item.value(), std::memory_order_relaxed);
        }
    });

    producer.join();
    consumer.join();

    constexpr int k_expectedSum = k_itemCount * (k_itemCount - 1) / 2;
    REQUIRE(consumedSum.load() == k_expectedSum);
}

TEST_CASE("ConcurrentQueue: concurrent producer blocks when full, consumer unblocks it", "[concurrent_queue]")
{
    ConcurrentQueue<int> q(4);
    std::atomic<bool> producerFinished{false};

    std::jthread producer([&]()
    {
        for (int i = 0; i < 4; ++i) q.push(i);
        q.push(99);
        producerFinished.store(true);
    });

    std::this_thread::sleep_for(50ms);
    REQUIRE_FALSE(producerFinished.load()); 

    auto item = q.pop();
    REQUIRE(item.has_value());

    std::this_thread::sleep_for(50ms);
    REQUIRE(producerFinished.load());

    q.close();
    producer.join();
}