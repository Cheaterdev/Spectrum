module;
#include "dx12_types.h"
#include "pch_dx.h"
export module PipelineState;

import FileSystem;
import D3D.Shaders;

import RootSignature;

import Shader;
import Device;

import Singleton;
import serialization;
import Trackable;
Graphics::RootLayout::ptr get_Signature(Layouts id);

import Graphics.Types;
export
{


namespace Graphics
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

		D3D12_PRIMITIVE_TOPOLOGY_TYPE topology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

		BlendState blend;
		RasterizerState rasterizer;
		RTVState rtv;
		PipelineStateDesc()
		{
			rtv.rtv_formats.emplace_back(DXGI_FORMAT_R8G8B8A8_UNORM);
			rasterizer.cull_mode = CullMode::Back;
			rasterizer.fill_mode = FillMode::Solid;
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
		SERIALIZE()
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
						D3D::shader_header header;
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
		}
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

	class TrackedPipeline : public TrackedObject
	{
	public:
		ComPtr<ID3D12PipelineState> m_pipelineState;
		ComPtr<ID3D12StateObject> m_StateObject;
	};
	class PipelineStateBase: public Trackable<TrackedPipeline>, public virtual Events::prop_handler
	{
		
	protected:
		
		std::string cache;

		virtual ~PipelineStateBase() {};

		template<class T>
		void register_shader(T shader)
		{
			if(shader) shader->on_change.register_handler(this, [this]() { on_change(); });
		}


	public:
		bool debuggable = false;
		std::string name;
		RootSignature::ptr root_signature;
		//UsedSlots slots;

		virtual	void on_change() = 0;
		ComPtr<ID3D12PipelineState> get_native();
		ComPtr<ID3D12StateObject> get_native_state();

		std::string get_cache()
		{
			ComPtr<ID3DBlob> blob;
			tracked_info->m_pipelineState->GetCachedBlob(&blob);
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
		}
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
		virtual ~ComputePipelineState();
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

		static PipelineState::ptr get_cache(PipelineStateDesc& desc, std::string name ="");
		static ComputePipelineState::ptr get_cache(ComputePipelineStateDesc& desc, std::string name = "");

	};



	using shader_identifier = std::array<std::byte, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES>;



	struct HitGroup
	{
		std::wstring name;
		RootSignature::ptr local_root;

		D3D12_HIT_GROUP_TYPE type;
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


		void include(CD3DX12_STATE_OBJECT_DESC& target) const
		{
			auto lib = target.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();


			D3D12_SHADER_BYTECODE libdxil = CD3DX12_SHADER_BYTECODE((void*)library->get_blob().data(), library->get_blob().size());
			lib->SetDXILLibrary(&libdxil);

			for (auto& e : exports)
			{

				lib->DefineExport(e.first.c_str(), e.second.empty() ? nullptr : e.second.c_str());
			}
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
		UINT MaxPayloadSizeInBytes=0;
		UINT MaxAttributeSizeInBytes=0;
		
	};

	class StateObject:public PipelineStateBase, public virtual Events::prop_handler
	{
		ComPtr<ID3D12StateObjectProperties> stateObjectProperties;

		void on_change() override
		{			
			CD3DX12_STATE_OBJECT_DESC raytracingPipeline{ desc.collection ? D3D12_STATE_OBJECT_TYPE_COLLECTION : D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE };

			for (auto& l : desc.libraries)
			{
				l.include(raytracingPipeline);


				debuggable |= l.library && l.library->depends_on("DebugInfo");
			}

			if (desc.global_root)
			{
				auto globalRootSignature = raytracingPipeline.CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();

				globalRootSignature->SetRootSignature(desc.global_root->get_native().Get());
			}

			if (desc.MaxTraceRecursionDepth)
			{
				auto pipelineConfig = raytracingPipeline.CreateSubobject<CD3DX12_RAYTRACING_PIPELINE_CONFIG_SUBOBJECT>();
				pipelineConfig->Config(desc.MaxTraceRecursionDepth);
			}

			if (desc.MaxPayloadSizeInBytes)
			{
				auto shaderConfig = raytracingPipeline.CreateSubobject<CD3DX12_RAYTRACING_SHADER_CONFIG_SUBOBJECT>();
				shaderConfig->Config(desc.MaxPayloadSizeInBytes, desc.MaxAttributeSizeInBytes);
			}


			std::map<RootSignature::ptr, CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT*> roots;


			for (auto& e : desc.hit_groups)
			{
				if (e.local_root)
					roots[e.local_root] = nullptr;
			}


			for (auto& e : roots)
			{
				auto localRootSignature = raytracingPipeline.CreateSubobject<CD3DX12_LOCAL_ROOT_SIGNATURE_SUBOBJECT>();
				localRootSignature->SetRootSignature(e.first->get_native().Get());

				auto rootSignatureAssociation = raytracingPipeline.CreateSubobject<CD3DX12_SUBOBJECT_TO_EXPORTS_ASSOCIATION_SUBOBJECT>();
				rootSignatureAssociation->SetSubobjectToAssociate(*localRootSignature);

				e.second = rootSignatureAssociation;
			}


			for (auto& e : desc.hit_groups)
			{
				auto hitGroup = raytracingPipeline.CreateSubobject<CD3DX12_HIT_GROUP_SUBOBJECT>();

				hitGroup->SetHitGroupExport(e.name.c_str());
				//TODO: more shaders
				hitGroup->SetClosestHitShaderImport(e.closest_hit_shader.c_str());
				hitGroup->SetHitGroupType(e.type);

				if (e.local_root)
					roots[e.local_root]->AddExport(e.name.c_str());
			}

			for (auto& c : desc.collections)
			{
				auto sharedCollection = raytracingPipeline.CreateSubobject<CD3DX12_EXISTING_COLLECTION_SUBOBJECT>();
				sharedCollection->SetExistingCollection(c->get_native().Get());

				debuggable |= c->debuggable;
			}


			TEST(Device::get().get_native_device()->CreateStateObject(raytracingPipeline, IID_PPV_ARGS(&tracked_info->m_StateObject)));
			TEST(tracked_info->m_StateObject.As(&stateObjectProperties));

			event_change();



		}

		static shader_identifier identify(void* data)
		{
			shader_identifier result;

			memcpy(result.data(), data, result.size());

			return result;
		}
		
		
	public:

		const StateObjectDesc desc;
		using ptr = s_ptr<StateObject>;
		ComPtr<ID3D12StateObject> get_native()
		{
			return tracked_info->m_StateObject;
		}Events::Event<void> event_change;
		StateObject(StateObjectDesc&desc):desc(desc)
		{

			for (auto& l : desc.libraries)
			{
				register_shader(l.library);
			}

			for (auto& c : desc.collections)
			{
				c->event_change.register_handler(this,[this]()
				{
						on_change();		
				});
			}
			
			on_change();
			
		}

		virtual ~StateObject()
		{


		}
		shader_identifier get_shader_id(std::wstring_view name)
		{
			return identify(stateObjectProperties->GetShaderIdentifier(name.data()));
		}
	};

}



/*
// REGISTER_TYPE(Graphics::PipelineStateBase);
// REGISTER_TYPE(Graphics::PipelineState);
*/

}