export module HAL;

export import :Format;

export import :Types;
export import :Sampler;

// TODO: Should be private
export import :Utils;
export import :Queue;
export import :Shader;
export import :HLSL;

export import :Fence;
export import :RootSignature;
export import :Descriptors;
export import :DescriptorHeap;
export import :Heap;
export import :HeapAllocators;
export import :TiledMemeoryManager;
export import :ResourceStates;
export import :ResourceViews;
export import :Resource;
export import :Texture;
export import :SwapChain;
export import :Adapter;
export import :Debug;
export import :Removeme;
export import :Shader;
export import :TextureData;
export import :API;

export namespace HAL
{
	void init();
}