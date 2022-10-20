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

	}
}