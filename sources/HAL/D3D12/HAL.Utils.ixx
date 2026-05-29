export module HAL:Utils;


import stl.core;

import d3d12;
import wrl;
import Core;

import :Types;
import :Sampler;
using namespace HAL;

static_assert(D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES == 32);

export namespace D3D
{
	using ResourceDesc = D3D12_RESOURCE_DESC1;

	using Heap = ComPtr<ID3D12Heap>;
	using Resource = ComPtr<ID3D12Resource>;
	using Device = ComPtr<ID3D12Device14>;
	using Fence = ComPtr<ID3D12Fence>;
	using CommandList = ComPtr<ID3D12GraphicsCommandList10>;
	using CommandAllocator = ComPtr<ID3D12CommandAllocator>;
	using CommandSignature = ComPtr<ID3D12CommandSignature>;
	using PipelineState = ComPtr<ID3D12PipelineState>;
	using StateObject = ComPtr<ID3D12StateObject>;
	using StateObjectProperties = ComPtr<ID3D12StateObjectProperties>;
	using Blob = ComPtr<ID3DBlob>;
	using DescriptorHeap = ComPtr<ID3D12DescriptorHeap>;
	using QueryHeap = ComPtr<ID3D12QueryHeap>;
	using RootSignature = ComPtr<ID3D12RootSignature>;
	using CommandQueue = ComPtr<ID3D12CommandQueue>;

	using Debug = ComPtr<ID3D12Debug1>;
	using InfoQueue = ComPtr<ID3D12InfoQueue>;


	using StorageFactory = ComPtr<IDStorageFactory>;
	using StorageFile = ComPtr<IDStorageFile>;
	using StorageQueue = ComPtr<IDStorageQueue2>;
}

export namespace DXGI
{
	using Adapter = ComPtr<IDXGIAdapter4>;
	using Swapchain = ComPtr<IDXGISwapChain4>;
	using Surface = ComPtr<IDXGISurface2>;
	using Device = ComPtr<IDXGIDevice2>;
	using Factory = ComPtr<IDXGIFactory7>;
	using Output = ComPtr<IDXGIOutput6>;
}

export D3D12_TEXTURE_ADDRESS_MODE to_native(TextureAddressMode mode);

export D3D12_PRIMITIVE_TOPOLOGY_TYPE to_native(PrimitiveTopologyType topology);

export D3D12_COMPARISON_FUNC to_native(ComparisonFunc mode);


export D3D12_FILTER to_native(Filter min, Filter mag, Filter mip, ComparisonFunc func);



export D3D12_SAMPLER_DESC to_native(const SamplerDesc& desc);

export D3D12_COMMAND_LIST_TYPE to_native(CommandListType type);

export D3D12_HEAP_TYPE to_native(HeapType type);

export HeapType from_native(D3D12_HEAP_TYPE type);

export D3D12_HEAP_FLAGS to_native(HeapFlags flags);

export D3D12_DESCRIPTOR_HEAP_FLAGS to_native(DescriptorHeapFlags flags);

export D3D12_DESCRIPTOR_HEAP_TYPE to_native(DescriptorHeapType type);

export D3D12_CULL_MODE to_native(CullMode mode);

export D3D12_FILL_MODE to_native(FillMode mode);

export D3D12_BLEND to_native(Blend blend);

export D3D12_STENCIL_OP to_native(StencilOp mode);

export D3D12_DEPTH_STENCILOP_DESC to_native(StencilDesc desc);

export D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS to_native(RaytracingBuildFlags flags);

export DXGI_FORMAT to_native(Format format);


export Format from_native(DXGI_FORMAT format);

export D3D12_DESCRIPTOR_RANGE_TYPE to_native(DescriptorRange range);

export D3D12_SHADER_VISIBILITY to_native(ShaderVisibility visibility);

