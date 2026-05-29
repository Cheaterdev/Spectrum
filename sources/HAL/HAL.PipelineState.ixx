export module HAL:PipelineState;

import Core;
import :Types;
import :Device;
import :RootSignature;
import :Shader;
import :API.PipelineState;

export namespace HAL
{
	struct PipelineStateDesc
	{
		std::string name;
		Layouts layout;
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
		PipelineStateDesc();

		bool is_memory();

		bool operator==(const PipelineStateDesc& r) const = default;

		auto  operator<=>(const  PipelineStateDesc& r)  const = default;
	private:

		SERIALIZE()
		{
			ar& NVP(name);
			ar& NVP(layout);

			ar& NVP(topology);
			ar& NVP(blend);
			ar& NVP(rasterizer);
			ar& NVP(rtv);

		
			ar& NVP(pixel);
			ar& NVP(vertex);
			ar& NVP(geometry);
			ar& NVP(hull);
			ar& NVP(domain);

			ar& NVP(mesh);
			ar& NVP(amplification);



		}
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
		UsedSlots slots;
		bool debuggable = false;
		std::string name;
		RootSignature::ptr root_signature;
		virtual	void on_change() = 0;
	};

	class PipelineState : public PipelineStateBase, public TypedObject<PipelineState>
	{
		friend class PipelineStateCache;

		PipelineState(Device& device, PipelineStateDesc _desc, std::string cache);

	public:
		PipelineState() = default;
		using ptr = s_ptr<PipelineState>;
		const  PipelineStateDesc desc;
		void on_change() override;


		static ptr create(Device& device, PipelineStateDesc& desc, std::string name);
	};







	struct ComputePipelineStateDesc
	{
		Layouts layout;

		compute_shader::ptr shader;

		std::string name;
		bool operator==(const ComputePipelineStateDesc& r) const = default;
		std::strong_ordering  operator<=>(const  ComputePipelineStateDesc& r)  const = default;

	private:
		SERIALIZE()
		{
			ar& NVP(name);
			ar& NVP(layout);
			ar& NVP(shader);
		}
	};



	class ComputePipelineState : public PipelineStateBase
	{
		void on_change() override;

		friend class PipelineStateCache;
		explicit ComputePipelineState(Device& device, const ComputePipelineStateDesc& _desc, std::string cache);

	public:
		using ptr = s_ptr<ComputePipelineState>;
		const ComputePipelineStateDesc desc;




		static ptr create(Device& device, ComputePipelineStateDesc& desc, std::string name);

	};



	class PipelineStateCache
	{
		Cache<PipelineStateDesc, PipelineState::ptr> cache;
		Cache<ComputePipelineStateDesc, ComputePipelineState::ptr> compute_cache;

		std::mutex m;
		std::map<std::string, std::string> binary_cache;

		Device& device;

	public:
		PipelineStateCache(Device& device);
		virtual ~PipelineStateCache();

		PipelineState::ptr get_cache(PipelineStateDesc& desc, std::string name = "");
		ComputePipelineState::ptr get_cache(ComputePipelineStateDesc& desc, std::string name = "");

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

		void export_shader(std::wstring name, std::wstring as = L"");

	};

	class StateObject;

	enum class StateObjectType:uint
	{
		Collection,
		Raytracing,
		WorkGraph
	};
	struct StateObjectDesc
	{

		StateObjectType type = StateObjectType::Raytracing;

		RootSignature::ptr global_root;

		std::list<LibraryObject> libraries;
		std::list<HitGroup> hit_groups;

		std::list<std::shared_ptr<StateObject>> collections;

		UINT MaxTraceRecursionDepth = 0;
		UINT MaxPayloadSizeInBytes = 0;
		UINT MaxAttributeSizeInBytes = 0;

	};

	class StateObject :public PipelineStateBase, public  API::StateObject, public TypedObject<StateObject>
	{


		void on_change() override;

		static HAL::shader_identifier identify(void* data);


	public:

		const StateObjectDesc desc;
		using ptr = std::shared_ptr<StateObject>;

		Events::Event<> event_change;
		StateObject(StateObjectDesc& desc);

		virtual ~StateObject() = default;
		HAL::shader_identifier get_shader_id(std::wstring_view name);
	};

	template<class T>
	class Wrapper
	{
		std::shared_ptr<T>& pso;
	public:
		Wrapper(std::shared_ptr<T>& pso) :pso(pso)
		{

		}
		SERIALIZE()
		{
			IF_LOAD()
			{
				HAL::PipelineStateDesc desc;
				ar& NVP(desc);
				Device* _dev = nullptr;
				if constexpr (requires { cereal::get_user_data<UniversalContext>(ar); })
					_dev = cereal::get_user_data<UniversalContext>(ar).template get_context<Device*>();
			//	if (!_dev) _dev = &Device::get();
				pso = HAL::PipelineState::create(*_dev, desc, desc.name);

			}
			else
			{
			auto& desc = pso->desc;
			ar& NVP(desc);
			}
		}
	};

	template<class K, class T>
	class WrapperMap
	{
		std::map<K, T>& pso;
	public:
		WrapperMap(std::map<K, T>& pso) :pso(pso)
		{

		}
		SERIALIZE()
		{
			IF_LOAD()
			{
				Device* _dev = nullptr;
				if constexpr (requires { cereal::get_user_data<UniversalContext>(ar); })
					_dev = cereal::get_user_data<UniversalContext>(ar).template get_context<Device*>();

				uint size;
				ar& NVP(size);
				for (uint i = 0; i < size; i++)
				{
					HAL::PipelineStateDesc desc;

					ar& NVP(desc);
					K k;

					ar& NVP(k);

					pso[k] = HAL::PipelineState::create(*_dev, desc, desc.name);
				}


			}
			else
			{
			uint size = static_cast<uint>(pso.size());
			ar& NVP(size);
			for (auto& [k, v] : pso)
			{
				ar& NVP(v->desc);
				ar& NVP(k);
			}
			}
		}
	};


	template<class T>
	auto wrap(std::shared_ptr<T>& ptr)
	{
		return Wrapper<T>(ptr);
	}


	template<class K, class T>
	auto wrap(std::map<K, T>& ptr)
	{
		return WrapperMap<K, T>(ptr);
	}
}

