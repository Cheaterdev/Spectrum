class RenderTargetView
{
	DX11_RenderTargetView view;

public:
	RenderTargetView(Resource * resource, const D3D11_RENDER_TARGET_VIEW_DESC& desc);

	DX11_RenderTargetView operator*() const;
};


class DepthStencilView
{
	DX11_DepthStencilView view;

public:
	DepthStencilView(Resource * resource, const D3D11_DEPTH_STENCIL_VIEW_DESC& desc);

	DX11_DepthStencilView operator*() const;
};