//
//export D3D12_BARRIER_LAYOUT to_native(TextureLayout flags)
//{
//
//	if (flags == TextureLayout::UNDEFINED) return  D3D12_BARRIER_LAYOUT_UNDEFINED;
//
//
//	D3D12_BARRIER_LAYOUT result = D3D12_BARRIER_LAYOUT::D3D12_BARRIER_LAYOUT_COMMON;
//
//
//
//
//	if (check(flags & TextureLayout::COMMON)) result |= D3D12_BARRIER_LAYOUT::D3D12_BARRIER_LAYOUT_COMMON;
//	if (check(flags & TextureLayout::PRESENT)) result |= D3D12_BARRIER_LAYOUT::D3D12_BARRIER_LAYOUT_PRESENT;
//	if (check(flags & TextureLayout::RENDER_TARGET)) result |= D3D12_BARRIER_LAYOUT::D3D12_BARRIER_LAYOUT_RENDER_TARGET;
//	if (check(flags & TextureLayout::UNORDERED_ACCESS)) result |= D3D12_BARRIER_LAYOUT::D3D12_BARRIER_LAYOUT_UNORDERED_ACCESS;
//	if (check(flags & TextureLayout::DEPTH_STENCIL_WRITE)) result |= D3D12_BARRIER_LAYOUT::D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_WRITE;
//	if (check(flags & TextureLayout::DEPTH_STENCIL_READ)) result |= D3D12_BARRIER_LAYOUT::D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_READ;
//	if (check(flags & TextureLayout::SHADER_RESOURCE)) result |= D3D12_BARRIER_LAYOUT::D3D12_BARRIER_LAYOUT_SHADER_RESOURCE;
//	if (check(flags & TextureLayout::COPY_DEST)) result |= D3D12_BARRIER_LAYOUT::D3D12_BARRIER_LAYOUT_COPY_DEST;
//	if (check(flags & TextureLayout::COPY_SOURCE)) result |= D3D12_BARRIER_LAYOUT::D3D12_BARRIER_LAYOUT_COPY_SOURCE;
//	if (check(flags & TextureLayout::RESOLVE_DEST)) result |= D3D12_BARRIER_LAYOUT::D3D12_BARRIER_LAYOUT_RESOLVE_DEST;
//	if (check(flags & TextureLayout::RESOLVE_SOURCE)) result |= D3D12_BARRIER_LAYOUT::D3D12_BARRIER_LAYOUT_RESOLVE_SOURCE;
//
//	return result;
//}



export D3D12_BARRIER_LAYOUT to_native(TextureLayout layout);




export D3D12_BARRIER_SYNC  to_native(BarrierSync flags);




export D3D12_BARRIER_ACCESS  to_native(BarrierAccess flags);

export D3D12_RESOURCE_FLAGS to_native(const ResFlags& flags);


export ResFlags from_native(D3D12_RESOURCE_FLAGS flags);


export GPUAddressPtr to_native(const ResourceAddress& address);

export D3D12_VIEWPORT to_native(Viewport v);


export D3D_PRIMITIVE_TOPOLOGY to_native(HAL::PrimitiveTopologyType topology, HAL::PrimitiveTopologyFeed feedType, bool adjusted, uint controlpoints);

export D3D12_HIT_GROUP_TYPE  to_native(HitGroupType type);



export D3D12_RAYTRACING_GEOMETRY_TYPE   to_native(GeometryType type);



export D3D12_RAYTRACING_GEOMETRY_FLAGS   to_native(GeometryFlags flags);

export struct RaytracingDescNative :public D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS
{
	std::vector<D3D12_RAYTRACING_GEOMETRY_DESC> descs;


};


export RaytracingDescNative to_native(const RaytracingBuildDescBottomInputs& inputs);

export D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS  to_native(const RaytracingBuildDescTopInputs& inputs);

export CD3DX12_RESOURCE_DESC  to_native_1(const ResourceDesc& desc);

export CD3DX12_RESOURCE_DESC1  to_native(const ResourceDesc& desc);

export namespace cereal
{

	template<class Archive>
	void serialize(Archive& ar, D3D12_DISPATCH_ARGUMENTS& g)
	{
		ar& NVPG(ThreadGroupCountX);
		ar& NVPG(ThreadGroupCountY);
		ar& NVPG(ThreadGroupCountZ);
	}


	  	template<class Archive>
	void serialize(Archive& ar, D3D12_RAYTRACING_INSTANCE_DESC& g)
	{

	}
		     	template<class Archive>
	void serialize(Archive& ar, D3D12_DRAW_INDEXED_ARGUMENTS& g)
	{
		ar& NVPG(IndexCountPerInstance);
			ar& NVPG(InstanceCount);
			ar& NVPG(StartIndexLocation);
			ar& NVPG(BaseVertexLocation);
			ar& NVPG(StartInstanceLocation);
	}

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
	void serialize(Archive& ar, D3D::ResourceDesc& g, const unsigned int)
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
