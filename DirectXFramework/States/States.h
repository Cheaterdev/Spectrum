template<class _native_state, class _desc>
class state 
{
    _native_state native_state;
    _desc desc;
	typedef resource_manager_base<state<_native_state, _desc>, _desc, state<_native_state, _desc>> manager;
	friend class manager;
    static std::shared_ptr<state> create_new(const _desc &desc)
    {
        return  std::shared_ptr<state>(new state<_native_state, _desc>(desc));
    }

public:
	using ptr = s_ptr<state<_native_state, _desc>>;

    state(_desc desc);
	static std::shared_ptr<state> get_resource(const _desc &desc)
	{
		return manager::get().get_resource(desc);
	}

    _native_state get_native()
    {
        return native_state;
    }

};


typedef state<DX11_SamplerState, D3D11_SAMPLER_DESC> sampler_state;
typedef state<DX11_DepthStencilState, D3D11_DEPTH_STENCIL_DESC> depth_stencil_state;
typedef state<DX11_RasterizerState, D3D11_RASTERIZER_DESC> rasterizer_state;
typedef state<DX11_BlendState, D3D11_BLEND_DESC> blend_state;

extern template sampler_state;
extern template depth_stencil_state;
extern template rasterizer_state;
extern template blend_state;
