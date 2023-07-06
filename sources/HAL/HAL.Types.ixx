export module HAL:Types;
import <HAL.h>;
import Core;

import :Format;
import windows;

#undef DOMAIN

export namespace HAL
{

	struct GlobalAllocationPolicy
	{
		using AllocatorType = CommonAllocator;
		using LockPolicy = Thread::Lockable;
	};


	struct FrameAllocationPolicy
	{
		using AllocatorType = LinearAllocator;
		using LockPolicy = Thread::Lockable;
	};

	struct LocalAllocationPolicy
	{
		using AllocatorType = LinearAllocator;
		using LockPolicy = Thread::Free;
	};

	class Resource;
	class Device;
	class Queue;


	using GPUAddressPtr = uint64;

	struct Viewport
	{
		float2 pos;
		float2 size;
		float2 depths;
	};

	enum class DescriptorRange : int
	{
		SRV,
		UAV,
		CBV,
		SAMPLER
	};

	enum class ShaderVisibility : int
	{
		ALL,
		VERTEX,
		HULL,
		DOMAIN,
		GEOMETRY,
		PIXEL,
		AMPLIFICATION,
		MESH
	};

	enum class HeapType : uint
	{
		DEFAULT,
		UPLOAD,
		READBACK,
		RESERVED, // remove
	};

	enum class MemoryType : uint
	{
		PHYSICAL = 1,
		VIRTUAL = 2,
		COMMITED = PHYSICAL | VIRTUAL
	};

	enum class CommandListType : uint
	{
		DIRECT,
		COMPUTE,
		COPY
	};

	enum class HeapFlags :uint
	{
		NONE,
		BUFFERS_ONLY,
		TEXTURES_ONLY,
		RTDS_ONLY

	};


	enum class TextureLayout : uint
	{

	
		NONE = 0,		
		PRESENT = 1 << 0,
	//	GENERIC_READ,
		RENDER_TARGET= 1 << 1,
		UNORDERED_ACCESS= 1 << 2,
		DEPTH_STENCIL_WRITE= 1 << 3,
		DEPTH_STENCIL_READ= 1 << 4,
		SHADER_RESOURCE= 1 << 5,
		COPY_SOURCE= 1 << 6,
		COPY_DEST= 1 << 7,

		COPY_QUEUE = 1<<8,
		//RESOLVE_SOURCE,
		//RESOLVE_DEST,
		//SHADING_RATE_SOURCE,
		//VIDEO_DECODE_READ,
		//VIDEO_DECODE_WRITE,
		//VIDEO_PROCESS_READ,
		//VIDEO_PROCESS_WRITE,
		//VIDEO_ENCODE_READ,
		//VIDEO_ENCODE_WRITE,
		//DIRECT_QUEUE_COMMON,
		//DIRECT_QUEUE_GENERIC_READ,
		//DIRECT_QUEUE_UNORDERED_ACCESS,
		//DIRECT_QUEUE_SHADER_RESOURCE,
		//DIRECT_QUEUE_COPY_SOURCE,
		//DIRECT_QUEUE_COPY_DEST,
		//COMPUTE_QUEUE_COMMON,
		//COMPUTE_QUEUE_GENERIC_READ,
		//COMPUTE_QUEUE_UNORDERED_ACCESS,
		//COMPUTE_QUEUE_SHADER_RESOURCE,
		//COMPUTE_QUEUE_COPY_SOURCE,
		//COMPUTE_QUEUE_COPY_DEST,
		//VIDEO_QUEUE_COMMON,
		UNDEFINED = 0,
		
	};
	
	CommandListType get_best_cmd_type(TextureLayout layout);
	enum class BarrierSync : uint
	{
		NONE = 0x0,
	//	ALL = 0x1,
	
		INDEX_INPUT = 1<<0,
		VERTEX_SHADING = 1<<1,
		PIXEL_SHADING =  1<<2,
		DEPTH_STENCIL =  1<<3,
		RENDER_TARGET = 1<<4,
		COMPUTE_SHADING = 1<<5,
		RAYTRACING =  1<<6,
		COPY =  1<<7,
		RESOLVE =  1<<8,
		EXECUTE_INDIRECT =  1<<9,
		PREDICATION =  1<<10,
		EMIT_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO =  1<<11,
		CLEAR_UNORDERED_ACCESS_VIEW =  1<<12,
		VIDEO_DECODE =  1<<13,
		VIDEO_PROCESS =  1<<14,
		VIDEO_ENCODE =  1<<15,
		BUILD_RAYTRACING_ACCELERATION_STRUCTURE = 1<<16,
		COPY_RAYTRACING_ACCELERATION_STRUCTURE =  1<<17,
		SPLIT = 0x80000000,
	
