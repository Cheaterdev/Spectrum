export module Graphics:Types;

export import HAL;

export {
	namespace Graphics
	{
		using Resource = HAL::Resource;
		using DescriptorRange = HAL::DescriptorRange;
		using ShaderVisibility = HAL::ShaderVisibility;

		using SamplerDesc = HAL::SamplerDesc;
		using Filter = HAL::Filter;
		using TextureAddressMode = HAL::TextureAddressMode;
		using ComparisonFunc = HAL::ComparisonFunc;



		using HeapType = HAL::HeapType;
		using MemoryType = HAL::MemoryType;
		using CommandListType = HAL::CommandListType;
		using ResourceState = HAL::ResourceState;
		using HandleType = HAL::HandleType;
		using DescriptorHeapFlags = HAL::DescriptorHeapFlags;
		using DescriptorHeapType = HAL::DescriptorHeapType;


		using Blend = HAL::Blend;
		using CullMode = HAL::CullMode;
		using FillMode = HAL::FillMode;

		using StencilOp = HAL::StencilOp;
		using ComparisonFunc = HAL::ComparisonFunc;


		using RenderTarget = HAL::RenderTarget;
		using RasterizerState = HAL::RasterizerState;
		using BlendState = HAL::BlendState;
		using RenderTargetState = HAL::RenderTargetState;

		using Format = HAL::Format;
		using Handle = HAL::Handle;
		using HandleTableLight = HAL::HandleTableLight;
		using HandleTable = HAL::HandleTable;

		using ResourceInfo = HAL::ResourceInfo;
		using DescriptorHeap = HAL::DescriptorHeap;


		using ResourceHeap = HAL::Heap;

		using ResourceHeapPage = HAL::ResourceHeapPage;
		using ResourceHandle = HAL::ResourceHandle;

		template<class LockPolicy>
		using ResourceHeapAllocator = HAL::ResourceHeapAllocator<LockPolicy>;

		using Event = HAL::Event;
		using Fence = HAL::Fence;
		using FenceWaiter = HAL::FenceWaiter;

		using IndirectCommand = HAL::IndirectCommand;


		using DescriptorConstBuffer = HAL::DescriptorConstBuffer;
		using RootSignatureDesc = HAL::RootSignatureDesc;
		using RootSignatureType = HAL::RootSignatureType;

		using RootSignature = HAL::RootSignature;

		using Viewport = HAL::Viewport;

	using TextureView = HAL::TextureView;
	using Texture3DView = HAL::Texture3DView;
	using CubeView = HAL::CubeView;
	using TextureViewDesc = HAL::TextureViewDesc;
	using CubeViewDesc = HAL::CubeViewDesc;
	using Texture3DViewDesc = HAL::Texture3DViewDesc;
	using FormattedBufferViewDesc = HAL::FormattedBufferViewDesc;
using StructuredBufferViewDesc = HAL::StructuredBufferViewDesc;
template<class T, Format::Formats _format>
using FormattedBufferView = HAL::FormattedBufferView<T, _format>;

template<class T>
using StructuredBufferView = HAL::StructuredBufferView<T>;

using RTXSceneView = HAL::RTXSceneView;
	using BufferView = HAL::BufferView;
	using ResourceView = HAL::ResourceView;


	using Texture = HAL::Texture;
using HandleTable = HAL::HandleTable;



	using vertex_shader = HAL::vertex_shader;
	using pixel_shader = HAL::pixel_shader;
	using geometry_shader = HAL::geometry_shader;
	using hull_shader = HAL::hull_shader;
	using domain_shader = HAL::domain_shader;

	using mesh_shader = HAL::mesh_shader;
	using amplification_shader = HAL::amplification_shader;
	using compute_shader = HAL::compute_shader;
	using library_shader = HAL::library_shader;
	using Device = HAL::Device;


using PipelineStateBase = HAL::PipelineStateBase;
using ComputePipelineState = HAL::ComputePipelineState;

using PipelineState = HAL::PipelineState;

using ComputePipelineStateDesc = HAL::ComputePipelineStateDesc;

using PipelineStateDesc = HAL::PipelineStateDesc;


using StateObjectDesc = HAL::StateObjectDesc;
using StateObject = HAL::StateObject;
using LibraryObject = HAL::LibraryObject;

	}
}