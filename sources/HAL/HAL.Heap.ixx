export module HAL:Heap;
import Core;
import :Types;
import :Sampler;
import :Utils;
import :Device;
import :API.Heap;

using namespace HAL;


export namespace HAL
{
	struct HeapDesc
	{
		size_t Size;
		HeapType Type;
		MemoryType Memory;
		HeapFlags Flags;
	};

	inline std::atomic<uint64_t> temp_live_heap_bytes{ 0 }; // TEMP

	class Buffer;
	class Heap :public SharedObject<Heap>, public ObjectState<TrackedObjectState>, public TrackedObject, public API::Heap
	{
		friend class API::Heap;
		HeapDesc desc;
		std::shared_ptr<Buffer> buffer;
	public:
		using ptr = std::shared_ptr<Heap>;

		Heap(Device& device, const HeapDesc& desc);

		HeapType get_type();
		virtual ~Heap() // TEMP: heap free log (heaps.temp)
		{
			const uint64_t live = (temp_live_heap_bytes -= desc.Size);
			std::ofstream("heaps.temp", std::ios::app) << "FREE size=" << desc.Size / (1024.0 * 1024.0) << " MB live=" << live / (1024.0 * 1024.0) << " MB\n";
		}

		std::span<std::byte> get_data();
		size_t get_size();
		std::shared_ptr<Buffer> as_buffer();
	};
}