			DRAW = INDEX_INPUT | VERTEX_SHADING | PIXEL_SHADING | DEPTH_STENCIL | RENDER_TARGET,
	ALL_SHADING = VERTEX_SHADING | PIXEL_SHADING | COMPUTE_SHADING,
	NON_PIXEL_SHADING = COMPUTE_SHADING | VERTEX_SHADING,

		ALL_COMPUTE = CLEAR_UNORDERED_ACCESS_VIEW | COMPUTE_SHADING | RAYTRACING | RAYTRACING | COPY | EXECUTE_INDIRECT | EMIT_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO | BUILD_RAYTRACING_ACCELERATION_STRUCTURE,
		ALL_DIRECT = ALL_COMPUTE | DRAW | ALL_SHADING
	};

	enum class BarrierAccess : uint
	{
		COMMON = 0,
		VERTEX_BUFFER = 0x1,
		CONSTANT_BUFFER = 0x2,
		INDEX_BUFFER = 0x4,
		RENDER_TARGET = 0x8,
		UNORDERED_ACCESS = 0x10,
		DEPTH_STENCIL_WRITE = 0x20,
		DEPTH_STENCIL_READ = 0x40,
		SHADER_RESOURCE = 0x80,
		STREAM_OUTPUT = 0x100,
		INDIRECT_ARGUMENT = 0x200,
		PREDICATION = 0x200, // Aliased with ACCESS_INDIRECT_ARGUMENT
		COPY_DEST = 0x400,
		COPY_SOURCE = 0x800,
		RESOLVE_DEST = 0x1000,
		RESOLVE_SOURCE = 0x2000,
		RAYTRACING_ACCELERATION_STRUCTURE_READ = 0x4000,
		RAYTRACING_ACCELERATION_STRUCTURE_WRITE = 0x8000,
		SHADING_RATE_SOURCE = 0x10000,
		VIDEO_DECODE_READ = 0x20000,
		VIDEO_DECODE_WRITE = 0x40000,
		VIDEO_PROCESS_READ = 0x80000,
		VIDEO_PROCESS_WRITE = 0x100000,
		VIDEO_ENCODE_READ = 0x200000,
		VIDEO_ENCODE_WRITE = 0x400000,
		NO_ACCESS = 0x80000000
	};

	//constexpr BarrierAccess operator |( BarrierAccess const& lhs,BarrierAccess const& rhs)
	//{
	//	using underlying = typename std::underlying_type<BarrierAccess>::type;
	//	return static_cast<BarrierAccess> (
	//		static_cast<underlying>(lhs) |
	//		static_cast<underlying>(rhs)
	//		);
	//}

	//constexpr BarrierSync operator |( BarrierSync const& lhs,BarrierSync const& rhs)
	//{
	//	using underlying = typename std::underlying_type<BarrierSync>::type;
	//	return static_cast<BarrierSync> (
	//		static_cast<underlying>(lhs) |
	//		static_cast<underlying>(rhs)
	//		);
	//}
	//
	//constexpr TextureLayout operator |( TextureLayout const& lhs,TextureLayout const& rhs)
	//{
	//	using underlying = typename std::underlying_type<TextureLayout>::type;
	//	return static_cast<TextureLayout> (
	//		static_cast<underlying>(lhs) |
	//		static_cast<underlying>(rhs)
	//		);
	//}
struct ResourceState
{
	BarrierSync operation;//= BarrierSync::NONE;
	BarrierAccess access;// = BarrierAccess::NO_ACCESS;
	TextureLayout layout;// = TextureLayout::UNDEFINED;

	ResourceState(BarrierSync s= BarrierSync::NONE, BarrierAccess a= BarrierAccess::NO_ACCESS, TextureLayout l = TextureLayout::UNDEFINED) ;
	bool operator==(const ResourceState&) const = default;

