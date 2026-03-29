#pragma once
#include "core/concurrent_queue.hpp"
#include "core/types.hpp"
#include <memory>

namespace mocap {

class FramePool : public std::enable_shared_from_this<FramePool>
{
public:
    static std::shared_ptr<FramePool> create(size_t size = 3)
    {
        auto pool = std::shared_ptr<FramePool>(new FramePool(size));
        for (size_t i = 0; i < size; ++i)
        {
            pool->m_freeQueue.try_push(new CaptureFrame());
        }
        return pool;
    }

    ~FramePool()
    {
        //CaptureFrame* frame = nullptr;
        while (auto f = m_freeQueue.try_pop())
        {
            delete f.value();
        }
    }

    std::shared_ptr<CaptureFrame> acquire()
    {
        auto optFrame = m_freeQueue.try_pop();
        // alloc new pool if empty
        CaptureFrame* rawFrame = optFrame.has_value() ? optFrame.value() : new CaptureFrame();

        // weak_ptr prevents the frame from keeping pool alive forever
        std::weak_ptr<FramePool> weakPool = weak_from_this();
        
        // custom deleter (MAGIC!)
        return std::shared_ptr<CaptureFrame>(rawFrame, [weakPool](CaptureFrame* ptr)
        {
            if (auto pool = weakPool.lock())
            {
                pool->m_freeQueue.try_push(ptr); // return pool
            }
            else
            {
                delete ptr; // pool destroyed, safely delete memory
            }
        });
    }

private:
    FramePool(size_t size) : m_freeQueue(size + 5) {}
    ConcurrentQueue<CaptureFrame*> m_freeQueue;
};

}