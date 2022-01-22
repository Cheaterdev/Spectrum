module;


export module d3d12_types;

export import d3d12;
import Utils;
import serialization;

export
{


	bool operator==(const D3D12_DEPTH_STENCILOP_DESC& l, const D3D12_DEPTH_STENCILOP_DESC& r);
	std::strong_ordering operator<=>(const D3D12_DEPTH_STENCILOP_DESC& l, const D3D12_DEPTH_STENCILOP_DESC& r);
	namespace Graphics
	{





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
	E(StencilDepthFailOp);
	E(StencilFailOp);
	E(StencilFunc);
	E(StencilPassOp);

	return true;
}

std::strong_ordering operator<=>(const D3D12_DEPTH_STENCILOP_DESC& l, const D3D12_DEPTH_STENCILOP_DESC& r)
{
	C(StencilDepthFailOp);
	C(StencilFailOp);
	C(StencilFunc);
	C(StencilPassOp);

	return std::strong_ordering::equal;
}
