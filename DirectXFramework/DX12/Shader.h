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
			static const char* compile_code_dxil;

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
              // reflection = r.reflection;
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
            static Cache<std::string, size_t> shader_ids;

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
				D3D::shader_include In("unknown", depender);

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
				return result;
            }

			static std::shared_ptr<_shader_type> load_native(const D3D::shader_header& header, resource_file_depender& depender)
			{

				std::unique_ptr<std::string> res_blob ;
				D3D::shader_include In(header.file_name, depender);
			
				while (!res_blob)
				{
					depender.clear();
					res_blob = D3D12ShaderCompilerInfo::get().Compile_Shader_File(header.file_name, header.macros, compile_code_dxil, header.entry_point, &In);
				}

                auto result = std::make_shared<_shader_type>();
                result->header = header;
                result->blob = std::move(*res_blob);
                result->compile();
                result->own_id();
                result->hash = MD5(result->blob);
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

}



/*
using namespace dxc;


HRESULT CreateCompiler(IDxcCompiler** ppCompiler) {
	return DxcCreateInstance(CLSID_DxcCompiler, __uuidof(IDxcCompiler),
		(void**)ppCompiler);
}


HRESULT CompileFromBlob(IDxcBlobEncoding* pSource, LPCWSTR pSourceName,
	const D3D_SHADER_MACRO* pDefines, IDxcIncludeHandler* pInclude,
	LPCSTR pEntrypoint, LPCSTR pTarget, UINT Flags1,
	UINT Flags2, ID3DBlob** ppCode,
	ID3DBlob** ppErrorMsgs) {
	CComPtr<IDxcCompiler> compiler;
	CComPtr<IDxcOperationResult> operationResult;
	HRESULT hr;

	// Upconvert legacy targets
	char Target[7] = "?s_6_0";
	Target[6] = 0;
	if (pTarget[3] < '6') {
		Target[0] = pTarget[0];
		pTarget = Target;
	}

	try {
		CA2W pEntrypointW(pEntrypoint);
		CA2W pTargetProfileW(pTarget);
		std::vector<std::wstring> defineValues;
		std::vector<DxcDefine> defines;
		if (pDefines) {
			CONST D3D_SHADER_MACRO* pCursor = pDefines;

			// Convert to UTF-16.
			while (pCursor->Name) {
				defineValues.push_back(std::wstring(CA2W(pCursor->Name)));
				if (pCursor->Definition)
					defineValues.push_back(std::wstring(CA2W(pCursor->Definition)));
				else
					defineValues.push_back(std::wstring());
				++pCursor;
			}

			// Build up array.
			pCursor = pDefines;
			size_t i = 0;
			while (pCursor->Name) {
				defines.push_back(
					DxcDefine{ defineValues[i++].c_str(), defineValues[i++].c_str() });
				++pCursor;
			}
		}

		std::vector<LPCWSTR> arguments;
		// /Gec, /Ges Not implemented:
		//if(Flags1 & D3DCOMPILE_ENABLE_BACKWARDS_COMPATIBILITY) arguments.push_back(L"/Gec");
		//if(Flags1 & D3DCOMPILE_ENABLE_STRICTNESS) arguments.push_back(L"/Ges");
		if (Flags1 & D3DCOMPILE_IEEE_STRICTNESS) arguments.push_back(L"/Gis");
		if (Flags1 & D3DCOMPILE_OPTIMIZATION_LEVEL2)
		{
			switch (Flags1 & D3DCOMPILE_OPTIMIZATION_LEVEL2)
			{
			case D3DCOMPILE_OPTIMIZATION_LEVEL0: arguments.push_back(L"/O0"); break;
			case D3DCOMPILE_OPTIMIZATION_LEVEL2: arguments.push_back(L"/O2"); break;
			case D3DCOMPILE_OPTIMIZATION_LEVEL3: arguments.push_back(L"/O3"); break;
			}
		}
		// Currently, /Od turns off too many optimization passes, causing incorrect DXIL to be generated.
		// Re-enable once /Od is implemented properly:
		//if(Flags1 & D3DCOMPILE_SKIP_OPTIMIZATION) arguments.push_back(L"/Od");
		if (Flags1 & D3DCOMPILE_DEBUG) arguments.push_back(L"/Zi");
		if (Flags1 & D3DCOMPILE_PACK_MATRIX_ROW_MAJOR) arguments.push_back(L"/Zpr");
		if (Flags1 & D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR) arguments.push_back(L"/Zpc");
		if (Flags1 & D3DCOMPILE_AVOID_FLOW_CONTROL) arguments.push_back(L"/Gfa");
		if (Flags1 & D3DCOMPILE_PREFER_FLOW_CONTROL) arguments.push_back(L"/Gfp");
		// We don't implement this:
		//if(Flags1 & D3DCOMPILE_PARTIAL_PRECISION) arguments.push_back(L"/Gpp");
		if (Flags1 & D3DCOMPILE_RESOURCES_MAY_ALIAS) arguments.push_back(L"/res_may_alias");
		arguments.push_back(L"-HV");
		arguments.push_back(L"2016");

		IFR(CreateCompiler(&compiler));
		IFR(compiler->Compile(pSource, pSourceName, pEntrypointW, pTargetProfileW,
			arguments.data(), (UINT)arguments.size(),
			defines.data(), (UINT)defines.size(), pInclude,
			&operationResult));
	}
	catch (const std::bad_alloc&) {
		return E_OUTOFMEMORY;
	}
	catch (const CAtlException & err) {
		return err.m_hr;
	}

	operationResult->GetStatus(&hr);
	if (SUCCEEDED(hr)) {
		return operationResult->GetResult((IDxcBlob**)ppCode);
	}
	else {
		if (ppErrorMsgs)
			operationResult->GetErrorBuffer((IDxcBlobEncoding**)ppErrorMsgs);
		return hr;
	}
}
*/