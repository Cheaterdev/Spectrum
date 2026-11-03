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
		virtual ~Heap() = default;

		std::span<std::byte> get_data();
		size_t get_size();
		std::shared_ptr<Buffer> as_buffer();
	};
}

