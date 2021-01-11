// CFW1GlyphRenderStates.h

#ifndef IncludeGuard__FW1_CFW1GlyphRenderStates
#define IncludeGuard__FW1_CFW1GlyphRenderStates

#include "CFW1Object.h"
#include "DX12/CommandList.h"


namespace FW1FontWrapper
{
	using namespace Render;

    // Shader etc. needed to draw glyphs
    class CFW1GlyphRenderStates : public CFW1Object<IFW1GlyphRenderStates>
    {
        public:
            // IUnknown
            virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject);

            virtual void STDMETHODCALLTYPE SetStates(DX12::CommandList::ptr& list, UINT Flags);
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
            bool							m_hasGeometryShader;
    };


}// namespace FW1FontWrapper


#endif// IncludeGuard__FW1_CFW1GlyphRenderStates
