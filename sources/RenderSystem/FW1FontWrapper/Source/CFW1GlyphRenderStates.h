// CFW1GlyphRenderStates.h

#ifndef IncludeGuard__FW1_CFW1GlyphRenderStates
#define IncludeGuard__FW1_CFW1GlyphRenderStates

#include "CFW1Object.h"


namespace FW1FontWrapper
{
	using namespace HAL;

    // Shader etc. needed to draw glyphs
    class CFW1GlyphRenderStates : public CFW1Object<IFW1GlyphRenderStates>
    {
        public:
            // IUnknown
            virtual HRESULT STDMETHODCALLTYPE QueryInterface(const IID& riid, void** ppvObject);

            virtual void STDMETHODCALLTYPE SetStates(HAL::CommandList::ptr& list, unsigned int Flags);
            virtual void STDMETHODCALLTYPE UpdateShaderConstants(
                HAL::CommandList::ptr& list,
                const FW1_RECTF* pClipRect,
                const float* pTransformMatrix
            );
            virtual int STDMETHODCALLTYPE HasGeometryShader();

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
                float					TransformMatrix[16];
                float					ClipRect[4];
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