	constexpr BarrierSync get_operation()const { return operation; }
	constexpr BarrierAccess get_access() const { return access; }

	constexpr TextureLayout get_layout() const { return layout; }


	ResourceState operator |(const ResourceState& state)const;
	ResourceState operator &(const ResourceState& state)const;
	const ResourceState& operator |=(const ResourceState& state);
	const ResourceState& operator &=(const ResourceState& state);


	CommandListType get_best_cmd_type() const;
	bool has_write_bits() const;

	bool is_no_access() const;
	bool is_valid() const;
	//{

	//	operation |= state.operation;
	//	access |= state.access;

	//	assert(layout == state.layout);
	//	return *this;
	//}
};
//ResourceState operator | (ResourceState a, ResourceState b);
//
//constexpr ResourceState operator | (ResourceState a, ResourceState b)
//{
//	//ResourceState other;
//	//other.operation = EnumOps::OR(a.operation,b.operation);
//	//other.access = EnumOps::OR(a.access,b.access);
//	//other.layout = a.layout;
//	//test t = test::A | test::B;
//	//assert(a.layout == b.layout);
//	return ResourceState(a.get_operation() | b.get_operation(), a.get_access() | b.get_access(), a.get_layout());
//}

	namespace ResourceStates
	{

			extern const  ResourceState INDEX_BUFFER ;//= { BarrierSync::INDEX_INPUT, BarrierAccess::INDEX_BUFFER, TextureLayout::UNDEFINED };
		extern const   ResourceState COPY_SOURCE;// = { BarrierSync::COPY, BarrierAccess::COPY_SOURCE, TextureLayout::COPY_SOURCE };
		extern const   ResourceState COPY_DEST;// = { BarrierSync::COPY, BarrierAccess::COPY_DEST, TextureLayout::COPY_DEST };

		extern const  ResourceState SHADER_RESOURCE;// = { BarrierSync::PIXEL_SHADING, BarrierAccess::SHADER_RESOURCE, TextureLayout::SHADER_RESOURCE };
	//	extern const  ResourceState NON_PIXEL_SHADER_RESOURCE;// = { BarrierSync::NON_PIXEL_SHADING, BarrierAccess::SHADER_RESOURCE, TextureLayout::SHADER_RESOURCE };
		extern const  ResourceState UNORDERED_ACCESS;// = { BarrierSync::ALL_SHADING, BarrierAccess::UNORDERED_ACCESS, TextureLayout::UNORDERED_ACCESS };
		extern const  ResourceState RAYTRACING_STRUCTURE;// = { BarrierSync::RAYTRACING, BarrierAccess::RAYTRACING_ACCELERATION_STRUCTURE_READ, TextureLayout::UNDEFINED };

		extern const  ResourceState RAYTRACING_STRUCTURE_WRITE;// = { BarrierSync::RAYTRACING, BarrierAccess::RAYTRACING_ACCELERATION_STRUCTURE_READ|BarrierAccess::RAYTRACING_ACCELERATION_STRUCTURE_WRITE, TextureLayout::UNDEFINED };
		extern const  ResourceState INDIRECT_ARGUMENT;// = { BarrierSync::EXECUTE_INDIRECT, BarrierAccess::INDIRECT_ARGUMENT, TextureLayout::UNDEFINED };

		extern const  ResourceState RENDER_TARGET;// = {BarrierSync::RENDER_TARGET, BarrierAccess::RENDER_TARGET, TextureLayout::RENDER_TARGET};
		extern const  ResourceState DEPTH_STENCIL ;//= {BarrierSync::DEPTH_STENCIL, BarrierAccess::DEPTH_STENCIL_WRITE | BarrierAccess::DEPTH_STENCIL_READ,TextureLayout::DEPTH_STENCIL_WRITE|TextureLayout::DEPTH_STENCIL_READ};

	//	extern const  ResourceState CONSTANT_BUFFER ;//= {BarrierSync::ALL, BarrierAccess::CONSTANT_BUFFER, TextureLayout::UNDEFINED};

