module HAL:API.Queue;

import vulkan;
import Core;
import HAL;
#undef THIS

using namespace HAL;

// Vulkan native implementation for the symbols the D3D12 build defines in
// D3D12/HAL.D3D12.Queue.cpp (also `module HAL:API.Queue`):
//   * HAL::API::Queue            — native queue submit/sync
//   * HAL::API::DirectStorageQueue (none needed; base is empty)
//   * HAL::Queue::update_tile_mappings / get_clock_time   (native-split members
//     of the common HAL::Queue class)
//   * HAL::DirectStorageQueue::*  (the whole streaming queue)
// The rest of HAL::Queue lives in the common HAL.Queue.cpp and is shared.
//
// Phase 0: all native operations are stubs.  Phase 1 wires vkQueueSubmit2 +
// timeline semaphores; DirectStorage streaming is replaced by a Vulkan
// staging-buffer uploader in a later phase.

namespace HAL
{
    // ---- native-split members of common HAL::Queue -------------------------

    void Queue::update_tile_mappings(const update_tiling_info& /*infos*/)
    {
        // Phase 4+: vkQueueBindSparse for sparse/tiled resources.
    }

    ClockCalibrationInfo Queue::get_clock_time() const
    {
        // Phase 1: vkGetCalibratedTimestampsEXT.
        return { 0, 0, frequency };
    }

    // ---- HAL::DirectStorageQueue (streaming) -------------------------------

    DirectStorageQueue::DirectStorageQueue(Device& device) : device(device), requestCounter(device) {}
    DirectStorageQueue::~DirectStorageQueue() { executor.stop_and_wait(); }

    void DirectStorageQueue::flush() {}
    void DirectStorageQueue::stop_all() {}

    HAL::FenceWaiter DirectStorageQueue::signal()
    {
        auto value = ++m_fenceValue;
        requestCounter.signal(value);
        return FenceWaiter{ &requestCounter, value };
    }

    void DirectStorageQueue::signal_and_wait()
    {
        auto s = signal();
        flush();
        s.wait();
    }

    bool DirectStorageQueue::is_complete(UINT64 fence)
    {
        return requestCounter.get_completed_value() >= fence;
    }

    FenceWaiter DirectStorageQueue::get_waiter()
    {
        return FenceWaiter{ &requestCounter, 0 };
    }

    HAL::FenceWaiter DirectStorageQueue::execute(StorageRequest /*request*/)
    {
        // Phase: replace with Vulkan staging-buffer upload + (optional) GDeflate.
        return signal();
    }

    // ---- HAL::API::Queue ---------------------------------------------------

    namespace API
    {
        void Queue::construct(HAL::CommandListType type, Device* device)
        {
            // Phase 1: vkGetDeviceQueue for the family matching `type`, create
            // the per-frame VkCommandPool, query timestamp period for frequency.
            family_idx = static_cast<uint32_t>(-1);
        }

        void Queue::execute(const API::CommandList* /*list*/) {}
        void Queue::flush() {}
        void Queue::signal(Fence& /*fence*/, Fence::CounterType /*value*/) {}
        void Queue::gpu_wait(HAL::FenceWaiter /*waiter*/) {}

        VkQueue Queue::get_native() const { return vk_queue; }
    }
}
