#include "pch.h"

namespace DX11
{

    template<>
    sampler_state::state(D3D11_SAMPLER_DESC desc)
    {
        this->desc = desc;
        DX11::Device::get().get_native_device()->CreateSamplerState(&desc, &native_state);
    }

    template<>
	depth_stencil_state::state(D3D11_DEPTH_STENCIL_DESC desc)
    {
        this->desc = desc;
        DX11::Device::get().get_native_device()->CreateDepthStencilState(&desc, &native_state);
    }

	template<>
	rasterizer_state::state(D3D11_RASTERIZER_DESC desc)
	{
		this->desc = desc;
		DX11::Device::get().get_native_device()->CreateRasterizerState(&desc, &native_state);
	}

	template<>
	blend_state::state(D3D11_BLEND_DESC desc)
	{
		this->desc = desc;
		DX11::Device::get().get_native_device()->CreateBlendState(&desc, &native_state);
	}

    template sampler_state;
    template depth_stencil_state;
	template rasterizer_state;
	template blend_state;


}

