export module HAL:Types;

import Core;

import :Format;
import windows;

#undef DOMAIN

export namespace HAL
{

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




	enum class CullMode : char
	{
		None,
		Back,
		Front,

		GENERATE_OPS
	};

	enum class FillMode : char
	{
		Wireframe,
		Solid,

		GENERATE_OPS
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




	enum class ResFlags :uint
	{
		None = 0,
		ShaderResource = 1 << 0,
		UnorderedAccess = 1 << 1,
		RenderTarget = 1 << 2,
		DepthStencil = 1 << 3,
		CrossAdapter = 1 << 4,

		GENERATE_OPS
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




	struct shader_macro
	{
		std::string name;
		std::string value;
		shader_macro() = default;
		shader_macro(std::string name, std::string value = "1");

		GEN_DEF_COMP(shader_macro)
	private:
		SERIALIZE()
		{
			ar& NVP(name)& NVP(value);
		}
	};

	struct shader_header
	{
		std::string file_name;
		std::string entry_point;
		uint flags;
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

		ResourceAddress offset(uint offset) const
		{
			return { resource, resource_offset + offset };
		}

		std::byte* get_cpu_data() const;
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
		PERFORM_UPDATE = 0x20,
		GENERATE_OPS
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
