export module HAL;

export import :Format;

export import :Types;
export import :Sampler;

// TODO: Should be private
export import :Utils;

export import :Shaders;


export import :Fence;
export import :RootSignature;
export import :Descriptors;
export import :Heap;
export import :HeapAllocators;
export import :TiledMemeoryManager;
export import :ResourceStates;

export import :Adapter;
export import :Debug;
export import :Removeme;


export import :API;

export namespace HAL
{
	void init();
}