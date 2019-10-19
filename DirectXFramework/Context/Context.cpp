#include "pch.h"


namespace DX11
{

    void Context::set_topology(D3D11_PRIMITIVE_TOPOLOGY topology)
    {
        if (this->topology != topology)
        {
            get_native()->IASetPrimitiveTopology(topology);
            this->topology = topology;
        }
        else
        {
            topology_optimized_count++;
		}current_state = nullptr;
    }

    std::vector<D3D11_VIEWPORT> &Context::get_viewports()
    {
        return vps;
    }

    void Context::set_viewports(std::vector<D3D11_VIEWPORT> &vps)
    {
        if (this->vps != vps)
        {
            this->vps = vps;
            native_context->RSSetViewports(static_cast<UINT>(vps.size()), vps.data());
        }
        else
        {
            viewports_optimized_count++;
        }
    }

	std::vector<sizer_long> &Context::get_scissors()
	{
		return scissors;
	}

	void Context::set_scissors(std::vector<sizer_long> &scissors)
	{
		if (this->scissors != scissors)
		{
			this->scissors = scissors;
			native_context->RSSetScissorRects(static_cast<UINT>(scissors.size()), reinterpret_cast<D3D11_RECT *>(scissors.data()));
		}
		else
		{
			viewports_optimized_count++;
		}
	}



	void Context::set(pipeline_state::ptr s)
	{
		if (s != current_state)
		{
			set(s->v_shader);
			set(s->p_shader);
			set(s->g_shader);
			set(s->d_shader);
			set(s->h_shader);
			set_topology(s->topology);
			set(s->layout);
			set(s->rasterizer);
			set(s->blend);
			set(s->depth_stencil);

			current_state = s;

		}
else
		total_pipeline_count++;

	}



	
    void Context::set(pixel_shader::ptr shader)
    {
        if (this->p_shader != shader)
        {
            shader->set(*this);
            this->p_shader = shader;
        }
        else
        {
            pixel_optimized_count++;
        }
		current_state = nullptr;
    }

    void Context::set(vertex_shader::ptr shader)
    {
        if (this->v_shader != shader)
        {
            shader->set(*this);
            this->v_shader = shader;
        }
        else
        {
            vertex_optimized_count++;
		}current_state = nullptr;
    }

    void Context::set(geometry_shader::ptr shader)
    {
        if (this->g_shader != shader)
        {
            shader->set(*this);
            this->g_shader = shader;
        }
        else
        {
            geometry_optimized_count++;
		}current_state = nullptr;
    }

	void Context::set(domain_shader::ptr shader)
	{
		if (this->d_shader != shader)
		{
			shader->set(*this);
			this->d_shader = shader;
		}
		else
		{
			vertex_optimized_count++;
		}current_state = nullptr;
	}


	void Context::set(hull_shader::ptr shader)
	{
		if (this->h_shader != shader)
		{
			shader->set(*this);
			this->h_shader = shader;
		}
		else
		{
			vertex_optimized_count++;
		}current_state = nullptr;
	}

    void Context::set(input_layout::ptr layout)
    {
        if (this->layout != layout)
        {
            layout->set(*this);
            this->layout = layout;
        }
        else
        {
            layout_optimized_count++;
		}current_state = nullptr;
    }

    void Context::set(rasterizer_state::ptr state)
    {
		if (rast_state != state)
		{
			native_context->RSSetState(state->get_native());
			rast_state = state;
		}
		else
		{
			state_optimized_count++;
		}current_state = nullptr;
    }

    void Context::set(blend_state &state, vec4 factor , unsigned int mask)
    {
        native_context->OMSetBlendState(state.get_native(), factor.data(), mask);
    }

    void Context::set(depth_stencil_state &state, unsigned int ref)
    {
        native_context->OMSetDepthStencilState(state.get_native(), ref);
    }
	
    void Context::clear_depth(DepthStencilView &dsv, unsigned char stencil)
    {
        native_context->ClearDepthStencilView(*dsv, D3D11_CLEAR_STENCIL, 0, stencil);
    }

    void Context::clear_depth(DepthStencilView &dsv, float value)
    {
        native_context->ClearDepthStencilView(*dsv, D3D11_CLEAR_DEPTH, value, 0);
    }

    void Context::clear_depth(DepthStencilView &dsv, float value, unsigned char stencil)
    {
        native_context->ClearDepthStencilView(*dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, value, stencil);
    }

    void Context::clear_rtv(RenderTargetView &rtv, vec4 color)
    {
        native_context->ClearRenderTargetView(*rtv, color.data());
    }

    RenderTargetState &Context::get_rts()
    {
        return render_target_state;
    }

    void Context::draw_indexed(int count, int index_offset, int vertex_offset)
    {
        flush();
        native_context->DrawIndexed(count, index_offset, vertex_offset);
    }

	void Context::draw_indexed_instanced(int count, int index_offset, int vertex_offset, int instance_count, int instance_offset)
	{
		flush();
		native_context->DrawIndexedInstanced(count, instance_count, index_offset, vertex_offset, instance_offset);
	}

    void Context::draw(int count, int offset)
    {
        flush();
        native_context->Draw(count, offset);
    }

	void Context::draw_instanced(int count, int offset, int instance_count, int instance_offset)
	{
		flush();
		native_context->DrawInstanced(count, instance_count, offset, instance_offset);
	}

    void Context::flush(bool force /*= false*/)
    {
        if (p_shader && (p_shader != pixel_shader::null))
        {
            shader_state<pixel_shader>::const_buffer_state.set<pixel_shader>(*this, force);
            shader_state<pixel_shader>::shader_resource_state.set<pixel_shader>(*this, force);
            shader_state<pixel_shader>::sampler_state.set<pixel_shader>(*this, force);
        }
        else
        {
            pixel_optimized_count++;
        }

        if (v_shader && (v_shader != vertex_shader::null))
        {
            shader_state<vertex_shader>::sampler_state.set<vertex_shader>(*this, force);
            shader_state<vertex_shader>::const_buffer_state.set<vertex_shader>(*this, force);
            shader_state<vertex_shader>::shader_resource_state.set<vertex_shader>(*this, force);
        }
        else
        {
            vertex_optimized_count++;
        }

        if (g_shader && (g_shader != geometry_shader::null))
        {
            shader_state<geometry_shader>::const_buffer_state.set<geometry_shader>(*this, force);
            shader_state<geometry_shader>::shader_resource_state.set<geometry_shader>(*this, force);
            shader_state<geometry_shader>::sampler_state.set<geometry_shader>(*this, force);
        }
        else
        {
            geometry_optimized_count++;
        }

        if (force || render_target_state.is_changed())
            render_target_state.set(native_context);
    }

    Context::Context(DX11_DeviceContext _native_context) : NativeContext(_native_context)
    {
    }

	void Context::generate_mipmaps(ShaderResourceView& srv)
	{
		native_context->GenerateMips(*srv);
	}

}
