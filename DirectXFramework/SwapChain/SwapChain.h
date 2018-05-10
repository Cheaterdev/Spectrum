
struct hwnd_provider
{
    virtual	HWND get_hwnd() const = 0;
};

struct swap_chain_desc
{
    int max_fps;
    DXGI_FORMAT format;
    hwnd_provider* window;
    hwnd_provider* fullscreen;
    bool stereo;
};

class Device;

class SwapChainTexture: public Texture2D
{
    int left_index;
    int right_index;

    void init_views()
    {
       if (desc.ArraySize == 2)
        {
            D3D11_RENDER_TARGET_VIEW_DESC DescRT; ZeroMemory(&DescRT, sizeof(DescRT));
            DescRT.Format = desc.Format;
            DescRT.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
            DescRT.Texture2DArray.ArraySize = 1;
            DescRT.Texture2DArray.MipSlice = 0;

            left_index = render_target_views.size();
            DescRT.Texture2DArray.FirstArraySlice = 0;
            render_target_views.emplace_back(this, DescRT);

            right_index = render_target_views.size();
            DescRT.Texture2DArray.FirstArraySlice = 1;
            render_target_views.emplace_back(this, DescRT);
        }
    }

public:
    SwapChainTexture(DX11_Texture2D tex) : Texture2D(tex)
    {
		init_views();
    }
    RenderTargetView& get_left()
    {
        return render_target_views[left_index];
    }
    RenderTargetView &get_right()
    {
        return render_target_views[right_index];
    }
};

class SwapChain
{
    friend class Device;
    DXGI_SwapChain swap_chain;
    std::shared_ptr<SwapChainTexture> texture;
	DXGI_SWAP_CHAIN_DESC1 desc;
	swap_chain_desc custom_desc;
	void update_texture()
	{

		DX11_Texture2D native_texture;
		swap_chain->GetBuffer(0, __uuidof(*native_texture), (LPVOID*)&native_texture);
		texture.reset(new SwapChainTexture(native_texture));
	}
public:

	using ptr = std::shared_ptr<SwapChain>;

	SwapChain(DXGI_SwapChain sc, swap_chain_desc  c_desc)
    {
        swap_chain = sc;
		sc->GetDesc1(&desc);
		update_texture();
		custom_desc = c_desc;
    }
	~SwapChain(){
		swap_chain->SetFullscreenState(FALSE, 0);
	}
	bool is_3d_enabled()
	{
		return texture->get_desc().ArraySize == 2;
	}
    std::shared_ptr<SwapChainTexture> get_texture()
    {
        return texture;
    }

	void resize(ivec2 new_size)
	{
		texture = nullptr;
		desc.Width = new_size.x;
		desc.Height = new_size.y;

		swap_chain->ResizeBuffers(desc.BufferCount,
			desc.Width,
			desc.Height,
			desc.Format,
			0);
		update_texture();
	}

    void present()
    {
	/*	RECT r;
		GetClientRect(custom_desc.window->get_hwnd(),&r);
		if (desc.Height==r.bottom-r.top)
		if (desc.Width == r.right - r.left)
		*/
			
			swap_chain->Present(0, DXGI_PRESENT_DO_NOT_WAIT);
    }
};
