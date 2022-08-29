#pragma once

import Singleton;
import d3d12;

import Utils;
import serialization;

#include "Serialization/serialization_defines.h"

typedef ComPtr<IDXGISwapChain1>		DXGI_SwapChain;
typedef ComPtr<IDXGISurface2>			DXGI_Surface;
typedef ComPtr<IDXGIDevice2>			DXGI_Device;
typedef ComPtr<IDXGIAdapter2>			DXGI_Adapter;
typedef ComPtr<IDXGIFactory2>			DXGI_Factory;
typedef ComPtr<IDXGIOutput1>			DXGI_Output;


// serialization

class string_heap : public Singleton<string_heap>
{
	friend class Singleton<string_heap>;

	std::set<std::string> strings;

public:
	const char* get_string(std::string str)
	{
		auto data = strings.find(str);

		if (data == strings.end())
			data = strings.insert(strings.end(), str);

		return data->c_str();
	}

};


UINT BitsPerPixel(DXGI_FORMAT fmt);
DXGI_FORMAT to_srv(DXGI_FORMAT);
DXGI_FORMAT to_dsv(DXGI_FORMAT);
UINT get_default_mapping(DXGI_FORMAT);
DXGI_FORMAT to_linear(DXGI_FORMAT);
DXGI_FORMAT to_typeless(DXGI_FORMAT);

bool is_shader_visible(DXGI_FORMAT);

namespace cereal
{
	template<class Archive>
	void serialize(Archive& ar, D3D12_DEPTH_STENCILOP_DESC& g, const unsigned int)
	{
		//	ar & g.DefaultValue;
		ar& NVPG(StencilDepthFailOp);
		ar& NVPG(StencilFailOp);
		ar& NVPG(StencilFunc);
		ar& NVPG(StencilPassOp);
	}

	template<class Archive>
	void serialize(Archive& ar, DXGI_SAMPLE_DESC& g, const unsigned int)
	{
		//	ar & g.DefaultValue;
		ar& NVPG(Count);
		ar& NVPG(Quality);
	}


	template<class Archive>
	void serialize(Archive& ar, D3D12_RESOURCE_DESC& g, const unsigned int)
	{
		ar& NVPG(Dimension);
		ar& NVPG(Alignment);
		ar& NVPG(Width);
		ar& NVPG(Height);
		ar& NVPG(DepthOrArraySize);
		ar& NVPG(MipLevels);
		ar& NVPG(Format);
		ar& NVPG(SampleDesc);
		ar& NVPG(Layout);
		ar& NVPG(Flags);
	}


	template<class Archive>
	void serialize(Archive& ar, D3D12_AUTO_BREADCRUMB_NODE& g, const unsigned int)
	{
		std::wstring CommandListDebugName = g.pCommandListDebugNameW ? g.pCommandListDebugNameW : L"";
		std::wstring CommandQueueDebugName = g.pCommandQueueDebugNameW ? g.pCommandQueueDebugNameW : L"";
		ar& NVP(CommandListDebugName);
		ar& NVP(CommandQueueDebugName);
		if (g.pLastBreadcrumbValue && *g.pLastBreadcrumbValue < g.BreadcrumbCount - 1)
		{
			ar& NP("FAILED", true);
		}


		std::vector<D3D12_AUTO_BREADCRUMB_OP> ops;
		ops.assign(g.pCommandHistory, g.pCommandHistory + g.BreadcrumbCount);

		UINT offset = g.pLastBreadcrumbValue ? (*g.pLastBreadcrumbValue) : g.BreadcrumbCount;

		for (UINT i = 0; i < ops.size(); i++)
		{
			auto op = ops[i];
			if (i <= offset) ar& NP("DONE", op);
			if (i == offset + 1) ar& NP("FAILED", op);
			if (i > offset + 1) ar& NP("WAITING", op);

		}
	}
}

