#pragma once


namespace DX12
{
    class PipelineStateBase;



    template<class _shader_type>
    class Shader : public resource_manager<_shader_type, D3D::shader_header>
    {
            friend class resource_manager<_shader_type, D3D::shader_header>;
        protected:
            std::string blob;
            static const char* compile_code;
            MD5 hash;
			MD5 blob_hash;
            D3D::reflection reflection;

            //   DX11_ClassLinkage class_linkage;
            //   std::vector<ID3D11ClassInstance*>    class_instances;
            virtual void compile() {};
            friend class PipelineStateBase;
            std::set<PipelineStateBase*> pipelines;

            void operator=(const Shader& r)
            {
                resource_manager<_shader_type, D3D::shader_header>::operator=(r);
                reflection = r.reflection;
                blob = r.blob;
                hash = r.hash;

                for (auto& p : pipelines)
                    p->on_change();
            }


            void own_id()
            {
				blob_hash = MD5(blob);
                this->id = shader_ids[blob_hash.hexdigest()];
            }
        public:
            static Cache<std::string, int> shader_ids;
            const MD5& get_hash() const
            {
                return hash;
            }
            int id;
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
#ifdef USE_D3D_COMPILER
                std::vector<D3D_SHADER_MACRO> Macros;
                std::string build_macro = "BUILD_FUNC_" + func_name;
                std::string build_def = "1";
                D3D_SHADER_MACRO Macro;
                Macro.Definition = build_def.c_str();
                Macro.Name = build_macro.c_str();
                Macros.push_back(Macro);
                Macro.Definition = nullptr;
                Macro.Name = nullptr;
                Macros.push_back(Macro);

				for (unsigned int i = 0; i < macros.size(); i++)
				{
					D3D_SHADER_MACRO Macro;
					Macro.Definition = macros[i].value.c_str();
					Macro.Name = macros[i].name.c_str();
					Macros.push_back(Macro);
				}


                D3D_Blob res_blob = nullptr;
                D3D_Blob error_blob = nullptr;

                if (FAILED(D3DCompile(data.data(), data.size(), nullptr, Macros.data(), nullptr, func_name.c_str(), compile_code, flags, flags, &res_blob, &error_blob)))
                    if (error_blob != nullptr)
                        Log::get() << static_cast<char*>(error_blob->GetBufferPointer()) << Log::endl;

                if (!res_blob)
                    return nullptr;

                auto result = std::make_shared<_shader_type>();
                result->blob.assign(static_cast<char*>(res_blob->GetBufferPointer()), static_cast<char*>(res_blob->GetBufferPointer()) + res_blob->GetBufferSize());
                result->own_id();
                result->reflection = D3D::reflection(D3D::D3D_VERSION::V12, result->blob);
                result->compile();
                result->hash = MD5(data);
                return result;
#else
                MessageBoxA(0, "USE_D3D_COMPILER", "Error", 0);
                return  std::shared_ptr<_shader_type>(new _shader_type());
#endif
            }


			static std::shared_ptr<_shader_type> load_native(const D3D::shader_header& header, resource_file_depender& depender)
			{
#ifdef USE_D3D_COMPILER
				D3D_Blob res_blob = nullptr;
				D3D::shader_include In(header.file_name, depender);
				std::vector<D3D_SHADER_MACRO> Macros;

				for (unsigned int i = 0; i < header.macros.size(); i++)
				{
					D3D_SHADER_MACRO Macro;
					Macro.Definition = header.macros[i].value.c_str();
					Macro.Name = header.macros[i].name.c_str();
					Macros.push_back(Macro);
				}

				std::string build_macro = "BUILD_FUNC_" + header.entry_point;
				std::string build_def = "1";
				D3D_SHADER_MACRO Macro;
				Macro.Definition = build_def.c_str();
				Macro.Name = build_macro.c_str();
				Macros.push_back(Macro);
				Macro.Definition = nullptr;
				Macro.Name = nullptr;
				Macros.push_back(Macro);
				std::shared_ptr<file> file;
				auto flags = header.flags | D3DCOMPILE_PARTIAL_PRECISION; // | D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
				std::string data;

				while (!res_blob)
				{
					depender.clear();
					D3D_Blob error_blob = nullptr;
					file = FileSystem::get().get_file(header.file_name);

					if (file)
					{
						data = file->load_all();

						if (FAILED(D3DCompile(data.data(), data.size(), header.file_name.c_str(), Macros.data(), &In, header.entry_point.c_str(), compile_code, flags, flags, &res_blob, &error_blob)))
							if (error_blob != nullptr)
							{
								Log::get() << static_cast<char*>(error_blob->GetBufferPointer()) << Log::endl;
								MessageBoxA(nullptr, static_cast<char*>(error_blob->GetBufferPointer()), "Error", 0);
							}
					}

					else
						MessageBoxA(nullptr, "no file found", "Error", 0);
				}

			/*	D3D_Blob new_blob = nullptr;
				D3DStripShader(res_blob->GetBufferPointer(), res_blob->GetBufferSize(), D3DCOMPILER_STRIP_REFLECTION_DATA
			|D3DCOMPILER_STRIP_DEBUG_INFO,
			|		D3DCOMPILER_STRIP_TEST_BLOBS ,
			|		D3DCOMPILER_STRIP_PRIVATE_DATA,
			|		D3DCOMPILER_STRIP_ROOT_SIGNATURE, &new_blob);*/
                depender.add_depend(file);
                auto result = std::make_shared<_shader_type>();
                result->header = header;
                result->blob.assign(static_cast<char*>(res_blob->GetBufferPointer()), static_cast<char*>(res_blob->GetBufferPointer()) + res_blob->GetBufferSize());
                result->reflection = D3D::reflection(D3D::D3D_VERSION::V12, result->blob);
                result->compile();
                result->own_id();
                result->hash = MD5(data);
                return result;
#else
                MessageBoxA(0, "USE_D3D_COMPILER", "Error", 0);
                return  std::shared_ptr<_shader_type>(new _shader_type());
#endif
            }
        private:
            friend class boost::serialization::access;
            template<class Archive>
            void serialize(Archive& ar, const unsigned int)
            {
                ar& NVP(blob);
                ar& NVP(reflection);
                ar& NVP(hash);

                if (Archive::is_loading::value)
                {
                    own_id();
                    compile();
                }
            }
    };

    template<class _shader_type>
    Cache<std::string, int> Shader<_shader_type>::shader_ids([](const std::string& blob)
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

}