		extern const  ResourceState NO_ACCESS ;
//extern const  ResourceState WRITE_STATES ;
extern const  ResourceState UNKNOWN ;
	/*	extern const  ResourceState INDEX_BUFFER = { BarrierSync::INDEX_INPUT, BarrierAccess::INDEX_BUFFER, TextureLayout::UNDEFINED };
		extern const  ResourceState COPY_SOURCE = { BarrierSync::COPY, BarrierAccess::COPY_SOURCE, TextureLayout::COPY_SOURCE };
		extern const  ResourceState COPY_DEST = { BarrierSync::COPY, BarrierAccess::COPY_DEST, TextureLayout::COPY_DEST };

		extern const  ResourceState PIXEL_SHADER_RESOURCE = { BarrierSync::PIXEL_SHADING, BarrierAccess::SHADER_RESOURCE, TextureLayout::SHADER_RESOURCE };
		extern const  ResourceState NON_PIXEL_SHADER_RESOURCE = { BarrierSync::NON_PIXEL_SHADING, BarrierAccess::SHADER_RESOURCE, TextureLayout::SHADER_RESOURCE };
		extern const  ResourceState UNORDERED_ACCESS = { BarrierSync::ALL_SHADING, BarrierAccess::UNORDERED_ACCESS, TextureLayout::UNORDERED_ACCESS };
		extern const  ResourceState RAYTRACING_STRUCTURE = { BarrierSync::RAYTRACING, BarrierAccess::RAYTRACING_ACCELERATION_STRUCTURE_READ, TextureLayout::UNDEFINED };

		inline  constexpr ResourceState RAYTRACING_STRUCTURE_WRITE = { BarrierSync::RAYTRACING, BarrierAccess::RAYTRACING_ACCELERATION_STRUCTURE_READ|BarrierAccess::RAYTRACING_ACCELERATION_STRUCTURE_WRITE, TextureLayout::UNDEFINED };
		extern const  ResourceState INDIRECT_ARGUMENT = { BarrierSync::EXECUTE_INDIRECT, BarrierAccess::INDIRECT_ARGUMENT, TextureLayout::UNDEFINED };

		extern const  ResourceState RENDER_TARGET = {BarrierSync::RENDER_TARGET, BarrierAccess::RENDER_TARGET, TextureLayout::RENDER_TARGET};
		extern const  ResourceState DEPTH_STENCIL = {BarrierSync::DEPTH_STENCIL, BarrierAccess::DEPTH_STENCIL_WRITE | BarrierAccess::DEPTH_STENCIL_READ,TextureLayout::DEPTH_STENCIL_WRITE|TextureLayout::DEPTH_STENCIL_READ};

		extern const  ResourceState CONSTANT_BUFFER = {BarrierSync::ALL, BarrierAccess::CONSTANT_BUFFER, TextureLayout::UNDEFINED};*/
	}
		
	enum class HandleType : char
	{
		CBV,
		RTV,
		DSV,
		SRV,
		UAV,
		SMP
	};

	enum class DescriptorHeapFlags : char
	{
		None,
		ShaderVisible
	};


	enum class DescriptorHeapType : char
	{
		CBV_SRV_UAV,
		SAMPLER,
		RTV,
		DSV
	};




	enum class CullMode : char
	{
		None,
		Back,
		Front
	};

	enum class FillMode : char
	{
		Wireframe,
		Solid
	};

	enum class Blend : char
	{
		Zero,
		One,
		SrcColor,
		InvSrcColor,
		SrcAlpha,
		InvSrcAlpha,
		DstAlpha,
		InvDstAlpha,
		DestColor,
		InvDestColor,
		SrcAlphaSat,
		BlendFactor,
		InvBlendFactor,
		Src1Color,
		InvSrc1Color,
		Src1Alpha,
		InvSrc1Alpha
	};

	enum class StencilOp : char
	{
		Keep,
		Zero,
		Replace,
		IncrementSat,
		DecrementSat,
		Invert,
		Increment,
		Decrement
	};

	enum class ComparisonFunc : uint
	{
		NEVER,
		LESS,
		EQUAL,
		LESS_EQUAL,
		GREATER,
		NOT_EQUAL,
		GREATER_EQUAL,
		ALWAYS,
		NONE,
		MIN,
		MAX
	};

	struct StencilDesc
	{
		StencilOp      StencilFailOp;
		StencilOp      StencilDepthFailOp;
		StencilOp      StencilPassOp;
		ComparisonFunc StencilFunc;

