#pragma once
std::optional<SlotID> get_slot(std::string_view slot_name);
namespace DX12
{
    class PipelineStateBase;

class UsedSlots
{
   
public:
    std::set<SlotID> slots_usage;
	void merge(resource_file_depender & depender)
	{
        for (auto& d : depender.get_files())
        {
            auto autogen_start = d.file_name.find(L"autogen");
            if (autogen_start != std::wstring::npos)
            {
                auto autogen_end = autogen_start + 6;

                auto last_slash = d.file_name.find_first_of(L"\\/", autogen_end);

                if (last_slash == autogen_end + 1)
                {
                  
                    if (d.file_name.find_first_of(L"\\/", last_slash+1) == std::wstring::npos)
                    {
                        auto point = d.file_name.find(L".", last_slash);

                        auto name = d.file_name.substr(last_slash + 1, point - last_slash - 1);

                        auto slot_id = get_slot(convert(name));

                        assert(slot_id);
                        slots_usage.insert(slot_id.value());
                    }
                }
            }
        }
	}
    void merge(UsedSlots& other)
	{

        slots_usage.merge(other.slots_usage);
	}
void clear()
	{
    slots_usage.clear();
	}
private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int)
    {
        ar& NVP(slots_usage);
    }
};

    template<class _shader_type>
    class Shader : public resource_manager<_shader_type, D3D::shader_header>
    {
            friend class resource_manager<_shader_type, D3D::shader_header>;
        protected:
            std::string blob;
            static const char* compile_code;
			static const char* compile_code_dxil;

            MD5 hash;
			MD5 blob_hash;
            D3D::reflection reflection;

            //   DX11_ClassLinkage class_linkage;
            //   std::vector<ID3D11ClassInstance*>    class_instances;
            virtual void compile() {};
            friend class PipelineStateBase;
         
            std::mutex pipelines_mutex;
            std::set<PipelineStateBase*> pipelines;


            void on_register(PipelineStateBase* pip)
            {
                std::lock_guard<std::mutex> g(pipelines_mutex);
                pipelines.insert(pip);
            }

			void on_unregister(PipelineStateBase* pip)
			{
				std::lock_guard<std::mutex> g(pipelines_mutex);
				pipelines.erase(pip);
			}
            void operator=(const Shader& r)
            {
                resource_manager<_shader_type, D3D::shader_header>::operator=(r);
              // reflection = r.reflection;
                blob = r.blob;
                hash = r.hash;

                on_change();
            	
				std::lock_guard<std::mutex> g(pipelines_mutex);
                for (auto& p : pipelines)
                    p->on_change();
            }


            void own_id()
            {
				blob_hash = MD5(blob);
                this->id = shader_ids[blob_hash.hexdigest()];
            }
        public:
            static Cache<std::string, size_t> shader_ids;

            UsedSlots slots_usage;
            Events::Event<void> on_change;
            const MD5& get_hash() const
            {
                return hash;
            }

			size_t id;

            std::string& get_blob()
            {
                return blob;
            }

            const D3D::shader_header& get_header() const
            {
                return resource_manager<_shader_type, D3D::shader_header>::header;
            }

			static std::shared_ptr<_shader_type> create_from_memory(std::string data, std::string func_name, UINT flags, std::vector<D3D::shader_macro> macros = {})
            {
                auto t = CounterManager::get().start_count<_shader_type>();
                resource_file_depender depender;
				D3D::shader_include In("shaders/", depender);

				std::unique_ptr<std::string> res_blob;

                while (!res_blob)
                {
                    depender.clear();
                    res_blob = D3D12ShaderCompilerInfo::get().Compile_Shader(data, macros, compile_code_dxil, func_name, &In);
                }
                auto result = std::make_shared<_shader_type>();

           
				result->blob = std::move(*res_blob);
				result->own_id();
                result->compile();
				result->hash = MD5(result->blob);
                result->slots_usage.merge(depender);
				return result;
            }

			static std::shared_ptr<_shader_type> load_native(const D3D::shader_header& header, resource_file_depender& depender)
			{

				std::unique_ptr<std::string> res_blob ;
	
			
				while (!res_blob)
				{
					depender.clear();


                    if (header.contains_text)
                    {
                        D3D::shader_include In("shaders/", depender);
                        res_blob = D3D12ShaderCompilerInfo::get().Compile_Shader(header.file_name, header.macros, compile_code_dxil, header.entry_point, &In);
                    }                    
                    else
					{
						D3D::shader_include In(header.file_name, depender);
                        res_blob = D3D12ShaderCompilerInfo::get().Compile_Shader_File(header.file_name, header.macros, compile_code_dxil, header.entry_point, &In);
                    }
				}

                auto result = std::make_shared<_shader_type>();
                result->header = header;
                result->blob = std::move(*res_blob);
                result->compile();
                result->own_id();
                result->hash = MD5(result->blob);
                result->slots_usage.merge(depender);
           
            
                return result;
            }
        private:
            friend class boost::serialization::access;
            template<class Archive>
            void serialize(Archive& ar, const unsigned int)
            {
                ar& NVP(blob);
                ar& NVP(reflection);
                ar& NVP(hash);
                ar& NVP(slots_usage);
            	
                if (Archive::is_loading::value)
                {
                    own_id();
                    compile();
                }
            }
    };

    template<class _shader_type>
    Cache<std::string, size_t> Shader<_shader_type>::shader_ids([](const std::string& blob)
    {
        return Shader<_shader_type>::shader_ids.size();
    });







    class pixel_shader : public Shader<pixel_shader>
    {
            friend class Shader<pixel_shader>;
        public:

            using ptr = s_ptr<pixel_shader>;

            static const ptr null;
    };


    class geometry_shader : public Shader<geometry_shader>
    {
            friend class Shader<geometry_shader>;

        public:

            using ptr = s_ptr<geometry_shader>;

            static const ptr null;
        protected:
            friend class Context;


    };



    class vertex_shader : public Shader<vertex_shader>
    {
            friend class Shader<vertex_shader>;

            std::shared_ptr<Render::shader_inputs> input_desc;
            void compile() override;

        public:


            using ptr = s_ptr<vertex_shader>;
            std::shared_ptr<Render::shader_inputs>& get_input_desc()
            {
                return input_desc;
            }

            static const ptr null;


    };



    class hull_shader : public Shader<hull_shader>
    {
            friend class Shader<hull_shader>;
        public:
            using ptr = s_ptr<hull_shader>;
            static const ptr null;
    };




    class domain_shader : public Shader<domain_shader>
    {
            friend class Shader<domain_shader>;
        public:

            using ptr = s_ptr<domain_shader>;

            static const ptr null;
    };

    class compute_shader : public Shader<compute_shader>
    {
            friend class Shader<compute_shader>;
        public:

            using ptr = s_ptr<compute_shader>;

            static const ptr null;
    };


    class library_shader;
    class exported_shader
    {
    public:
        std::shared_ptr<library_shader> library;
        std::wstring shader_name;
    };
	
    class library_shader : public Shader<library_shader>
	{
		friend class Shader<library_shader>;

     
	public:

		using ptr = s_ptr<library_shader>;

		static const ptr null;

       
	};

}


