export module HAL:API.Fence;
import :Utils;


export namespace HAL
{
	namespace API
	{
		class Fence
		{
		public: /////////////
			D3D::Fence m_fence;
		public:
			using CounterType = UINT64;
		};

		class Event
		{
		public: //////////////
			friend class Fence;
			HANDLE m_fenceEvent;
		};

	}
}


