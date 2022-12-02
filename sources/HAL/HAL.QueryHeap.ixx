export module HAL:QueryHeap;
import Core;
import :Types;
import :Sampler;
import :Utils;
import :Device;
import :API.QueryHeap;

using namespace HAL;


export namespace HAL
{
	struct QueryHeapDesc
	{
		uint Count;
		QueryType type;
	};

	class QueryHeap:public SharedObject<QueryHeap>, public ObjectState<TrackedObjectState>, public TrackedObject, public API::QueryHeap
	{

		friend class GPUTimer;
		QueryHeapDesc desc;

	
	public:
		using ptr = std::shared_ptr<QueryHeap>;
		QueryHeap(Device& device, const QueryHeapDesc & desc);
	std::vector<UINT64> read_back_data;


		std::shared_ptr<QueryHeap> get_tracked()
		{
			return get_ptr<QueryHeap>();
		}
		size_t get_size()
		{
			return desc.Count;
		}
	};

	struct QueryContext
	{
		using HeapPageType = HAL::QueryHeap;
		using HeapMemoryOptions = QueryType;

		static const size_t PageAlignment = 64;
	};



	using QueryHandle = Allocators::HeapHandle<HAL::QueryHeap>;

	class QueryHeapFactory :public Allocators::HeapFactory<QueryContext, GlobalAllocationPolicy>
	{
		Device& device;
		virtual ptr_type make_heap(QueryType type, size_t size) override
		{
			QueryHeapDesc desc = { size , type };
			return std::make_shared<HAL::QueryHeap>(device, desc);
		}

	public:
		QueryHeapFactory(Device& device) :device(device)
		{

		}

	};
	template<class AllocationPolicy>
	class QueryHeapPageManager :public Allocators::HeapPageManager<QueryContext, AllocationPolicy>
	{
		Device& device;
	public:
		QueryHeapPageManager(Device& _device) :Allocators::HeapPageManager<QueryContext, AllocationPolicy>(_device.get_query_heap_factory()), device(_device) {}


	};

}

