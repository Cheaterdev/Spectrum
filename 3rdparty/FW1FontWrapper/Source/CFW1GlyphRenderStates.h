// CFW1GlyphRenderStates.h

#ifndef IncludeGuard__FW1_CFW1GlyphRenderStates
#define IncludeGuard__FW1_CFW1GlyphRenderStates

#include "CFW1Object.h"


namespace FW1FontWrapper
{
	using namespace Render;

	template <class T>
	struct FontSignature : public T
	{
		using T::T;

		typename T::template ConstBuffer	<0, Render::ShaderVisibility::GEOMETRY, 0>									geometry = this;
		typename T::template Table			<1, Render::ShaderVisibility::GEOMETRY, Render::DescriptorRange::SRV, 0, 1>	geometry_data = this;
		typename T::template Table			<2, Render::ShaderVisibility::PIXEL, Render::DescriptorRange::SRV, 0, 1>	pixel_data = this;
		typename T::template Table			<3, Render::ShaderVisibility::PIXEL, Render::DescriptorRange::SAMPLER, 0, 1>	pixel_sampler = this;

		//constexpr typename T::SRV_2 srv = { 0,  Render::ShaderVisibility::PIXEL, 0_offset };
	};

	using FontSig = SignatureTypes<FontSignature>;

    // Shader etc. needed to draw glyphs
    class CFW1GlyphRenderStates : public CFW1Object<IFW1GlyphRenderStates>, protected DX11::NativeContextAccessor
    {
        public:
            // IUnknown
            virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject);

            virtual void STDMETHODCALLTYPE SetStates(Render::CommandList::ptr& list, UINT Flags);
            virtual void STDMETHODCALLTYPE UpdateShaderConstants(
                Render::CommandList::ptr& list,
                const FW1_RECTF* pClipRect,
                const FLOAT* pTransformMatrix
            );
            virtual BOOL STDMETHODCALLTYPE HasGeometryShader();

            // Public functions
        public:
            CFW1GlyphRenderStates();

            HRESULT initRenderResources(
                IFW1Factory* pFW1Factory,
                bool wantGeometryShader,
                bool anisotropicFiltering
            );

            // Internal types
        private:
            struct ShaderConstants
            {
                FLOAT					TransformMatrix[16];
                FLOAT					ClipRect[4];
            };

            // Internal functions
        private:
            virtual ~CFW1GlyphRenderStates();

            HRESULT createQuadShaders();
            HRESULT createGlyphShaders();
            HRESULT createPixelShaders();
            HRESULT createConstantBuffer();
            HRESULT createRenderStates(bool anisotropicFiltering);

            // Internal data
        private:
            std::wstring				m_lastError;

            D3D_FEATURE_LEVEL			m_featureLevel;


            //     Render::HandleTable const_buffer_table;

            Render::HandleTable pixel_sampler_table;


			FontSig::Pipeline::ptr geometry_state;
			FontSig::Pipeline::ptr geometry_state_clip;

			FontSig::RootSignature::ptr root_signature;

            bool							m_hasGeometryShader;


            //  DX11::sampler_state::ptr		m_pSamplerState;
            /* DX11::blend_state::ptr			m_pBlendState;

             DX11::rasterizer_state::ptr		m_pRasterizerState;
             DX11::depth_stencil_state::ptr	m_pDepthStencilState;*/
    };


}// namespace FW1FontWrapper


#endif// IncludeGuard__FW1_CFW1GlyphRenderStates
