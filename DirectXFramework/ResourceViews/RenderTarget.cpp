#include "pch.h"
namespace DX11{
	DX11_RenderTargetView RenderTargetView::operator*() const
	{
		return view;
	}

	RenderTargetView::RenderTargetView(Resource * resource, const D3D11_RENDER_TARGET_VIEW_DESC& desc)
	{
		Device::get().get_native_device()->CreateRenderTargetView(resource->get_native(), &desc, &view);
	}


	DX11_DepthStencilView DepthStencilView::operator*() const
	{
		return view;
	}

	DepthStencilView::DepthStencilView(Resource * resource, const D3D11_DEPTH_STENCIL_VIEW_DESC& desc)
	{
		Device::get().get_native_device()->CreateDepthStencilView(resource->get_native(), &desc, &view);
	}

}
