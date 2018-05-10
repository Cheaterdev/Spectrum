
template<class _shader_type>
class shader : public resource_manager<_shader_type, D3D::shader_header>, protected NativeContextAccessor
{
        friend class resource_manager<_shader_type, D3D::shader_header>;
    protected:
        std::string blob;
        D3D::reflection reflection;

        DX11_ClassLinkage class_linkage;
        std::vector<ID3D11ClassInstance*>    class_instances;
        virtual void compile() = 0;
    public:
        virtual ~shader()
        {
        }

        std::string& get_blob()
        {
            return blob;
        }
        D3D::shader_header& get_header()
        {
            return header;
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

            while (!res_blob)
            {
                depender.clear();
                D3D_Blob error_blob = 0;
                file = FileSystem::get().get_file(header.file_name);

                if (file)
                {
                    auto data = file->load_all();

                    if (FAILED(D3DCompile(data.data(), data.size(), header.file_name.c_str(), Macros.data(), &In, header.entry_point.c_str(), _shader_type::compile_code, header.flags, header.flags, &res_blob, &error_blob)))
                        if (error_blob != nullptr)
                        {
                            Log::get() << static_cast<char*>(error_blob->GetBufferPointer()) << Log::endl;
                            MessageBoxA(nullptr, static_cast<char*>(error_blob->GetBufferPointer()), "Error", 0);
                        }
                }

                else
                    MessageBoxA(nullptr, "no file found", "Error", 0);
            }

            depender.add_depend(file);
            auto result = std::make_shared<_shader_type>();
            result->header = header;
            result->blob.assign(static_cast<char*>(res_blob->GetBufferPointer()), static_cast<char*>(res_blob->GetBufferPointer()) + res_blob->GetBufferSize());
            result->reflection = D3D::reflection(D3D::D3D_VERSION::V11, result->blob);
            {
                auto& lock = Device::get().lock();
                Device::get().get_native_device()->CreateClassLinkage(&result->class_linkage);
            }
            result->class_instances.resize(result->reflection.get_interface_slots_count());
            result->compile();
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
            ar& blob;
            ar& reflection;

            if (Archive::is_loading::value)
                compile();
        }
};



class pixel_shader: public shader<pixel_shader>
{
        friend class shader<pixel_shader>;
        DX11_PixelShader native_shader;

        static const char* compile_code;
        void compile() override;

    public:

        using ptr = s_ptr<pixel_shader>;

        static const ptr null;
    protected:
        friend class Context;

        void set(NativeContext& context);
};


class geometry_shader : public shader<geometry_shader>
{
        friend class shader<geometry_shader>;
        DX11_GeometryShader native_shader;

        static const char* compile_code;
        void compile() override;

    public:

        using ptr = s_ptr<geometry_shader>;

        static const ptr null;
    protected:
        friend class Context;

        void set(NativeContext& context);
};



class vertex_shader : public shader<vertex_shader>
{
        friend class shader<vertex_shader>;
        DX11_VertexShader native_shader;

        static const char* compile_code;
        std::shared_ptr<shader_inputs> input_desc;
        void compile() override;
    public:


        using ptr = s_ptr<vertex_shader>;
        std::shared_ptr<shader_inputs>& get_input_desc()
        {
            return input_desc;
        }

        static const ptr null;

    protected:
        friend class Context;

        void set(NativeContext& context);
};



class hull_shader : public shader<hull_shader>
{
        friend class shader<hull_shader>;
        DX11_HullShader native_shader;

        static const char* compile_code;
        void compile() override;

    public:

        using ptr = s_ptr<hull_shader>;

        static const ptr null;
    protected:
        friend class Context;

        void set(NativeContext& context);
};




class domain_shader : public shader<domain_shader>
{
        friend class shader<domain_shader>;
        DX11_DomainShader native_shader;

        static const char* compile_code;
        void compile() override;

    public:

        using ptr = s_ptr<domain_shader>;

        static const ptr null;
    protected:
        friend class Context;

        void set(NativeContext& context);
};

