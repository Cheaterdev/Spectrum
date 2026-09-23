export module HAL:SIG;


import :Concepts;

import :DescriptorHeap;
import Core;
import :HLSL;

import :Enums;

export
{


	namespace Graphics
	{
		using Bindless = std::vector<HAL::Handle>;


	}


	using GPUAddress = uint64_t;

	template<class T>
	class Pointer
	{
		uint ptr;

	public:

		void operator=(const HLSL::ConstantBuffer<T>& t)
		{
			ptr = t.get_offset();
		}

			SERIALIZE()
		{
			 ar& NVP(ptr);
		}
	};
	class DrawIndexedArguments
	{
	public:

		UINT IndexCountPerInstance;
		UINT InstanceCount;
		UINT StartIndexLocation;
		INT  BaseVertexLocation;
		UINT StartInstanceLocation;

		using Compiled = DrawIndexedArguments;
		static const IndirectCommands CommandID = IndirectCommands::DrawIndexedArguments;

		template<class Processor> static void for_each(Processor& processor) {
			processor.template process<DrawIndexedArguments>();
		}
	private:
		SERIALIZE()
		{

			ar& NVP(IndexCountPerInstance);
			ar& NVP(InstanceCount);
			ar& NVP(StartIndexLocation);
			ar& NVP(BaseVertexLocation);
			ar& NVP(StartInstanceLocation);

		}

	};

	class DispatchMeshArguments
	{
	public:
		UINT ThreadGroupCountX;
		UINT ThreadGroupCountY;
		UINT ThreadGroupCountZ;

		using Compiled = DispatchMeshArguments;
		static const IndirectCommands CommandID = IndirectCommands::DispatchMeshArguments;


		template<class Processor> static void for_each(Processor& processor) {
			processor.template process<DispatchMeshArguments>();
		}


	private:
		SERIALIZE()
		{
			ar& NVP(ThreadGroupCountX);
			ar& NVP(ThreadGroupCountY);
			ar& NVP(ThreadGroupCountZ);
		}

	};


	class DispatchArguments
	{
	public:
		UINT ThreadGroupCountX;
		UINT ThreadGroupCountY;
		UINT ThreadGroupCountZ;

		using Compiled = DispatchArguments;
		static const IndirectCommands CommandID = IndirectCommands::DispatchArguments;

		template<class Processor> static void for_each(Processor& processor) {
			processor.template process<DispatchArguments>();
		}

	private:
		SERIALIZE()
		{
			ar& NVP(ThreadGroupCountX);
			ar& NVP(ThreadGroupCountY);
			ar& NVP(ThreadGroupCountZ);
		}
	};

	// GPU-driven indirect ray dispatch (D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH_RAYS,
	// DXR Tier 1.1). Byte-for-byte mirror of D3D12_DISPATCH_RAYS_DESC -- three
	// GPU-address ranges (raygen/miss/hit/callable, the last currently unused)
	// followed by Width/Height/Depth -- because ExecuteIndirect reads this
	// buffer's raw bytes straight off the GPU using that exact layout; field
	// order and width (GPUAddress = UINT64) must match, not just total size.
	// A shader populates it (see ddgi.prism's DDGIProbeDispatchArgsBuild): the
	// three shader-table addresses/sizes/strides come from the RTXPSO's own
	// tables (constant for the PSO's lifetime), Width/Height/Depth from
	// whatever this frame's actual dispatch size should be -- the whole
	// reason to go through ExecuteIndirect instead of a fixed CPU-recorded
	// dispatch is that the latter can be computed on the GPU (e.g. a
	// residency-compacted probe count) without a CPU readback.
	// D3D12 REQUIRES a command signature's ByteStride for a
	// D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH_RAYS argument to be at least 104
	// bytes (confirmed the hard way: CreateCommandSignature D3D12 ERROR #743,
	// "Command signature byte stride (100 bytes) is not large enough.
	// Required size is (104 bytes)." when this was tried packed to the
	// tight 100-byte sum of its real fields) -- natural C++ alignment
	// (GPUAddress = uint64_t forces 8-byte struct alignment, rounding 100 up
	// to 104) already gives the right size; do NOT #pragma pack(1) this.
	// The GPU-side mirror (DispatchRaysArguments in ddgi.prism /
	// dispatch_rays_args_build.hlsl) must match this 104-byte stride
	// exactly -- see its own comment for the explicit trailing pad field
	// that keeps it there, since HLSL has no equivalent automatic alignment
	// padding for structured-buffer elements.
	class DispatchRaysArguments
	{
	public:
		GPUAddress RayGenStartAddress;
		GPUAddress RayGenSizeInBytes;
		GPUAddress MissStartAddress;
		GPUAddress MissSizeInBytes;
		GPUAddress MissStrideInBytes;
		GPUAddress HitGroupStartAddress;
		GPUAddress HitGroupSizeInBytes;
		GPUAddress HitGroupStrideInBytes;
		GPUAddress CallableStartAddress;
		GPUAddress CallableSizeInBytes;
		GPUAddress CallableStrideInBytes;
		UINT Width;
		UINT Height;
		UINT Depth;

		using Compiled = DispatchRaysArguments;
		static const IndirectCommands CommandID = IndirectCommands::DispatchRaysArguments;

		template<class Processor> static void for_each(Processor& processor) {
			processor.template process<DispatchRaysArguments>();
		}

	private:
		SERIALIZE()
		{
			ar& NVP(RayGenStartAddress);
			ar& NVP(RayGenSizeInBytes);
			ar& NVP(MissStartAddress);
			ar& NVP(MissSizeInBytes);
			ar& NVP(MissStrideInBytes);
			ar& NVP(HitGroupStartAddress);
			ar& NVP(HitGroupSizeInBytes);
			ar& NVP(HitGroupStrideInBytes);
			ar& NVP(CallableStartAddress);
			ar& NVP(CallableSizeInBytes);
			ar& NVP(CallableStrideInBytes);
			ar& NVP(Width);
			ar& NVP(Height);
			ar& NVP(Depth);
		}
	};

	using DefaultCB = HAL::Buffer*;// std::vector<std::byte>;
	using DynamicData = std::vector<std::byte>;// std::vector<std::byte>;




	struct EntryPointsCompiled
	{

		struct Record
		{
			HAL::GPUAddressPtr address;
			uint size;

		};
		//std::vector<Record>

	};
	
}