		bool operator==(const StencilDesc&) const = default;
		auto operator<=>(const StencilDesc& r)  const = default;

	private:
		SERIALIZE()
		{
			ar& NVP(StencilFailOp);
			ar& NVP(StencilDepthFailOp);
			ar& NVP(StencilPassOp);
			ar& NVP(StencilFunc);
		}
	};

	struct RasterizerState
	{
		CullMode cull_mode = CullMode::Back;
		FillMode fill_mode = FillMode::Solid;
		bool conservative = false;
		bool operator==(const RasterizerState&) const = default;
		auto operator<=>(const  RasterizerState& r)  const = default;

	private:
		SERIALIZE()
		{
			ar& NVP(cull_mode);
			ar& NVP(fill_mode);
			ar& NVP(conservative);
		}
	};

	struct RenderTarget
	{
		bool enabled = false;
		Blend dest = Blend::InvSrcAlpha;
		Blend source = Blend::SrcAlpha;
		Blend dest_alpha = Blend::Zero;
		Blend source_alpha = Blend::One;
		bool operator==(const RenderTarget&) const = default;
		std::strong_ordering  operator<=>(const  RenderTarget& r)  const = default;
	private:
		SERIALIZE()
		{
			ar& NVP(enabled);
			ar& NVP(source);
			ar& NVP(dest);
			ar& NVP(dest_alpha);
			ar& NVP(source_alpha);
		}
	};

	struct BlendState
	{
		bool independ_blend = false;

		std::array<RenderTarget, 8> render_target;
		bool operator==(const BlendState&) const = default;
		std::strong_ordering operator<=>(const  BlendState& r)  const = default;


	private:
		SERIALIZE()
		{
			ar& NVP(independ_blend);

			ar& NVP(render_target);
			for (auto& r : render_target) // ICE Here
				ar& NVP(r);
		}
	};


	struct RenderTargetState
	{
		bool enable_depth = false;
		bool enable_depth_write = true;


		Format ds_format = Format::UNKNOWN;
		std::vector<Format> rtv_formats;
		ComparisonFunc func = ComparisonFunc::LESS;

		bool enable_stencil = false;
		uint8 stencil_read_mask = 0xff;
		uint8 stencil_write_mask = 0xff;
		StencilDesc stencil_desc;

		RenderTargetState()
		{
			stencil_desc.StencilDepthFailOp = StencilOp::Keep;
			stencil_desc.StencilFailOp = StencilOp::Keep;
			stencil_desc.StencilPassOp = StencilOp::Keep;
			stencil_desc.StencilFunc = ComparisonFunc::LESS;
		}
		bool operator==(const RenderTargetState& r) const = default;
		auto operator<=>(const  RenderTargetState& r)  const = default;
	private:
		SERIALIZE()
		{
			ar& NVP(enable_depth);
			ar& NVP(enable_depth_write);
			ar& NVP(ds_format);
			ar& NVP(rtv_formats);
			ar& NVP(func);
			ar& NVP(enable_stencil);
			ar& NVP(stencil_read_mask);
			ar& NVP(stencil_write_mask);
			ar& NVP(stencil_desc);
		}

	};


	enum class RTOptions :uint
	{
		SetHandles = 1,
		ClearDepth = 1 << 1,
		ClearStencil = 1 << 2,
		SetViewport = 1 << 3,
		SetScissors = 1 << 4,

		ClearColor = 1 << 5,
		ClearDepthStencil = ClearDepth | ClearStencil,
		ClearAll = ClearDepthStencil | ClearColor,
		Default = SetHandles | SetScissors | SetViewport,
	};

	enum class ResFlags :uint
	{
		None = 0,
		ShaderResource = 1 << 0,
		UnorderedAccess = 1 << 1,
		RenderTarget = 1 << 2,
		DepthStencil = 1 << 3,
		CrossAdapter = 1 << 4,
		Raytracing = 1<<5,
		SimultaniousAccess = 1<<6
	};

	struct TextureDesc
	{
		uint3 Dimensions;
		uint ArraySize;
		uint MipLevels;
		Format Format;

		bool is1D() const { return Dimensions.y == 0 && Dimensions.z == 0; }
		bool is2D() const { return Dimensions.y != 0 && Dimensions.z == 0; }
		bool is3D() const { return Dimensions.y != 0 && Dimensions.z != 0; }

