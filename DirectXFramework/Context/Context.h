
template<class T>
struct shader_state
{
    ConstBufferState const_buffer_state;
    ShaderResourceState shader_resource_state;
    SamplerState sampler_state;

    shader_state<T>& get()
    {
        return *this;
    }
};
class Context : public NativeContext, shader_state<pixel_shader>, shader_state<vertex_shader>, shader_state<geometry_shader>
{
        friend class Device;


        Context(const Context& r) = delete;

        RenderTargetState render_target_state;
        rasterizer_state::ptr rast_state;
//	blend_state::ptr rast_state;


        input_layout::ptr layout;
        vertex_shader::ptr v_shader;
        pixel_shader::ptr p_shader;
        geometry_shader::ptr g_shader;
        hull_shader::ptr h_shader;
        domain_shader::ptr d_shader;
        D3D11_PRIMITIVE_TOPOLOGY topology;


        pipeline_state::ptr current_state;

        std::vector<D3D11_VIEWPORT> vps;
        std::vector<sizer_long> scissors;
    public:
        Context(DX11_DeviceContext _native_context);
        template<class T>
        shader_state<T>& get_shader_state()
        {
            return shader_state<T>::get();
        }

        void flush(bool force = false);
        void draw(int count, int offset);
        void draw_indexed(int count, int index_offset, int vertex_offset);
        void draw_instanced(int count, int offset, int instance_count, int instance_offset = 0);
        void draw_indexed_instanced(int count, int index_offset, int vertex_offset, int instance_count, int instance_offset);

        RenderTargetState& get_rts();

        void clear_rtv(RenderTargetView& rtv, vec4 color);

        void clear_depth(DepthStencilView& dsv, float value, unsigned char stencil);

        void clear_depth(DepthStencilView& dsv, float value);

        void clear_depth(DepthStencilView& dsv, unsigned char stencil);

        template<class T>
        void set_index_buffer(std::shared_ptr<T> obj, DXGI_FORMAT format, unsigned int offset = 0)
        {
            native_context->IASetIndexBuffer(static_cast<ID3D11Buffer*>(obj->get_native()), format, offset);
        }

        void set(pipeline_state::ptr s);


        void set(depth_stencil_state& state, unsigned int ref = 0);

        void set(blend_state& state, vec4 factor = vec4(1, 1, 1, 1), unsigned int mask = 0xffffffff);

        void set(rasterizer_state::ptr state);

        void set(input_layout::ptr layout);

        void set(geometry_shader::ptr shader);

        void set(vertex_shader::ptr shader);

        void set(pixel_shader::ptr shader);

        void set(domain_shader::ptr shader);

        void set(hull_shader::ptr shader);

        template<class T, class ...Args>
        void set(std::shared_ptr<T>& obj, Args ... args)
        {
            set(*obj, args...);
        }

        template<class T, class ...Args>
        void set(const std::shared_ptr<T>& obj, Args ... args)
        {
            set(*obj, args...);
        }

        template<class T, class ...Args>
        void set(T& obj, Args... args)
        {
            // TODO: make this function unused
            obj.set(*this, args...);
        }

        void set_viewports(std::vector<D3D11_VIEWPORT>& vps);

        std::vector<D3D11_VIEWPORT>& get_viewports();


        void set_scissors(std::vector<sizer_long>& vps);

        std::vector<sizer_long>& get_scissors();


        void set_topology(D3D11_PRIMITIVE_TOPOLOGY topology);

        vertex_shader::ptr get_vertex_shader()
        {
            return v_shader;
        }

        void generate_mipmaps(ShaderResourceView& srv);


};