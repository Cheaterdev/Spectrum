#include "pch.h"
#include "Vertexes.h"

namespace DX12
{

    void vertex_buffer_stage::set(MeshRenderContext::ptr& context)
    {
        context->pipeline.layout = layout;
        // context.set(layout ? layout : input_layout::null);
        //     for (auto && b : base_buffers)
        //       b->update(context->list);
    //    context->list->get_graphics().set_vertex_buffers(0, native_handles);
    }

    void vertex_buffer_stage::add(GPUBuffer::ptr buffer, std::vector<vertex_buffer_layout>& layouts)
    {
        for (auto& layout : layouts)
            layout.slot = (UINT)base_buffers.size();

        base_buffers.push_back(buffer);
///////////////////////////////////////////////////////////////////        native_handles.push_back(buffer->get_vertex_buffer_view());
    }

    vertex_buffer_stage& vertex_mesh::get_layout(vertex_shader::ptr& shader)
    {
        auto shader_desc = shader->get_input_desc();
        vertex_buffer_stage& stage = resulted_layouts[shader_desc];

        if (!stage.layout.inputs.size())
        {
            stage = vertex_buffer_stage();
            input_layout_header new_header;

            for (auto& s : shader_desc->inputs)
            {
                if (to_lower(s.name) == "sv_instanceid")
                    continue;

                bool found = false;

                for (auto& b : buffers)
                {
                    for (auto& l : b.layouts)
                    {
                        if (l.name == s.name && l.index == s.index)
                        {
                            //    stage.add(b.buffer, b.layouts);
							input_layout_row elem;
                            elem.SemanticName = l.name;
                            elem.AlignedByteOffset = l.offset;
                            elem.Format = l.format;
                            elem.InputSlot = l.slot;
                            elem.InputSlotClass = static_cast<D3D12_INPUT_CLASSIFICATION>(l.type);
                            elem.InstanceDataStepRate = 0;
                            elem.SemanticIndex = l.index;
                            new_header.inputs.push_back(elem);
                            found = true;
                            break;
                        }
                    }

                    if (found) break;
                }

                if (!found)
                    Log::get() << Log::LEVEL_ERROR << "Incompatible layout creating here for semantic: " << s.name << s.index << Log::endl;
            }

            stage.create_layout(new_header);
        }

        return stage;
    }

    void vertex_mesh::set(MeshRenderContext::ptr& list)
    {
        get_layout(list->pipeline.vertex).set(list);
    }

    void vertex_buffer_stage::create_layout(input_layout_header& header)
    {
        layout = header;
    }

    void vertex_buffer_stage::create_layout(std::vector<vertex_buffer_layout>& layouts, vertex_shader::ptr& shader)
    {
        input_layout_header new_header;
        auto shader_desc = shader->get_input_desc();

        for (auto& s : shader_desc->inputs)
        {
            if (to_lower(s.name) == "sv_instanceid")
                continue;

            for (auto& l : layouts)
            {
                if (l.name == s.name && l.index == s.index)
                {
					input_layout_row elem;
                    elem.SemanticName = l.name;
                    elem.AlignedByteOffset = l.offset;
                    elem.Format = l.format;
                    elem.InputSlot = l.slot;
                    elem.InputSlotClass = static_cast<D3D12_INPUT_CLASSIFICATION>(l.type);
                    elem.InstanceDataStepRate = 0;
                    elem.SemanticIndex = l.index;
                    new_header.inputs.push_back(elem);
                    break;
                }
            }
        }

        create_layout(new_header);
    }


}