		bool operator==(const TextureDesc&) const = default;
		bool operator!=(const TextureDesc&) const = default;
		auto operator<=>(const TextureDesc&)  const = default;

		inline uint CalcSubresource(uint MipSlice, uint ArraySlice, uint PlaneSlice) const
		{
			return MipSlice + ArraySlice * MipLevels + PlaneSlice * MipLevels * ArraySize;
		}

		uint Subresources() const
		{
			return ArraySize*MipLevels;
		}

		uint3 get_size(uint subres) const
		{
			return uint3::max({1,1,1}, Dimensions/std::pow(2,subres));
		}

		uint get_mip(uint subres) const
		{
		return subres % MipLevels;
		}


		uint get_array(uint subres) const
		{
		return subres/ MipLevels;
		}

		uint get_plane(uint subres) const
		{
		return subres/ (MipLevels * ArraySize);
		}
	private:
		SERIALIZE()
		{
			ar& NVP(Dimensions);
			ar& NVP(ArraySize);
			ar& NVP(MipLevels);
			ar& NVP(Format);
		}

	};

	struct BufferDesc
	{
		size_t SizeInBytes;

		bool operator==(const BufferDesc&) const = default;
		bool operator!=(const BufferDesc&) const = default;
		auto operator<=>(const BufferDesc&)  const = default;

	private:
		SERIALIZE()
		{
			ar& NVP(SizeInBytes);
		}
	};

	struct ResourceDesc
	{
		std::variant<TextureDesc, BufferDesc> desc = BufferDesc{ 0 };
		ResFlags Flags;

		bool operator==(const ResourceDesc&) const = default;
		bool operator!=(const ResourceDesc&) const = default;
		auto operator<=>(const ResourceDesc&)  const = default;


		static ResourceDesc Buffer(size_t SizeInBytes, ResFlags Flags = ResFlags::ShaderResource)
		{
			return ResourceDesc{ BufferDesc{SizeInBytes}, Flags };
		}
		static ResourceDesc Tex1D(Format Format, uint Size, uint ArraySize, uint MipLevels, ResFlags Flags = ResFlags::ShaderResource)
		{
			return ResourceDesc{ TextureDesc{uint3(Size,0,0), ArraySize,MipLevels, Format, }, Flags };
		}
		static ResourceDesc Tex2D(Format Format, uint2 Size, uint ArraySize, uint MipLevels, ResFlags Flags = ResFlags::ShaderResource)
		{
			return ResourceDesc{ TextureDesc{uint3(Size,0), ArraySize,MipLevels, Format, }, Flags };
		}
		static ResourceDesc Tex3D(Format Format, uint3 Size, uint MipLevels, ResFlags Flags = ResFlags::ShaderResource)
		{
			return ResourceDesc{ TextureDesc{Size, 1,MipLevels, Format, }, Flags };
		}
		bool is_buffer() const
		{
			return std::holds_alternative<HAL::BufferDesc>(desc);
		}

		bool is_texture() const
		{
			return std::holds_alternative<HAL::TextureDesc>(desc);
		}

		const HAL::BufferDesc& as_buffer() const
		{
			return std::get<HAL::BufferDesc>(desc);
		}

		const HAL::TextureDesc& as_texture() const
		{
			return std::get<HAL::TextureDesc>(desc);
		}

		HAL::BufferDesc& as_buffer()
		{
			return std::get<HAL::BufferDesc>(desc);
		}

		HAL::TextureDesc& as_texture()
		{
			return std::get<HAL::TextureDesc>(desc);
		}
	private:
		SERIALIZE()
		{
			ar& NVP(desc);
			ar& NVP(Flags);
		}

	};


	struct ResourceAllocationInfo
	{
		size_t size;
		size_t alignment;
		HeapFlags flags;
	};


struct texture_layout
{
	uint64 size;
	uint rows_count;
	uint row_stride;
	uint slice_stride;

	uint alignment;

	Format format;
};


	struct shader_macro
	{
		std::string name;
		std::string value;
		shader_macro() = default;
		shader_macro(std::string name, std::string value = "1");

