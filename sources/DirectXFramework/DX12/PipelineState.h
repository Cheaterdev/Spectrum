#pragma once
#include <compare>


bool operator==(const D3D12_DEPTH_STENCILOP_DESC& l, const D3D12_DEPTH_STENCILOP_DESC& r);
std::strong_ordering operator<=>(const D3D12_DEPTH_STENCILOP_DESC& l, const D3D12_DEPTH_STENCILOP_DESC& r);


namespace DX12
{

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
	};
	struct input_layout_header
	{
		std::vector<input_layout_row> inputs;

		bool operator==(const input_layout_header&) const = default;
		auto   operator<=>(const  input_layout_header& r)  const = default;

	};


	struct RasterizerState
	{
		D3D12_CULL_MODE cull_mode;
		D3D12_FILL_MODE fill_mode;
		bool conservative = false;
		bool operator==(const RasterizerState&) const = default;
		auto   operator<=>(const  RasterizerState& r)  const = default;
	};

	struct RenderTarget
	{
		bool enabled = true;
		D3D12_BLEND dest = D3D12_BLEND::D3D12_BLEND_INV_SRC_ALPHA;
		D3D12_BLEND source = D3D12_BLEND::D3D12_BLEND_SRC_ALPHA;
		D3D12_BLEND dest_alpha = D3D12_BLEND::D3D12_BLEND_ZERO;
		D3D12_BLEND source_alpha = D3D12_BLEND::D3D12_BLEND_ONE;
			bool operator==(const RenderTarget&) const = default;
			auto  operator<=>(const  RenderTarget & r)  const = default;
	};

	struct BlendState
	{
		bool independ_blend = false;

		std::array<RenderTarget, 8> render_target;
		bool operator==(const BlendState&) const = default;
		auto operator<=>(const  BlendState& r)  const = default;
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

	};

	struct PipelineStateDesc
	{
		input_layout_header layout;
		RootSignature::ptr root_signature;
		vertex_shader::ptr vertex;
		pixel_shader::ptr pixel;
		geometry_shader::ptr geometry;
		hull_shader::ptr hull;
		domain_shader::ptr domain;
		D3D12_PRIMITIVE_TOPOLOGY_TYPE topology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

		BlendState blend;
		RasterizerState rasterizer;
		RTVState rtv;
		PipelineStateDesc()
		{
			rtv.rtv_formats.emplace_back(DXGI_FORMAT_R8G8B8A8_UNORM);
			rasterizer.cull_mode = D3D12_CULL_MODE::D3D12_CULL_MODE_BACK;
			rasterizer.fill_mode = D3D12_FILL_MODE::D3D12_FILL_MODE_SOLID;
		}

		bool operator==(const PipelineStateDesc& r) const;
		std::strong_ordering  operator<=>(const  PipelineStateDesc& r)  const ;

	};
	

	template<class K0, class K1, class...Ks>
	struct my_map;
	template<class K0, class K1, class...Ks>
	using my_map_t = typename my_map<K0, K1, Ks...>::type;


	template<class K0, class K1>
	struct my_map<K0, K1> { using type = std::map<K0, K1>; };

	template<class K0, class K1, class K2, class...Ks>
	struct my_map<K0, K1, K2, Ks...>
	{
		using type2 = my_map_t<K1, K2, Ks...>;
		using type = std::map<K0, type2>;
	};





	class PipelineStateBase
	{

	protected:
		ComPtr<ID3D12PipelineState> m_pipelineState;
		virtual ~PipelineStateBase() {};

		template<class T>
		void register_shader(T shader)
		{
			if (shader)
				shader->pipelines.insert(this);
		}

		template<class T>
		void unregister_shader(T shader)
		{
			if (shader)
				shader->pipelines.erase(this);
		}
	public:
		virtual	void on_change() = 0;
		ComPtr<ID3D12PipelineState> get_native();


	};

	class PipelineState : public PipelineStateBase
	{

	public:
		using ptr = s_ptr<PipelineState>;
		const  PipelineStateDesc desc;
		void on_change() override;

		PipelineState(PipelineStateDesc _desc);

		virtual ~PipelineState();

	};

	template<class T>
	class PipelineStateTyped :public PipelineState
	{
	public:
		using PipelineState::PipelineState;
		using ptr = s_ptr<PipelineStateTyped<T>>;

	};

	class PipelineStateCache : public Singleton<PipelineStateCache>
	{
		//my_map<RootSignature::ptr, vertex_shader::ptr, pixel_shader::ptr, geometry_shader::ptr, hull_shader::ptr, domain_shader::ptr, DXGI_FORMAT, PipelineState::ptr>::type cache;
		Cache<PipelineStateDesc, PipelineState::ptr> cache;
		friend class Singleton<PipelineStateCache>;

		virtual ~PipelineStateCache()
		{
			//    cache.clear();
		}
		PipelineStateCache() : cache([](const PipelineStateDesc& desc)
			{
				return std::make_shared<PipelineState>(desc);
			})
		{
		}
	public:

		static PipelineState::ptr get_cache(PipelineStateDesc& desc);

	};


	class ComputePipelineState;


	struct ComputePipelineStateDesc
	{
		RootSignature::ptr root_signature;

		compute_shader::ptr shader;

		//   ComputePipelineStateDesc() = default;

		std::shared_ptr<ComputePipelineState> create();

	};



	class ComputePipelineState : public PipelineStateBase
	{
		void on_change() override;
	public:
		using ptr = s_ptr<ComputePipelineState>;
		const ComputePipelineStateDesc desc;


		explicit ComputePipelineState(const ComputePipelineStateDesc& _desc) : desc(_desc)
		{
			on_change();
			register_shader(desc.shader);
		}


		virtual ~ComputePipelineState();
	};
}


