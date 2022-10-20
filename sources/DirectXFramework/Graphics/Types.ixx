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
		using RTVState = HAL::RTVState;

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

	}
}