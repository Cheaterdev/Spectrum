// CFW1GlyphRenderStates.h

#ifndef IncludeGuard__FW1_CFW1GlyphRenderStates
#define IncludeGuard__FW1_CFW1GlyphRenderStates

#include "CFW1Object.h"


namespace FW1FontWrapper
{
	using namespace Render;

    // Shader etc. needed to draw glyphs
    class CFW1GlyphRenderStates : public CFW1Object<IFW1GlyphRenderStates>
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

         //   Render::HandleTable pixel_sampler_table;


			PipelineState::ptr geometry_state;
            PipelineState::ptr geometry_state_clip;

			//FontSig::RootSignature::ptr root_signature;

            bool							m_hasGeometryShader;


            //  DX11::sampler_state::ptr		m_pSamplerState;
            /* DX11::blend_state::ptr			m_pBlendState;

             DX11::rasterizer_state::ptr		m_pRasterizerState;
             DX11::depth_stencil_state::ptr	m_pDepthStencilState;*/
    };


}// namespace FW1FontWrapper


#endif// IncludeGuard__FW1_CFW1GlyphRenderStates
