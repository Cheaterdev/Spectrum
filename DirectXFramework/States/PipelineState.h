
	struct pipeline_state
	{

		using ptr = s_ptr<pipeline_state>;
		vertex_shader::ptr v_shader = vertex_shader::null;
		pixel_shader::ptr p_shader = pixel_shader::null;
		geometry_shader::ptr g_shader = geometry_shader::null;
		domain_shader::ptr d_shader = domain_shader::null;
		hull_shader::ptr h_shader = hull_shader::null;

		input_layout::ptr layout = input_layout::null;
		D3D11_PRIMITIVE_TOPOLOGY topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		blend_state::ptr  blend;
		rasterizer_state::ptr  rasterizer;
		depth_stencil_state::ptr  depth_stencil;

		static ptr create()
		{
			return ptr(new pipeline_state());
		}
	private:
		pipeline_state(){}
	};
