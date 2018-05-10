#include <atlbase.h>

typedef CComPtr<ID3D11Asynchronous>			DX11_Asynchronous;
typedef CComPtr<ID3D11BlendState>			DX11_BlendState;
typedef CComPtr<ID3D11Counter>				DX11_Counter;
typedef CComPtr<ID3D11CommandList>			DX11_CommandList;
typedef CComPtr<ID3D11DepthStencilState>	DX11_DepthStencilState;
typedef CComPtr<ID3D11Device>				DX11_Device;
typedef CComPtr<ID3D11DeviceChild>			DX11_DeviceChild;
typedef CComPtr<ID3D11DeviceContext>		DX11_DeviceContext;
typedef CComPtr<ID3D11InputLayout>			DX11_InputLayout;
typedef CComPtr<ID3D11Predicate>			DX11_Predicate;
typedef CComPtr<ID3D11Query>				DX11_Query;
typedef CComPtr<ID3D11RasterizerState>		DX11_RasterizerState;
typedef CComPtr<ID3D11SamplerState>			DX11_SamplerState;
typedef CComPtr<ID3D11Buffer>				DX11_Buffer;
typedef CComPtr<ID3D11Texture1D>			DX11_Texture1D;
typedef CComPtr<ID3D11Texture2D>			DX11_Texture2D;
typedef CComPtr<ID3D11Texture3D>			DX11_Texture3D;
typedef CComPtr<ID3D11DepthStencilView>		DX11_DepthStencilView;
typedef CComPtr<ID3D11RenderTargetView>		DX11_RenderTargetView;
typedef CComPtr<ID3D11ShaderResourceView>	DX11_ShaderResourceView;
typedef CComPtr<ID3D11UnorderedAccessView>	DX11_UnorderedAccessView;
typedef CComPtr<ID3D11View>					DX11_View;
typedef CComPtr<ID3D11ClassInstance>		DX11_ClassInstance;
typedef CComPtr<ID3D11ClassLinkage>			DX11_ClassLinkage;
typedef CComPtr<ID3D11ComputeShader>		DX11_ComputeShader;
typedef CComPtr<ID3D11DomainShader>			DX11_DomainShader;
typedef CComPtr<ID3D11GeometryShader>		DX11_GeometryShader;
typedef CComPtr<ID3D11HullShader>			DX11_HullShader;
typedef CComPtr<ID3D11PixelShader>			DX11_PixelShader;
typedef CComPtr<ID3D11ShaderReflection>		DX11_ShaderReflection;
typedef CComPtr<ID3D11ShaderReflectionConstantBuffer>		DX11_ShaderReflectionConstantBuffer;
typedef CComPtr<ID3D11ShaderReflectionType>					DX11_ShaderReflectionType;
typedef CComPtr<ID3D11ShaderReflectionVariable>				DX11_ShaderReflectionVariable;
typedef CComPtr<ID3D11VertexShader>			DX11_VertexShader;
typedef CComPtr<ID3DBlob> D3D_Blob;

typedef CComPtr<ID3D11Debug>			DX11_Debug;

typedef CComPtr<ID3D11InfoQueue>		DX11_InfoQueue;

typedef CComPtr<IDXGISwapChain1>		DXGI_SwapChain;
typedef CComPtr<IDXGISurface2>			DXGI_Surface;
typedef CComPtr<IDXGIDevice2>			DXGI_Device;
typedef CComPtr<IDXGIAdapter2>			DXGI_Adapter;
typedef CComPtr<IDXGIFactory2>			DXGI_Factory;
typedef CComPtr<IDXGIOutput1>			DXGI_Output;

// serialization

class string_heap : public Singleton<string_heap>
{
        friend class Singleton<string_heap>;

        std::set<std::string> strings;

    public:
        const char* get_string(std::string str)
        {
            auto data = strings.find(str);

            if (data == strings.end())
                data = strings.insert(strings.end(), str);

            return data->c_str();
        }

};

namespace boost
{
    namespace serialization
    {

        template<class Archive>
        void serialize(Archive& ar, D3D11_SHADER_VARIABLE_DESC& g, const unsigned int)
        {
            //	ar & g.DefaultValue;
            ar& g.SamplerSize;
            ar& g.Size;
            ar& g.StartOffset;
            ar& g.StartSampler;
            ar& g.StartTexture;
            string sSemanticName;

            if (Archive::is_saving::value)
                sSemanticName = g.Name;

            ar& sSemanticName;

            if (Archive::is_loading::value)
                g.Name = string_heap::get().get_string(sSemanticName);

            ar& g.TextureSize;
            ar& g.uFlags;
        }



        template<class Archive>
        void serialize(Archive& ar, D3D11_SIGNATURE_PARAMETER_DESC& g, const unsigned int)
        {
            ar& g.ComponentType;
            ar& g.Mask;
            ar& g.MinPrecision;
            ar& g.ReadWriteMask;
            ar& g.Register;
            ar& g.SemanticIndex;
            string sSemanticName;

            if (Archive::is_saving::value)
                sSemanticName = g.SemanticName;

            ar& sSemanticName;

            if (Archive::is_loading::value)
                g.SemanticName = string_heap::get().get_string(sSemanticName);

            ar& g.Stream;
            ar& g.SystemValueType;
        }


