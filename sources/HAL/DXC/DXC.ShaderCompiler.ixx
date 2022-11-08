export module HAL:API.ShaderCompiler;

import DXCompiler;
import Core;

export namespace HAL
{
    namespace API
    {

        struct ShaderCompiler
        {
        protected:
         //   ShaderCompiler();
            friend class shader_include_dxil;
            dxc::DxcDllSupport		DxcDllHelper;
            IDxcCompiler* compiler = nullptr;
            IDxcLibrary* library = nullptr;
        };
    }
}
