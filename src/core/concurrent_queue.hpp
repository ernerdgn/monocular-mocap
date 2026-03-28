#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>
#include <optional>

namespace mocap {

    template <typename T>
    class ConcurrentQueue
    {
        public:
            // adds a frame to queue
            void push(T value)
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_queue.push(std::move(value));
                m_cv.notify_one();
            }

            // gets frame, return null if empty to prevent block
            std::optional<T> try_pop()
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                if (m_queue.empty()) return std::nullopt;
                
                T value = std::move(m_queue.front());
                m_queue.pop();
                return value;
            }

            // wait for item to be available
            T wait_and_pop()
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                m_cv.wait(lock, [this] { return !m_queue.empty(); });
                
                T value = std::move(m_queue.front());
                m_queue.pop();
                return value;
            }

            size_t size() const
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                return m_queue.size();
            }

        private:
            std::queue<T> m_queue;
            mutable std::mutex m_mutex;
            std::condition_variable m_cv;
    };

}