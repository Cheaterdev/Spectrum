module;


export module d3d12_types;

export import d3d12;
import Utils;
import serialization;

export
{


	bool operator==(const D3D12_DEPTH_STENCILOP_DESC& l, const D3D12_DEPTH_STENCILOP_DESC& r);
	std::strong_ordering operator<=>(const D3D12_DEPTH_STENCILOP_DESC& l, const D3D12_DEPTH_STENCILOP_DESC& r);
	namespace DX12
	{


		enum class ResourceState : UINT
		{
			COMMON = D3D12_RESOURCE_STATE_COMMON,
			VERTEX_AND_CONSTANT_BUFFER = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
			INDEX_BUFFER = D3D12_RESOURCE_STATE_INDEX_BUFFER,
			RENDER_TARGET = D3D12_RESOURCE_STATE_RENDER_TARGET,
			UNORDERED_ACCESS = D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
			DEPTH_WRITE = D3D12_RESOURCE_STATE_DEPTH_WRITE,
			DEPTH_READ = D3D12_RESOURCE_STATE_DEPTH_READ,
			NON_PIXEL_SHADER_RESOURCE = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
			PIXEL_SHADER_RESOURCE = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			STREAM_OUT = D3D12_RESOURCE_STATE_STREAM_OUT,
			INDIRECT_ARGUMENT = D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT,
			COPY_DEST = D3D12_RESOURCE_STATE_COPY_DEST,
			COPY_SOURCE = D3D12_RESOURCE_STATE_COPY_SOURCE,
			RESOLVE_DEST = D3D12_RESOURCE_STATE_RESOLVE_DEST,
			RESOLVE_SOURCE = D3D12_RESOURCE_STATE_RESOLVE_SOURCE,
			GEN_READ = D3D12_RESOURCE_STATE_GENERIC_READ,
			PRESENT = D3D12_RESOURCE_STATE_PRESENT,
			PREDICATION = D3D12_RESOURCE_STATE_PREDICATION,
			RAYTRACING_STRUCTURE = D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
			UNKNOWN = UINT_MAX - 1,

			GENERATE_OPS
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

		enum class HeapType : int
		{
			DEFAULT = D3D12_HEAP_TYPE_DEFAULT,
			UPLOAD = D3D12_HEAP_TYPE_UPLOAD,
			READBACK = D3D12_HEAP_TYPE_READBACK,
			CUSTOM = D3D12_HEAP_TYPE_CUSTOM,
			RESERVED = 10,
			PRESENT = 5
		};


		enum class CommandListType : int
		{
			DIRECT = D3D12_COMMAND_LIST_TYPE_DIRECT,
			BUNDLE = D3D12_COMMAND_LIST_TYPE_BUNDLE,
			COMPUTE = D3D12_COMMAND_LIST_TYPE_COMPUTE,
			COPY = D3D12_COMMAND_LIST_TYPE_COPY,

			GENERATE_OPS
		};



		enum class HandleType : char
		{
			CBV,
			RTV,
			DSV,
			SRV,
			UAV,
			SMP
		};

		enum class DescriptorHeapType : char
		{
			CBV_SRV_UAV = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
			SAMPLER = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
			RTV = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
			DSV = D3D12_DESCRIPTOR_HEAP_TYPE_DSV
			, GENERATE_OPS
		};
		enum class DescriptorHeapFlags : char
		{
			NONE = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
			SHADER_VISIBLE = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE

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
			DXGI_FORMAT format;

			union
			{
				struct
				{

					UINT PlaneSlice;
					UINT MipSlice;
					UINT FirstArraySlice;
					UINT MipLevels;
					UINT ArraySize;
				} Texture2D;

				struct
				{
					UINT64 Size;
					UINT64 Offset;
					UINT64 Stride;
					bool counted;
				} Buffer;

			};
		};


		struct input_layout_row
		{
			std::string SemanticName;
			UINT SemanticIndex;
			DXGI_FORMAT Format;
			UINT InputSlot;
			UINT AlignedByteOffset;
			D3D12_INPUT_CLASSIFICATION InputSlotClass;
			UINT InstanceDataStepRate;

			D3D12_INPUT_ELEMENT_DESC create_native() const
			{
				D3D12_INPUT_ELEMENT_DESC res;

				res.SemanticName = SemanticName.c_str();
				res.SemanticIndex = SemanticIndex;
				res.Format = Format;
				res.InputSlot = InputSlot;
				res.AlignedByteOffset = AlignedByteOffset;
				res.InputSlotClass = InputSlotClass;
				res.InstanceDataStepRate = InstanceDataStepRate;

				return res;
			}
			bool operator==(const input_layout_row&) const = default;
			auto   operator<=>(const  input_layout_row& r)  const = default;

		private:
			SERIALIZE()
			{
				ar& NVP(SemanticName);
				ar& NVP(SemanticIndex);
				ar& NVP(Format);
				ar& NVP(InputSlot);
				ar& NVP(AlignedByteOffset);
				ar& NVP(InputSlotClass);
				ar& NVP(InstanceDataStepRate);

			}


		};
		struct input_layout_header
		{
			std::vector<input_layout_row> inputs;

//#define INVOKE_ICE

#ifdef INVOKE_ICE
			bool operator==(const input_layout_header& r) const = default;
#else
			bool operator==(const input_layout_header& r) const
			{
				return inputs == r.inputs;
			}
#endif

			auto   operator<=>(const  input_layout_header& r)  const = default;
		private:
			SERIALIZE()
			{
				ar& NVP(inputs);

			}

		};


		struct RasterizerState
		{
			D3D12_CULL_MODE cull_mode;
			D3D12_FILL_MODE fill_mode;
			bool conservative = false;
			bool operator==(const RasterizerState&) const = default;
			auto   operator<=>(const  RasterizerState& r)  const = default;

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
			D3D12_BLEND dest = D3D12_BLEND::D3D12_BLEND_INV_SRC_ALPHA;
			D3D12_BLEND source = D3D12_BLEND::D3D12_BLEND_SRC_ALPHA;
			D3D12_BLEND dest_alpha = D3D12_BLEND::D3D12_BLEND_ZERO;
			D3D12_BLEND source_alpha = D3D12_BLEND::D3D12_BLEND_ONE;
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
				for (auto& r : render_target)
					ar& NVP(r);

			}


		};


		struct RTVState
		{
			bool enable_depth = false;
			bool enable_depth_write = true;


			DXGI_FORMAT ds_format = DXGI_FORMAT_UNKNOWN;
			std::vector<DXGI_FORMAT> rtv_formats;
			D3D12_COMPARISON_FUNC func = D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_LESS;

			bool enable_stencil = false;
			UINT8 stencil_read_mask = 0xff;
			UINT8 stencil_write_mask = 0xff;
			D3D12_DEPTH_STENCILOP_DESC stencil_desc;

			RTVState()
			{
				stencil_desc.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
				stencil_desc.StencilFailOp = D3D12_STENCIL_OP_KEEP;
				stencil_desc.StencilPassOp = D3D12_STENCIL_OP_KEEP;

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

	}

}


module:private;


#define E(x) 	if ( l.x != r.x) return false;
#define C(x) 	if (auto cmp = l.x <=> r.x; cmp != 0) return cmp;

bool operator==(const D3D12_DEPTH_STENCILOP_DESC& l, const D3D12_DEPTH_STENCILOP_DESC& r)
{
	E(StencilDepthFailOp)
		E(StencilFailOp)
		E(StencilFunc)
		E(StencilPassOp)


		return true;
}


std::strong_ordering operator<=>(const D3D12_DEPTH_STENCILOP_DESC& l, const D3D12_DEPTH_STENCILOP_DESC& r)
{
	C(StencilDepthFailOp)
		C(StencilFailOp)
		C(StencilFunc)
		C(StencilPassOp)


		return std::strong_ordering::equal;
}
