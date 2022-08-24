// CFW1FontWrapper.h

#ifndef IncludeGuard__FW1_CFW1FontWrapper
#define IncludeGuard__FW1_CFW1FontWrapper

#include "CFW1Object.h"


namespace FW1FontWrapper
{


// Font-wrapper simplifying drawing strings and text-layouts
    class CFW1FontWrapper : public CFW1Object<IFW1FontWrapper>
    {
        public:
            // IUnknown
            virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject);

            // IFW1FontWrapper
            virtual HRESULT STDMETHODCALLTYPE GetFactory(IFW1Factory** ppFactory);

            virtual HRESULT STDMETHODCALLTYPE GetDWriteFactory(IDWriteFactory** ppDWriteFactory);
            virtual HRESULT STDMETHODCALLTYPE GetGlyphAtlas(IFW1GlyphAtlas** ppGlyphAtlas);
            virtual HRESULT STDMETHODCALLTYPE GetGlyphProvider(IFW1GlyphProvider** ppGlyphProvider);
            virtual HRESULT STDMETHODCALLTYPE GetRenderStates(IFW1GlyphRenderStates** ppRenderStates);
            virtual HRESULT STDMETHODCALLTYPE GetVertexDrawer(IFW1GlyphVertexDrawer** ppVertexDrawer);

            virtual void STDMETHODCALLTYPE DrawTextLayout(
                Graphics::CommandList::ptr& list,
                IDWriteTextLayout* pTextLayout,
                FLOAT OriginX,
                FLOAT OriginY,
                float4 Color,
                UINT Flags
            );
            virtual void STDMETHODCALLTYPE DrawTextLayout(
                Graphics::CommandList::ptr& list,
                IDWriteTextLayout* pTextLayout,
                FLOAT OriginX,
                FLOAT OriginY,
                float4 Color,
                const FW1_RECTF* pClipRect,
                const FLOAT* pTransformMatrix,
                UINT Flags
            );

            virtual void STDMETHODCALLTYPE DrawString(
                Graphics::CommandList::ptr& list,
                const WCHAR* pszString,
                FLOAT FontSize,
                FLOAT X,
                FLOAT Y,
                float4 Color,
                UINT Flags
            );
            virtual void STDMETHODCALLTYPE DrawString(
                Graphics::CommandList::ptr& list,
                const WCHAR* pszString,
                const WCHAR* pszFontFamily,
                FLOAT FontSize,
                FLOAT X,
                FLOAT Y,
                float4 Color,
                UINT Flags
            );
            virtual void STDMETHODCALLTYPE DrawString(
                Graphics::CommandList::ptr& list,
                const WCHAR* pszString,
                const WCHAR* pszFontFamily,
                FLOAT FontSize,
                const FW1_RECTF* pLayoutRect,
                float4 Color,
                const FW1_RECTF* pClipRect,
                const FLOAT* pTransformMatrix,
                UINT Flags
            );

            virtual FW1_RECTF STDMETHODCALLTYPE MeasureString(
                const WCHAR* pszString,
                const WCHAR* pszFontFamily,
                FLOAT FontSize,
                const FW1_RECTF* pLayoutRect,
                UINT Flags
            );

            virtual void STDMETHODCALLTYPE AnalyzeString(
                Graphics::CommandList::ptr& list,
                const WCHAR* pszString,
                const WCHAR* pszFontFamily,
                FLOAT FontSize,
                const FW1_RECTF* pLayoutRect,
                float4 Color,
                UINT Flags,
                IFW1TextGeometry* pTextGeometry
            );

            virtual void STDMETHODCALLTYPE AnalyzeTextLayout(
                Graphics::CommandList::ptr& list,
                IDWriteTextLayout* pTextLayout,
                FLOAT OriginX,
                FLOAT OriginY,
                float4 Color,
                UINT Flags,
                IFW1TextGeometry* pTextGeometry
            );

            virtual void STDMETHODCALLTYPE DrawGeometry(
                Graphics::CommandList::ptr& list,
                IFW1TextGeometry* pGeometry,
                const FW1_RECTF* pClipRect,
                const FLOAT* pTransformMatrix,
                UINT Flags
            );

            virtual void STDMETHODCALLTYPE Flush(Graphics::CommandList::ptr& list);

            // Public functions
        public:
            CFW1FontWrapper();

            HRESULT initFontWrapper(
                IFW1Factory* pFW1Factory,
                IFW1GlyphAtlas* pGlyphAtlas,
                IFW1GlyphProvider* pGlyphProvider,
                IFW1GlyphVertexDrawer* pGlyphVertexDrawer,
                IFW1GlyphRenderStates* pGlyphRenderStates,
                IDWriteFactory* pDWriteFactory,
                const FW1_DWRITEFONTPARAMS* pDefaultFontParams
            );

            // Internal functions
        private:
            virtual ~CFW1FontWrapper();

            IDWriteTextLayout* createTextLayout(
                const WCHAR* pszString,
                const WCHAR* pszFontFamily,
                FLOAT fontSize,
                const FW1_RECTF* pLayoutRect,
                UINT flags
            );

            // Internal data
        private:
            std::wstring					m_lastError;

            D3D_FEATURE_LEVEL				m_featureLevel;
            IDWriteFactory*					m_pDWriteFactory;

            IFW1GlyphAtlas*					m_pGlyphAtlas;
            IFW1GlyphProvider*				m_pGlyphProvider;

            IFW1GlyphRenderStates*			m_pGlyphRenderStates;
            IFW1GlyphVertexDrawer*			m_pGlyphVertexDrawer;

            CRITICAL_SECTION				m_textRenderersCriticalSection;
            std::stack<IFW1TextRenderer*>	m_textRenderers;
            CRITICAL_SECTION				m_textGeometriesCriticalSection;
            std::stack<IFW1TextGeometry*>	m_textGeometries;

            bool							m_defaultTextInited;
            IDWriteTextFormat*				m_pDefaultTextFormat;
    };


}// namespace FW1FontWrapper


#endif// IncludeGuard__FW1_CFW1FontWrapper