        template<class Archive>
        void serialize(Archive& ar, D3D11_SHADER_INPUT_BIND_DESC& g, const unsigned int)
        {
            ar& g.BindCount;
            ar& g.BindPoint;
            ar& g.Dimension;
            //	ar& g.Name;
            string sName;

            if (Archive::is_saving::value)
                sName = g.Name;

            ar& sName;

            if (Archive::is_loading::value)
                g.Name = string_heap::get().get_string(sName);

            ar& g.NumSamples;
            ar& g.ReturnType;
            ar& g.Type;
            ar& g.uFlags;
        }



        template<class Archive>
        void serialize(Archive& ar, D3D11_INPUT_ELEMENT_DESC& g, const unsigned int)
        {
            ar& g.AlignedByteOffset;
            ar& g.Format;
            ar& g.InputSlot;
            ar& g.InputSlotClass;
            ar& g.InstanceDataStepRate;
            ar& g.SemanticIndex;
            string sName;

            if (Archive::is_saving::value)
                sName = g.SemanticName;

            ar& sName;

            if (Archive::is_loading::value)
                g.SemanticName = string_heap::get().get_string(sName);
        }


        template<class Archive>
        void serialize(Archive& ar, D3D11_SHADER_BUFFER_DESC& g, const unsigned int)
        {
            string sName;

            if (Archive::is_saving::value)
                sName = g.Name;

            ar& sName;

            if (Archive::is_loading::value)
                g.Name = string_heap::get().get_string(sName);

            ar& g.Size;
            ar& g.Type;
            ar& g.uFlags;
            ar& g.Variables;
        }

        template<class Archive>
        void serialize(Archive& ar, D3D11_SAMPLER_DESC& g, const unsigned int)
        {
            ar& g.AddressU& g.AddressV& g.AddressW& boost::serialization::make_array(&g.BorderColor[0], 4) &g.ComparisonFunc& g.Filter& g.MaxAnisotropy& g.MaxLOD& g.MinLOD& g.MipLODBias;
        }

        template<class Archive>
        void serialize(Archive& ar, D3D11_DEPTH_STENCIL_DESC& g, const unsigned int)
        {
            ar& g.DepthEnable;
            ar& g.DepthWriteMask;
            ar& g.DepthFunc;
            ar& g.StencilEnable;
            ar& g.StencilReadMask;
            ar& g.StencilWriteMask;
            ar& g.FrontFace.StencilFailOp;
            ar& g.FrontFace.StencilDepthFailOp;
            ar& g.FrontFace.StencilPassOp;
            ar& g.FrontFace.StencilFunc;
            ar& g.BackFace.StencilFailOp;
            ar& g.BackFace.StencilDepthFailOp;
            ar& g.BackFace.StencilPassOp;
            ar& g.BackFace.StencilFunc;
        }

        template<class Archive>
        void serialize(Archive& ar, D3D11_RASTERIZER_DESC& g, const unsigned int)
        {
            ar& g.AntialiasedLineEnable;
            ar& g.CullMode;
            ar& g.DepthBias;
            ar& g.DepthBiasClamp;
            ar& g.DepthClipEnable;
            ar& g.FillMode;
            ar& g.FrontCounterClockwise;
            ar& g.MultisampleEnable;
            ar& g.ScissorEnable;
            ar& g.SlopeScaledDepthBias;
        }

        template<class Archive>
        void serialize(Archive& ar, D3D11_RENDER_TARGET_BLEND_DESC& g, const unsigned int)
        {
            ar& g.BlendEnable;
            ar& g.BlendOp;
            ar& g.BlendOpAlpha;
            ar& g.DestBlend;
            ar& g.DestBlendAlpha;
            ar& g.RenderTargetWriteMask;
            ar& g.SrcBlend;
            ar& g.SrcBlendAlpha;
        }

        void serialize(simple_log_archive& ar, D3D11_BLEND_DESC& g, const unsigned int);

        template<class Archive>
        void serialize(Archive& ar, D3D11_BLEND_DESC& g, const unsigned int)
        {
            ar& g.AlphaToCoverageEnable;
            ar& g.IndependentBlendEnable;
            ar& g.RenderTarget;
        }

    }
}

UINT BitsPerPixel(DXGI_FORMAT fmt);
DXGI_FORMAT to_srv(DXGI_FORMAT);
DXGI_FORMAT to_dsv(DXGI_FORMAT);
UINT get_default_mapping(DXGI_FORMAT);
DXGI_FORMAT to_linear(DXGI_FORMAT);
DXGI_FORMAT to_typeless(DXGI_FORMAT);
#define OP(x,y)\
if (l.x == r.x)\
{\
	y\
}\
else \
	return l.x < r.x;
#define OP_LAST(x,y)\
	return l.x < r.x;

bool operator<(const D3D11_SAMPLER_DESC& l, const D3D11_SAMPLER_DESC& r);

bool operator<(const D3D11_DEPTH_STENCIL_DESC& l, const D3D11_DEPTH_STENCIL_DESC& r);

bool operator<(const D3D11_RASTERIZER_DESC& l, const D3D11_RASTERIZER_DESC& r);

bool operator<(const D3D11_BLEND_DESC& l, const D3D11_BLEND_DESC& r);

bool operator<(const D3D11_RENDER_TARGET_BLEND_DESC& l, const D3D11_RENDER_TARGET_BLEND_DESC& r);
bool operator==(const D3D11_RENDER_TARGET_BLEND_DESC& l, const D3D11_RENDER_TARGET_BLEND_DESC& r);