		bool operator<(const shader_macro& r)const
		{
			if(name==r.name)
				return value<r.value;
			return name<r.name;

		}
		bool operator==(const shader_macro& r)const
		{

			return name==r.name && value==r.value;

		}
	private:
		SERIALIZE()
		{
			ar& NVP(name)& NVP(value);
		}
	};

		enum class ShaderOptions:uint
	{
		None = 0,
		FP16 = 1
	};

	struct shader_header
	{
		std::string file_name;
		std::string entry_point;
		ShaderOptions flags;
		std::vector<shader_macro> macros;
		bool contains_text = false;

		GEN_DEF_COMP(shader_header)
	private:
		SERIALIZE()
		{
			ar& NVP(file_name);
			ar& NVP(entry_point);
			ar& NVP(flags);
			ar& NVP(macros);
			ar& NVP(contains_text);
		}
	};

	class shader_include
	{
		std::string dir;
		resource_file_depender& depender;

	public:
		std::vector<std::string> autogen;

		shader_include(std::string dir, resource_file_depender& _depender);
		std::unique_ptr<std::string> load_file(std::string filename);
	};


	struct ResourceAddress
	{
		Resource* resource = nullptr;
		uint64 resource_offset = 0;

		explicit operator bool() const
		{
			return resource;
		}

		ResourceAddress offset(uint64 offset) const
		{
			return { resource, resource_offset + offset };
		}

		std::byte* get_cpu_data() const;
	};


	enum class QueryType:uint
	{
		Timestamp,
		Statistics
	};

	///////////////////////////////////////////////////
	///

	enum class GeometryType :uint
	{
TRIANGLES,
PROCEDURAL
	};
#undef OPAQUE
	enum class GeometryFlags :uint
	{
		NONE,
		OPAQUE
	};

	struct GeometryDesc
	{
		GeometryType Type;
		GeometryFlags Flags;

		HAL::ResourceAddress Transform3x4;
		Format IndexFormat;
		Format VertexFormat;
		uint IndexCount;
		uint VertexCount;
		HAL::ResourceAddress IndexBuffer;

		HAL::ResourceAddress VertexBuffer;
		uint64 VertexStrideInBytes;
	};
	struct InstanceDesc
	{
		float Transform[3][4];
		uint InstanceID : 24;
		uint InstanceMask : 8;
		uint InstanceContributionToHitGroupIndex : 24;
		uint Flags : 8;
		HAL::ResourceAddress AccelerationStructure;
	};


	enum class RaytracingBuildFlags:uint
	{
		NONE = 0,
		ALLOW_UPDATE = 0x1,
		ALLOW_COMPACTION = 0x2,
		PREFER_FAST_TRACE = 0x4,
		PREFER_FAST_BUILD = 0x8,
		MINIMIZE_MEMORY = 0x10,
		PERFORM_UPDATE = 0x20
	};

	struct RaytracingBuildDescBottomInputs
	{
		RaytracingBuildFlags Flags;
		std::vector<GeometryDesc> geometry;

	};

	struct RaytracingBuildDescTopInputs
	{
		RaytracingBuildFlags Flags;
		uint NumDescs;
		HAL::ResourceAddress instances;
	};

	struct RaytracingBuildDescStructure
	{
		HAL::ResourceAddress DestAccelerationStructureData;
		HAL::ResourceAddress SourceAccelerationStructureData;
		HAL::ResourceAddress ScratchAccelerationStructureData;
	};
	struct RaytracingPrebuildInfo
	{
		uint64 ResultDataMaxSizeInBytes;
		uint64 ScratchDataSizeInBytes;
		uint64 UpdateScratchDataSizeInBytes;
	};

	struct hwnd_provider
	{
		virtual	HWND get_hwnd() const = 0;
	};

	struct swap_chain_desc
	{
		int max_fps;
		Format format;
		hwnd_provider* window;
		hwnd_provider* fullscreen;
		bool stereo;
	};

	enum class PrimitiveTopologyType:uint
	{
		UNDEFINED,
		POINT,
		LINE,
		TRIANGLE,
		PATCH
	};

	enum class PrimitiveTopologyFeed :uint
	{
		LIST,
		STRIP
	};
	using shader_identifier = std::array<std::byte, 32/*D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES*/>;

	enum class HitGroupType :uint
	{
		TRIANGLES,
		PROCEDURAL
	};
}
