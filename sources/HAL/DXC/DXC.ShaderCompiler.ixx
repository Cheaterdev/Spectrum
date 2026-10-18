export module HAL:API.ShaderCompiler;

import DXCompiler;
import Core;

export namespace HAL
{
    namespace API
    {

        struct ShaderCompiler
        {
            // A failed compile can be fixed and retried (live shader editing); platforms without a compiler cannot.
            static constexpr bool can_compile = true;

        protected:
         //   ShaderCompiler();
            friend class shader_include_dxil;
            dxc::DxcDllSupport		DxcDllHelper;
            IDxcCompiler3* compiler = nullptr;
            IDxcUtils* library = nullptr;

        };
    }
}
