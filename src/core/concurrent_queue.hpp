#pragma once
#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>

namespace mocap
{

template <typename T>
class ConcurrentQueue
{
  public:
    explicit ConcurrentQueue(size_t capacity) : m_capacity(capacity), m_closed(false)
    {
    }

    bool push(T item)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_cv_push.wait(lock, [this]() { return m_queue.size() < m_capacity || m_closed; });
        if (m_closed)
            return false;
        m_queue.push(std::move(item));
        m_cv_pop.notify_one();
        return true;
    }

    bool try_push(T item)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_closed || m_queue.size() >= m_capacity)
            return false;
        m_queue.push(std::move(item));
        m_cv_pop.notify_one();
        return true;
    }

    std::optional<T> pop()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_cv_pop.wait(lock, [this]() { return !m_queue.empty() || m_closed; });
        if (m_queue.empty() && m_closed)
            return std::nullopt;

        T item = std::move(m_queue.front());
        m_queue.pop();
        m_cv_push.notify_one();
        return item;
    }

    std::optional<T> try_pop()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_queue.empty())
            return std::nullopt;

        T item = std::move(m_queue.front());
        m_queue.pop();
        m_cv_push.notify_one();
        return item;
    }

    void close()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_closed = true;
        m_cv_push.notify_all();
        m_cv_pop.notify_all();
    }

    bool is_closed() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_closed;
    }

    bool empty() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.empty();
    }

    size_t size() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.size();
    }

  private:
    size_t m_capacity;
    bool m_closed;
    std::queue<T> m_queue;
    mutable std::mutex m_mutex;
    std::condition_variable m_cv_push;
    std::condition_variable m_cv_pop;
};

} // namespace mocap