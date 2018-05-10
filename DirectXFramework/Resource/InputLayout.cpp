#include "pch.h"
namespace DX11
{
	std::shared_ptr<input_layout> input_layout::create_new(const input_layout_header &header, vertex_shader* shader)
	{
		DX11_InputLayout native_layout;

		if (header.inputs.size())
		{
			auto &lock=DX11::Device::get().lock();
		DX11::Device::get().get_native_device()->CreateInputLayout(header.inputs.data(), static_cast<unsigned int>(header.inputs.size()),
			shader->get_blob().data(), shader->get_blob().size(), &native_layout);
	}
        std::shared_ptr<input_layout> result;

        if (native_layout)
        {
            result.reset(new input_layout());
            result->native_layout = native_layout;
            result->desc = header;
        }

        return result;
    }

    std::shared_ptr<input_layout> input_layout::get_resource(const input_layout_header &h, std::shared_ptr<vertex_shader> shader)
    {
        return resource_manager_base<input_layout, input_layout_header, input_layout>::get().get_resource(h, shader.get());
    }
    std::shared_ptr<input_layout> input_layout::get_resource(const input_layout_header &h, vertex_shader* shader)
    {
        return resource_manager_base<input_layout, input_layout_header, input_layout>::get().get_resource(h, shader);
    }



    void input_layout::set(DX11::Context &context)
    {
		get_native_context(context)->IASetInputLayout(native_layout);
    }


    bool operator<(const shader_input_desc &l, const shader_input_desc &r)
    {
        OP(name,
           OP(index, return false;
             )
          );
    }

    bool operator<(const input_layout_header &l, const input_layout_header &r)
    {
        OP(inputs.size(),

           for (unsigned int i = 0; i < l.inputs.size(); i++)
    {
        OP(inputs[i].SemanticIndex,
           OP(inputs[i].Format,
              OP(inputs[i].InputSlot,
                 OP(inputs[i].AlignedByteOffset,
                    OP(inputs[i].InputSlotClass,
                       OP(inputs[i].InstanceDataStepRate,
                          return strcmp(l.inputs[i].SemanticName, r.inputs[i].SemanticName) < (int)0;
                         )
                      )
                   )
                )
             )
          );
        }
          );

		  return false;
    }


    bool operator==(const shader_input_desc &l, const shader_input_desc &r)
    {
        return l.name == r.name && l.index == r.index;
    }

    bool operator<(const shader_inputs &l, const shader_inputs &r)
    {
        if (l.inputs.size() == r.inputs.size())
        {
            for (unsigned int i = 0; i < l.inputs.size(); i++)
                if (!(l.inputs[i]== r.inputs[i]))
					return l.inputs[i] < r.inputs[i];

            return false;
        }

        return l.inputs.size() < r.inputs.size();
    }

    bool operator==(const shader_inputs &l, const shader_inputs &r)
    {
        if (l.inputs.size() == r.inputs.size())
        {
            for (unsigned int i = 0; i < l.inputs.size(); i++)
            {
                if (l.inputs[i].name == r.inputs[i].name)
                    return l.inputs[i].index == r.inputs[i].index;
                else
                    return false;
            }

            return true;
        }
        else
            return false;
    }


    bool operator<(const vertex_buffer_layout &l, const vertex_buffer_layout &r)
    {
        if (l.name == r.name)
        {
            if (l.index == r.index)
            {
                if (l.slot == r.slot)
                {
                    if (l.offset == r.offset)
                    {
                        if (l.format == r.format)
                        {
                            return false;
                        }
                        else
                            return l.format < r.format;
                    }
                    else
                        return l.offset < r.offset;
                }
                else
                    return l.slot < r.slot;
            }
            else
                return l.index < r.index;
        }
        else
            return l.name < r.name;
    }
	const input_layout::ptr input_layout::null(new input_layout());
}