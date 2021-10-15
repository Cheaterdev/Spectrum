namespace DX12
{
class Resource;
class Transitions;
struct swap_chain_desc;
struct RaytracingBuildDescBottomInputs;
struct RaytracingBuildDescTopInputs;
class SwapChain;
class Queue;
struct Handle;
//class Resource;
struct ResourceAllocationInfo;

class FrameResources;
class CommandList;
class ResourceHeap;

class PipelineState;
class ComputePipelineState;
class PipelineStateBase;
class StateObject;
class IndexBufferBase;
class UploadBuffer;
class CPUBuffer;

class GPUBuffer;

template<class T> class StructuredBuffer;
//  using BufferBase = GPUBuffer;
class QueryHeap;

class Texture;

class GraphicsContext;
class ComputeContext;
class CopyContext;
class FrameResourceManager;

}