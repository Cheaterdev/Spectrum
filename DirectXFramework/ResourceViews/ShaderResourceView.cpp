#include "pch.h"
namespace DX11{
	DX11_ShaderResourceView ShaderResourceView::operator*() const
	{
		return view;
	}

	ShaderResourceView::ShaderResourceView(Resource * resource, const D3D11_SHADER_RESOURCE_VIEW_DESC& desc)
	{
		Device::get().get_native_device()->CreateShaderResourceView(resource->get_native(), &desc, &view);
	}
}
