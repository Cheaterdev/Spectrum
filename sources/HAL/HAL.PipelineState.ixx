export module HAL:PipelineState;

import FileSystem;
import Singleton;
import serialization;
import Trackable;
import :Types;
import :RootSignature;
import :Shader;
import :API.PipelineState;

export namespace HAL
{
	struct PipelineStateDesc
	{
		std::string name;
		RootSignature::ptr root_signature;
		vertex_shader::ptr vertex;
		pixel_shader::ptr pixel;
		geometry_shader::ptr geometry;
		hull_shader::ptr hull;
		domain_shader::ptr domain;

		mesh_shader::ptr mesh;
		amplification_shader::ptr amplification;

		HAL::PrimitiveTopologyType topology;

		BlendState blend;
		RasterizerState rasterizer;
		RenderTargetState rtv;
		PipelineStateDesc()
		{
			rtv.rtv_formats.emplace_back(Format::R8G8B8A8_UNORM);
			rasterizer.cull_mode = CullMode::Back;
			rasterizer.fill_mode = FillMode::Solid;

			topology = HAL::PrimitiveTopologyType::TRIANGLE;
		}

		bool is_memory()
		{

			if (vertex && vertex->get_header().file_name.empty()) return true;
			if (pixel && pixel->get_header().file_name.empty()) return true;
			if (geometry && geometry->get_header().file_name.empty()) return true;
			if (hull && hull->get_header().file_name.empty()) return true;
			if (domain && domain->get_header().file_name.empty()) return true;

			if (mesh && mesh->get_header().file_name.empty()) return true;
			if (amplification && amplification->get_header().file_name.empty()) return true;


			return false;
		}

		bool operator==(const PipelineStateDesc& r) const = default;

		auto  operator<=>(const  PipelineStateDesc& r)  const = default;
	private:
		/*	SERIALIZE_PRETTY()
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
					auto save_header = [&](auto& shader) {

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

					save_header(mesh);
					save_header(amplification);

				}
				else
				{
					Layouts l;

					ar& NVP(l);

					root_signature = get_Signature(l);
					auto load_header = [&]<class T>(std::shared_ptr<T>&shader) {

						using Type = decltype(*shader.get());
						bool has_header;
						ar& NVP(has_header);

						if (has_header)
						{
							HAL::shader_header header;
							ar& NVP(header);

							shader = Type::get_resource(header);
						}

					};



					load_header(pixel);
					load_header(vertex);
					load_header(geometry);
					load_header(hull);
					load_header(domain);

					load_header(mesh);
					load_header(amplification);
				}
			}*/
	};

	class PipelineStateBase : public Trackable<API::TrackedPipeline>, public virtual Events::prop_handler, public API::PipelineStateBase
	{
		friend class API::PipelineStateBase;
	protected:

		std::string cache;

		virtual ~PipelineStateBase() {};

		template<class T>
		void register_shader(T shader)
		{
			if (shader) shader->on_change.register_handler(this, [this]() { on_change(); });
		}

	public:
		bool debuggable = false;
		std::string name;
		RootSignature::ptr root_signature;
		virtual	void on_change() = 0;
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


		static ptr create(PipelineStateDesc& desc, std::string name);


		/*private:
			SERIALIZE()
			{
				if constexpr (Archive::is_saving::value)
				{
					ComPtr<ID3DBlob> blob;
					tracked_info->m_pipelineState->GetCachedBlob(&blob);
					std::string str((char*)blob->GetBufferPointer(), blob->GetBufferSize());


					ar& NVP(str);
				}

				else
				{
					std::string str;
					ar& NVP(str);
				}
			}*/
	};



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

	};



	class PipelineStateCache : public Singleton<PipelineStateCache>
	{
		Cache<PipelineStateDesc, PipelineState::ptr> cache;
		Cache<ComputePipelineStateDesc, ComputePipelineState::ptr> compute_cache;

		std::mutex m;
		std::map<std::string, std::string> binary_cache;

		friend class Singleton<PipelineStateCache>;

		virtual ~PipelineStateCache();

		PipelineStateCache();
	public:

		static PipelineState::ptr get_cache(PipelineStateDesc& desc, std::string name = "");
		static ComputePipelineState::ptr get_cache(ComputePipelineStateDesc& desc, std::string name = "");

	};






	struct HitGroup
	{
		std::wstring name;
		RootSignature::ptr local_root;

		HAL::HitGroupType type;
		std::wstring any_hit_shader;
		std::wstring intersection_shader;
		std::wstring closest_hit_shader;

	};


	struct LibraryObject
	{
		library_shader::ptr library;
		std::map<std::wstring, std::wstring> exports;

		void export_shader(std::wstring name, std::wstring as = L"")
		{
			exports[name] = as;
		}

	};

	class StateObject;
	struct StateObjectDesc
	{

		bool collection = false;

		RootSignature::ptr global_root;

		std::list<LibraryObject> libraries;
		std::list<HitGroup> hit_groups;

		std::list<std::shared_ptr<StateObject>> collections;

		UINT MaxTraceRecursionDepth = 0;
		UINT MaxPayloadSizeInBytes = 0;
		UINT MaxAttributeSizeInBytes = 0;

	};

	class StateObject :public PipelineStateBase, public  API::StateObject
	{


		void on_change() override;

		static HAL::shader_identifier identify(void* data)
		{
			HAL::shader_identifier result;

			memcpy(result.data(), data, result.size());

			return result;
		}


	public:

		const StateObjectDesc desc;
		using ptr = std::shared_ptr<StateObject>;

		Events::Event<void> event_change;
		StateObject(StateObjectDesc& desc) :desc(desc)
		{

			for (auto& l : desc.libraries)
			{
				register_shader(l.library);
			}

			for (auto& c : desc.collections)
			{
				c->event_change.register_handler(this, [this]()
					{
						on_change();
					});
			}

			on_change();

		}

		virtual ~StateObject() = default;
		HAL::shader_identifier get_shader_id(std::wstring_view name);
	};

}