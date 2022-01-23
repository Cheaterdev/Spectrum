export module HAL.Types;

import Vectors;
import Utils;
import serialization;

import HAL.Format;

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

	enum class StencilOp: char
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


	struct RTVState
	{
		bool enable_depth = false;
		bool enable_depth_write = true;


		Format ds_format = Formats::Unknown;
		std::vector<Format> rtv_formats;
		ComparisonFunc func = ComparisonFunc::LESS;

		bool enable_stencil = false;
		uint8 stencil_read_mask = 0xff;
		uint8 stencil_write_mask = 0xff;
		StencilDesc stencil_desc;

		RTVState()
		{
			stencil_desc.StencilDepthFailOp = StencilOp::Keep;
			stencil_desc.StencilFailOp = StencilOp::Keep;
			stencil_desc.StencilPassOp = StencilOp::Keep;
			stencil_desc.StencilFunc = ComparisonFunc::LESS;
		}
		bool operator==(const RTVState& r) const = default;
		auto  operator<=>(const  RTVState& r)  const = default;
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



	enum class ResourceType : char
	{

		BUFFER,
		TEXTURE1D,
		//	TEXTURE1DARRAY,
		TEXTURE2D,
		//TEXTURE2DARRAY,
		TEXTURE3D,
		CUBE
	};



	struct ResourceViewDesc
	{
		ResourceType type;
		Format format;

		union
		{
			struct
			{

				uint PlaneSlice;
				uint MipSlice;
				uint FirstArraySlice;
				uint MipLevels;
				uint ArraySize;
			} Texture2D;

			struct
			{
				uint64 Size;
				uint64 Offset;
				uint64 Stride;
				bool counted;
			} Buffer;

		};
	};



}
