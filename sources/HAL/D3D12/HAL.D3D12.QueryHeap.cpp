module HAL:QueryHeap;

import d3d12;
import Math;
import Utils;

import :API.QueryHeap;
#undef THIS
namespace HAL
{
	QueryHeap::QueryHeap(Device& device, const QueryHeapDesc& desc)
	{
		D3D12_QUERY_HEAP_DESC QueryHeapDesc;
		QueryHeapDesc.Count = desc.Count;
		QueryHeapDesc.NodeMask = 1;
		QueryHeapDesc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
		HAL::Device::get().get_native_device()->CreateQueryHeap(&QueryHeapDesc, IID_PPV_ARGS(&heap));
		heap->SetName(L"QueryHeap");
	}

	namespace API
	{
		ComPtr<ID3D12QueryHeap> QueryHeap::get_native()
		{
			return heap;
		}

		

	}
}