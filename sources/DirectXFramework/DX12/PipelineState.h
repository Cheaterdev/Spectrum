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

	private:
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive& ar, const unsigned int)
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

		bool operator==(const input_layout_header&) const = default;
		auto   operator<=>(const  input_layout_header& r)  const = default;
	private:
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive& ar, const unsigned int)
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
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive& ar, const unsigned int)
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
			auto  operator<=>(const  RenderTarget & r)  const = default;



	private:
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive& ar, const unsigned int)
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
		auto operator<=>(const  BlendState& r)  const = default;


	private:
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive& ar, const unsigned int)
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
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive& ar, const unsigned int)
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

	struct PipelineStateDesc
	{
		std::string name;
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

		bool is_memory()
		{

			if (vertex && vertex->get_header().file_name.empty()) return true;
			if (pixel && pixel->get_header().file_name.empty()) return true;
			if (geometry && geometry->get_header().file_name.empty()) return true;
			if (hull && hull->get_header().file_name.empty()) return true;
			if (domain && domain->get_header().file_name.empty()) return true;
			return false;
		}

		bool operator==(const PipelineStateDesc& r) const;
		std::strong_ordering  operator<=>(const  PipelineStateDesc& r)  const ;
	private: 
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive& ar, const unsigned int)
		{
			ar& NVP(name);

			ar& NVP(topology);
			ar& NVP(blend);
			ar& NVP(rasterizer);
			ar& NVP(rtv);

			
			if constexpr (Archive::is_saving::value)
			{

				auto sig = dynamic_cast<RootLayout*>(root_signature.get());
				ar& NVP(sig->layout);
				auto save_header = [&](auto &shader) {
				
					bool has_header = !!shader;
					ar& NVP(has_header);

					if (has_header)
						ar& NVP(shader->get_header());
				};
			
				save_header(pixel);
				save_header(vertex);
				save_header(geometry);
				save_header(hull);
				save_header(domain);

			}
			else
			{
				Layouts l;

				ar& NVP(l);

				root_signature = get_Signature(l);
				auto load_header = [&]<class T>(std::shared_ptr<T>& shader) {

					bool has_header;
					ar& NVP(has_header);

					if (has_header)
					{
						D3D::shader_header header;
						ar& NVP(header);

						shader = T::get_resource(header);
					}
						
				};



				load_header(pixel);
				load_header(vertex);
				load_header(geometry);
				load_header(hull);
				load_header(domain);
			}
		}
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



	class PipelineLibrary :public Singleton<PipelineLibrary>
	{
		ComPtr<ID3D12PipelineLibrary> m_pipelineLibrary;
	public:
		PipelineLibrary()
		{
			auto file = FileSystem::get().get_file("pso");

			if (file)
			{
				auto data = file->load_all();
				TEST(Device::get().get_native_device()->CreatePipelineLibrary(data.data(), data.size(), IID_PPV_ARGS(&m_pipelineLibrary)));
			}
		
			if(!m_pipelineLibrary)
				TEST(Device::get().get_native_device()->CreatePipelineLibrary(nullptr, 0, IID_PPV_ARGS(&m_pipelineLibrary)));

		}


		virtual ~PipelineLibrary()
		{

			std::string data;

			data.resize(m_pipelineLibrary->GetSerializedSize());

			if (data.size())
			{

				TEST(m_pipelineLibrary->Serialize(data.data(), data.size()));

				FileSystem::get().save_data("pso", data);
			}

		}


		ComPtr<ID3D12PipelineState> create(std::string name,const D3D12_GRAPHICS_PIPELINE_STATE_DESC &desc)
		{
			ComPtr<ID3D12PipelineState> res;


			if (name.empty())
			{
				TEST(Device::get().get_native_device()->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&res)));
			}
			else
			{
				std::wstring wname = convert(name);
				HRESULT hr = m_pipelineLibrary->LoadGraphicsPipeline(wname.c_str(), &desc, IID_PPV_ARGS(&res));

				if (E_INVALIDARG == hr)
				{
					TEST(Device::get().get_native_device()->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&res)));
					TEST(m_pipelineLibrary->StorePipeline(wname.c_str(), res.Get()));
				}

			}
			
			return res;;
		}


	};

	class PipelineStateBase
	{

	protected:
		ComPtr<ID3D12PipelineState> m_pipelineState;
		std::string cache;
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

		std::string get_cache()
		{
			ComPtr<ID3DBlob> blob;
			m_pipelineState->GetCachedBlob(&blob);
			std::string str((char*)blob->GetBufferPointer(), blob->GetBufferSize());

			return str;
		}
	};

	class PipelineState : public PipelineStateBase
	{
		friend class PipelineStateCache;

		PipelineState(PipelineStateDesc _desc, std::string cache);
	
	public:
		PipelineState() = default;
		using ptr = s_ptr<PipelineState>;
		const  PipelineStateDesc desc;
		void on_change() override;

	
		static ptr create(PipelineStateDesc & desc, std::string name);

		virtual ~PipelineState();


	
	private:
		friend class boost::serialization::access;
		template<class Archive>
		void serialize(Archive& ar, const unsigned int)
		{
			if constexpr (Archive::is_saving::value)
			{
				ComPtr<ID3DBlob> blob;
				m_pipelineState->GetCachedBlob(&blob);
				std::string str((char*)blob->GetBufferPointer(), blob->GetBufferSize());


				ar& NVP(str);
			}

			else
			{
				std::string str;
				ar& NVP(str);
			}
		}
	};

	template<class T>
	class PipelineStateTyped :public PipelineState
	{
	public:
		using PipelineState::PipelineState;
		using ptr = s_ptr<PipelineStateTyped<T>>;

	};



	class ComputePipelineState;


	struct ComputePipelineStateDesc
	{
		RootSignature::ptr root_signature;

		compute_shader::ptr shader;

		std::string name;
		bool operator==(const ComputePipelineStateDesc& r) const = default;
		std::strong_ordering  operator<=>(const  ComputePipelineStateDesc& r)  const = default;
	};



	class ComputePipelineState : public PipelineStateBase
	{
		void on_change() override;

		friend class PipelineStateCache;
		explicit ComputePipelineState(const ComputePipelineStateDesc& _desc, std::string cache) : desc(_desc)
		{

			this->cache = cache;
			on_change();
			register_shader(desc.shader);
		}

	public:
		using ptr = s_ptr<ComputePipelineState>;
		const ComputePipelineStateDesc desc;


	

		static ptr create(ComputePipelineStateDesc& desc, std::string name);
		virtual ~ComputePipelineState();
	};



	class PipelineStateCache : public Singleton<PipelineStateCache>
	{
		//my_map<RootSignature::ptr, vertex_shader::ptr, pixel_shader::ptr, geometry_shader::ptr, hull_shader::ptr, domain_shader::ptr, DXGI_FORMAT, PipelineState::ptr>::type cache;
		Cache<PipelineStateDesc, PipelineState::ptr> cache;
		Cache<ComputePipelineStateDesc, ComputePipelineState::ptr> compute_cache;

		std::map<std::string, std::string> binary_cache;

		friend class Singleton<PipelineStateCache>;

		virtual ~PipelineStateCache()
		{
			FileSystem::get().save_data("pso",Serializer::serialize(binary_cache));
		}
		PipelineStateCache() : cache([this](const PipelineStateDesc& desc)
			{
				std::string binary = desc.name.empty()?"": binary_cache[desc.name];


				//Log::get() << desc << Log::endl;
				auto state=  PipelineState::ptr(new PipelineState(desc, binary));

				if (!desc.name.empty() && binary.empty())
				{
					binary_cache[desc.name] = state->get_cache();
				}

				return state;

			}), compute_cache([this](const ComputePipelineStateDesc& desc)
				{
					std::string binary = desc.name.empty() ? "" : binary_cache[desc.name];


					//Log::get() << desc << Log::endl;
					auto state = ComputePipelineState::ptr(new ComputePipelineState(desc,binary));

					if (!desc.name.empty() &&binary.empty())
					{
						binary_cache[desc.name] = state->get_cache();
					}

					return state;

				})
		{

			auto file = FileSystem::get().get_file("pso");
			if(file)
			Serializer::deserialize(file->load_all(), binary_cache);
			
		}
	public:

		static PipelineState::ptr get_cache(PipelineStateDesc& desc, std::string name ="");
		static ComputePipelineState::ptr get_cache(ComputePipelineStateDesc& desc, std::string name = "");

	};

}




BOOST_CLASS_EXPORT_KEY(DX12::PipelineStateBase);
BOOST_CLASS_EXPORT_KEY(DX12::PipelineState);