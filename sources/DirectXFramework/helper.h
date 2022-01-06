#pragma once

import Singleton;
import d3d12;

import Utils;
import serialization;
#include <d3d12.h>

#include "Serialization/serialization_defines.h"

typedef ComPtr<IDXGISwapChain1>		DXGI_SwapChain;
typedef ComPtr<IDXGISurface2>			DXGI_Surface;
typedef ComPtr<IDXGIDevice2>			DXGI_Device;
typedef ComPtr<IDXGIAdapter2>			DXGI_Adapter;
typedef ComPtr<IDXGIFactory2>			DXGI_Factory;
typedef ComPtr<IDXGIOutput1>			DXGI_Output;



#define D3DCOMPILE_DEBUG                                (1 << 0)
#define D3DCOMPILE_SKIP_VALIDATION                      (1 << 1)
#define D3DCOMPILE_SKIP_OPTIMIZATION                    (1 << 2)
#define D3DCOMPILE_PACK_MATRIX_ROW_MAJOR                (1 << 3)
#define D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR             (1 << 4)
#define D3DCOMPILE_PARTIAL_PRECISION                    (1 << 5)
#define D3DCOMPILE_FORCE_VS_SOFTWARE_NO_OPT             (1 << 6)
#define D3DCOMPILE_FORCE_PS_SOFTWARE_NO_OPT             (1 << 7)
#define D3DCOMPILE_NO_PRESHADER                         (1 << 8)
#define D3DCOMPILE_AVOID_FLOW_CONTROL                   (1 << 9)
#define D3DCOMPILE_PREFER_FLOW_CONTROL                  (1 << 10)
#define D3DCOMPILE_ENABLE_STRICTNESS                    (1 << 11)
#define D3DCOMPILE_ENABLE_BACKWARDS_COMPATIBILITY       (1 << 12)
#define D3DCOMPILE_IEEE_STRICTNESS                      (1 << 13)
#define D3DCOMPILE_OPTIMIZATION_LEVEL0                  (1 << 14)
#define D3DCOMPILE_OPTIMIZATION_LEVEL1                  0
#define D3DCOMPILE_OPTIMIZATION_LEVEL2                  ((1 << 14) | (1 << 15))
#define D3DCOMPILE_OPTIMIZATION_LEVEL3                  (1 << 15)
#define D3DCOMPILE_RESERVED16                           (1 << 16)
#define D3DCOMPILE_RESERVED17                           (1 << 17)
#define D3DCOMPILE_WARNINGS_ARE_ERRORS                  (1 << 18)
#define D3DCOMPILE_RESOURCES_MAY_ALIAS                  (1 << 19)
#define D3DCOMPILE_ENABLE_UNBOUNDED_DESCRIPTOR_TABLES   (1 << 20)
#define D3DCOMPILE_ALL_RESOURCES_BOUND                  (1 << 21)
#define D3DCOMPILE_DEBUG_NAME_FOR_SOURCE                (1 << 22)
#define D3DCOMPILE_DEBUG_NAME_FOR_BINARY                (1 << 23)



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

		for (int i = 0; i < ops.size(); i++)
		{
			auto op = ops[i];
			if (i <= offset) ar& NP("DONE", op);
			if (i == offset + 1) ar& NP("FAILED", op);
			if (i > offset + 1) ar& NP("WAITING", op);

		}
	}
}

