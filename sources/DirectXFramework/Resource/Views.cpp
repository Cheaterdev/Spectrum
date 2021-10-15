#include "pch_dx.h"
#include "DX12/CommandList.h"
import ResourceViews;


 void DX12::BufferView::place_cb(Handle h) {
	if (!resource) return;

	D3D12_CONSTANT_BUFFER_VIEW_DESC  desc = {};
	desc.BufferLocation = resource->get_gpu_address();
	desc.SizeInBytes = (UINT)view_desc.Buffer.Size;
	assert(desc.SizeInBytes < 65536);

	Device::get().create_cbv(h, resource.get(), desc);
}


