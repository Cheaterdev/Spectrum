export module HAL:API.Fence;
import :Utils;

export namespace HAL
{
    namespace API
    {
        class Fence
        {
        public:
            D3D::Fence    m_fence;
            using CounterType = UINT64;
        };

        class Event
        {
            friend class Fence;
        public:
            HANDLE m_fenceEvent;
        };

    }
}