class ShaderResourceView
{
	DX11_ShaderResourceView view;

public:
	ShaderResourceView(Resource * resource, const D3D11_SHADER_RESOURCE_VIEW_DESC& desc);
	ShaderResourceView() = default;
	DX11_ShaderResourceView operator*() const;
};