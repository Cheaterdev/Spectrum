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
	};

	class QueryHeap:public API::QueryHeap
	{

	public:
		using ptr = std::shared_ptr<QueryHeap>;
		QueryHeap(Device& device, const QueryHeapDesc & desc);
	};
}

