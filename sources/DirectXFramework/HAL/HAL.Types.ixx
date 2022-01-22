export module HAL.Types;

import Vectors;
import Utils;

export namespace HAL
{
	enum class HeapType : uint
	{
		DEFAULT,
		UPLOAD,
		READBACK,
		RESERVED, // remove
	};

	enum class MemoryType : uint
	{
		PHYSICAL,
		VIRTUAL,
		COMMITED = PHYSICAL | VIRTUAL
	};

	enum class CommandListType : uint
	{
		DIRECT,
		COMPUTE,
		COPY,

		GENERATE_OPS
	};

	enum class HeapFlags :uint
	{
		NONE,
		BUFFERS_ONLY,
		TEXTURES_ONLY,
		RTDS_ONLY,

		GENERATE_OPS

	};
	enum class ResourceState : uint
	{
		COMMON = 0,
		VERTEX_AND_CONSTANT_BUFFER = 1 << 0,
		INDEX_BUFFER = 1 << 1,
		RENDER_TARGET = 1 << 2,
		UNORDERED_ACCESS = 1 << 3,
		DEPTH_WRITE = 1 << 4,
		DEPTH_READ = 1 << 5,
		NON_PIXEL_SHADER_RESOURCE = 1 << 6,
		PIXEL_SHADER_RESOURCE = 1 << 7,
		STREAM_OUT = 1 << 8,
		INDIRECT_ARGUMENT = 1 << 9,
		COPY_DEST = 1 << 10,
		COPY_SOURCE = 1 << 11,
		RESOLVE_DEST = 1 << 12,
		RESOLVE_SOURCE = 1 << 13,
		GEN_READ = (((((0x1 | 0x2) | 0x40) | 0x80) | 0x200) | 0x800),
		PRESENT = 0,
		RAYTRACING_STRUCTURE = 0x400000,
		UNKNOWN = std::numeric_limits<uint>::max() - 1
		, GENERATE_OPS
	};


	constexpr ResourceState COPY_STATES = ResourceState::COPY_DEST
		| ResourceState::COPY_SOURCE;

	constexpr ResourceState COMPUTE_STATES = COPY_STATES
		| ResourceState::UNORDERED_ACCESS
		| ResourceState::NON_PIXEL_SHADER_RESOURCE
		| ResourceState::INDIRECT_ARGUMENT;

	constexpr ResourceState GRAPHIC_STATES = COMPUTE_STATES
		| ResourceState::VERTEX_AND_CONSTANT_BUFFER
		| ResourceState::INDEX_BUFFER
		| ResourceState::RENDER_TARGET
		| ResourceState::DEPTH_WRITE
		| ResourceState::DEPTH_READ
		| ResourceState::PIXEL_SHADER_RESOURCE
		| ResourceState::STREAM_OUT

		| ResourceState::RESOLVE_DEST
		| ResourceState::RESOLVE_SOURCE;

	enum class HandleType : char
	{
		CBV,
		RTV,
		DSV,
		SRV,
		UAV,
		SMP,

		GENERATE_OPS
	};

	enum class DescriptorHeapFlags : char
	{
		NONE,
		SHADER_VISIBLE,

		GENERATE_OPS
	};


	enum class DescriptorHeapType : char
	{
		CBV_SRV_UAV,
		SAMPLER,
		RTV,
		DSV,

		GENERATE_OPS
	};

}
