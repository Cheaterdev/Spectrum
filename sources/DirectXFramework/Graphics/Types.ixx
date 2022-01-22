export module Graphics.Types;

import HAL.Types;
import HAL.Sampler;

export {
	namespace Graphics
	{
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
	
	}
}