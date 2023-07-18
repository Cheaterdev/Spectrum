export module HAL;

export import :Format;

export import :Types;
export import :Sampler;

// TODO: Should be private
//export import :Utils;

export import :Device;
export import :Queue;
export import :Shader;
export import :HLSL;

export import :Fence;
export import :RootSignature;
export import :Descriptors;
export import :DescriptorHeap;
export import :Heap;
export import :HeapAllocators;
export import :TiledMemoryManager;
export import :ResourceStates;
export import :ResourceViews;
export import :Resource;
export import :Texture;
export import :SwapChain;
export import :Adapter;
//export import :Debug;

export import :Shader;
export import :TextureData;

export import :FrameManager;
//export import :API;
export import :PipelineState;
export import :CommandList;


export import :Raytracing;

export import :VirtualBuffer;
export import :Buffer;



export import :SIG;
export import :RT;
export import :Layout;
export import :Slots;
export import :PSO;
export import :RTX;
export import :Enums;

export import :Autogen;

export import :API.IndirectCommand;
export namespace HAL
{
	void init();
}