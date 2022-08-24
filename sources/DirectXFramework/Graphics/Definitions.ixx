export module Graphics:Definitions;


export
{
	namespace Graphics
	{
		//struct ResourceAddress;
		class Resource;
		class Transitions;
		struct swap_chain_desc;
		struct RaytracingBuildDescBottomInputs;
		struct RaytracingBuildDescTopInputs;
		class SwapChain;
		class Queue;
		struct Handle;
		//class Resource;
		class DescriptorHeap;
	//	struct ResourceAllocationInfo;
		class FrameResources;
		class CommandList;
		class ResourceHeap;

		//class PipelineState;
		class ComputePipelineState;
		class PipelineStateBase;
		class StateObject;
		class IndexBufferBase;

		class GPUBuffer;

		template<class T> class StructuredBuffer;
		//  using BufferBase = GPUBuffer;
		class QueryHeap;

		class Texture;

		class GraphicsContext;
		class ComputeContext;
		class CopyContext;
		class FrameResourceManager;

		class GPUTimeManager;
		class Eventer;

		struct texure_header;

	}

}