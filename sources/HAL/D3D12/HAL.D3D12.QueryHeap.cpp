module HAL:QueryHeap;
import <HAL.h>;
import <d3d12/d3d12_includes.h>;
import Core;

import :API.QueryHeap;
#undef THIS
namespace HAL
{
	QueryHeap::QueryHeap(Device& device, const QueryHeapDesc& desc):desc(desc)
	{
		D3D12_QUERY_HEAP_DESC QueryHeapDesc;
		QueryHeapDesc.Count = desc.Count;
		QueryHeapDesc.NodeMask = 1;
		QueryHeapDesc.Type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
		TEST(device, device.get_native_device()->CreateQueryHeap(&QueryHeapDesc, IID_PPV_ARGS(&heap)));
		heap->SetName(L"QueryHeap");

		read_back_data.resize(desc.Count);
	}

	namespace API
	{
		ComPtr<ID3D12QueryHeap> QueryHeap::get_native() const
		{
			return heap;
		}

		

	}
}
