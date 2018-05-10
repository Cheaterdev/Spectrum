#include "pch.h"
#undef min
namespace DX11
{

    const char* pixel_shader::compile_code = "ps_5_0";
    const char* vertex_shader::compile_code = "vs_5_0";
    const char* geometry_shader::compile_code = "gs_5_0";
    const char* domain_shader::compile_code = "ds_5_0";
    const char* hull_shader::compile_code = "hs_5_0";


    void pixel_shader::set(NativeContext& context)
    {
        get_native_context(context)->PSSetShader(native_shader, class_instances.data(), class_instances.size());
    }

    void pixel_shader::compile()
    {
        auto& lock = Device::get().lock();
        Device::get().get_native_device()->CreatePixelShader(blob.data(), blob.size(), class_linkage, &native_shader);
    }

    void vertex_shader::set(NativeContext& context)
    {
        get_native_context(context)->VSSetShader(native_shader, class_instances.data(), class_instances.size());
    }

    void vertex_shader::compile()
    {
        shader_inputs input_desc;

        for (auto& r : reflection.get_inputs())
        {
            input_desc.inputs.push_back({ r.SemanticName, r.SemanticIndex });
        }

        this->input_desc = layout_id_generator::get().get_unique(input_desc);
        {
            auto& lock = Device::get().lock();
            Device::get().get_native_device()->CreateVertexShader(blob.data(), blob.size(), class_linkage, &native_shader);
        }
    }


    void geometry_shader::set(NativeContext& context)
    {
        get_native_context(context)->GSSetShader(native_shader, class_instances.data(), class_instances.size());
    }

    void geometry_shader::compile()
    {
        auto& lock = Device::get().lock();
        Device::get().get_native_device()->CreateGeometryShader(blob.data(), blob.size(), class_linkage, &native_shader);
    }

    void hull_shader::set(NativeContext& context)
    {
        get_native_context(context)->HSSetShader(native_shader, class_instances.data(), class_instances.size());
    }

    void hull_shader::compile()
    {
        auto& lock = Device::get().lock();
        Device::get().get_native_device()->CreateHullShader(blob.data(), blob.size(), class_linkage, &native_shader);
    }


    void domain_shader::set(NativeContext& context)
    {
        get_native_context(context)->DSSetShader(native_shader, class_instances.data(), class_instances.size());
    }

    void domain_shader::compile()
    {
        auto& lock = Device::get().lock();
        Device::get().get_native_device()->CreateDomainShader(blob.data(), blob.size(), class_linkage, &native_shader);
    }

    const vertex_shader::ptr vertex_shader::null(new vertex_shader());
    const pixel_shader::ptr pixel_shader::null(new pixel_shader());
    const geometry_shader::ptr geometry_shader::null(new geometry_shader());
    const domain_shader::ptr domain_shader::null(new domain_shader());
    const hull_shader::ptr hull_shader::null(new hull_shader());
}