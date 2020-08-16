// CFW1FontWrapperInterface.cpp

#include "pch.h"

#include "CFW1FontWrapper.h"

#include "CFW1StateSaver.h"


namespace FW1FontWrapper
{


// Query interface
    HRESULT STDMETHODCALLTYPE CFW1FontWrapper::QueryInterface(REFIID riid, void** ppvObject)
    {
        if (ppvObject == NULL)
            return E_INVALIDARG;

        if (IsEqualIID(riid, __uuidof(IFW1FontWrapper)))
        {
            *ppvObject = static_cast<IFW1FontWrapper*>(this);
            AddRef();
            return S_OK;
        }

        return CFW1Object::QueryInterface(riid, ppvObject);
    }


// Get the factory that created this object
    HRESULT STDMETHODCALLTYPE CFW1FontWrapper::GetFactory(IFW1Factory** ppFactory)
    {
        if (ppFactory == NULL)
            return E_INVALIDARG;

        m_pFW1Factory->AddRef();
        *ppFactory = m_pFW1Factory;
        return S_OK;
    }

// Get DWrite factory
    HRESULT STDMETHODCALLTYPE CFW1FontWrapper::GetDWriteFactory(IDWriteFactory** ppDWriteFactory)
    {
        if (ppDWriteFactory == NULL)
            return E_INVALIDARG;

        m_pDWriteFactory->AddRef();
        *ppDWriteFactory = m_pDWriteFactory;
        return S_OK;
    }


// Get glyph atlas
    HRESULT STDMETHODCALLTYPE CFW1FontWrapper::GetGlyphAtlas(IFW1GlyphAtlas** ppGlyphAtlas)
    {
        if (ppGlyphAtlas == NULL)
            return E_INVALIDARG;

        m_pGlyphAtlas->AddRef();
        *ppGlyphAtlas = m_pGlyphAtlas;
        return S_OK;
    }


// Get glyph provider
    HRESULT STDMETHODCALLTYPE CFW1FontWrapper::GetGlyphProvider(IFW1GlyphProvider** ppGlyphProvider)
    {
        if (ppGlyphProvider == NULL)
            return E_INVALIDARG;

        m_pGlyphProvider->AddRef();
        *ppGlyphProvider = m_pGlyphProvider;
        return S_OK;
    }


// Get render states
    HRESULT STDMETHODCALLTYPE CFW1FontWrapper::GetRenderStates(IFW1GlyphRenderStates** ppRenderStates)
    {
        if (ppRenderStates == NULL)
            return E_INVALIDARG;

        m_pGlyphRenderStates->AddRef();
        *ppRenderStates = m_pGlyphRenderStates;
        return S_OK;
    }


// Get vertex drawer
    HRESULT STDMETHODCALLTYPE CFW1FontWrapper::GetVertexDrawer(IFW1GlyphVertexDrawer** ppVertexDrawer)
    {
        if (ppVertexDrawer == NULL)
            return E_INVALIDARG;

        m_pGlyphVertexDrawer->AddRef();
        *ppVertexDrawer = m_pGlyphVertexDrawer;
        return S_OK;
    }


// Draw text layout
    void STDMETHODCALLTYPE CFW1FontWrapper::DrawTextLayout(
        Render::CommandList::ptr& list,
        IDWriteTextLayout* pTextLayout,
        FLOAT OriginX,
        FLOAT OriginY,
        UINT32 Color,
        UINT Flags
    )
    {
        DrawTextLayout(list, pTextLayout, OriginX, OriginY, Color, NULL, NULL, Flags);
    }


// Draw text layout
    void STDMETHODCALLTYPE CFW1FontWrapper::DrawTextLayout(
        Render::CommandList::ptr& list,
        IDWriteTextLayout* pTextLayout,
        FLOAT OriginX,
        FLOAT OriginY,
        UINT32 Color,
        const FW1_RECTF* pClipRect,
        const FLOAT* pTransformMatrix,
        UINT Flags
    )
    {
        IFW1TextGeometry* pTextGeometry = NULL;

        // If needed, get a text geometry to store vertices in
        if ((Flags & FW1_ANALYZEONLY) == 0 && (Flags & FW1_CACHEONLY) == 0)
        {
            EnterCriticalSection(&m_textGeometriesCriticalSection);

            if (!m_textGeometries.empty())
            {
                pTextGeometry = m_textGeometries.top();
                m_textGeometries.pop();
            }

            LeaveCriticalSection(&m_textGeometriesCriticalSection);

            if (pTextGeometry == NULL)
            {
                IFW1TextGeometry* pNewTextGeometry;
                HRESULT hResult = m_pFW1Factory->CreateTextGeometry(&pNewTextGeometry);

                if (FAILED(hResult))
                {
                }
                else
                    pTextGeometry = pNewTextGeometry;
            }

            if (pTextGeometry != NULL)
                pTextGeometry->Clear();
        }

        // Draw
        AnalyzeTextLayout(list, pTextLayout, OriginX, OriginY, Color, Flags, pTextGeometry);

        if ((Flags & FW1_ANALYZEONLY) == 0 && (Flags & FW1_CACHEONLY) == 0)
            DrawGeometry(list, pTextGeometry, pClipRect, pTransformMatrix, Flags);

        if (pTextGeometry != NULL)
        {
            // Keep the text geometry for future use
            EnterCriticalSection(&m_textGeometriesCriticalSection);
            m_textGeometries.push(pTextGeometry);
            LeaveCriticalSection(&m_textGeometriesCriticalSection);
        }
    }


// Draw text
    void STDMETHODCALLTYPE CFW1FontWrapper::DrawString(
        Render::CommandList::ptr& list,
        const WCHAR* pszString,
        FLOAT FontSize,
        FLOAT X,
        FLOAT Y,
        UINT32 Color,
        UINT Flags
    )
    {
        FW1_RECTF rect;
        rect.Left = rect.Right = X;
        rect.Top = rect.Bottom = Y;
        DrawString(list, pszString, NULL, FontSize, &rect, Color, NULL, NULL, Flags | FW1_NOWORDWRAP);
    }


// Draw text
    void STDMETHODCALLTYPE CFW1FontWrapper::DrawString(
        Render::CommandList::ptr& list,
        const WCHAR* pszString,
        const WCHAR* pszFontFamily,
        FLOAT FontSize,
        FLOAT X,
        FLOAT Y,
        UINT32 Color,
        UINT Flags
    )
    {
        FW1_RECTF rect;
        rect.Left = rect.Right = X;
        rect.Top = rect.Bottom = Y;
        DrawString(list, pszString, pszFontFamily, FontSize, &rect, Color, NULL, NULL, Flags | FW1_NOWORDWRAP);
    }


// Draw text
    void STDMETHODCALLTYPE CFW1FontWrapper::DrawString(
        Render::CommandList::ptr& list,
        const WCHAR* pszString,
        const WCHAR* pszFontFamily,
        FLOAT FontSize,
        const FW1_RECTF* pLayoutRect,
        UINT32 Color,
        const FW1_RECTF* pClipRect,
        const FLOAT* pTransformMatrix,
        UINT Flags
    )
    {
        IDWriteTextLayout* pTextLayout = createTextLayout(pszString, pszFontFamily, FontSize, pLayoutRect, Flags);

        if (pTextLayout != NULL)
        {
            // Draw
            DrawTextLayout(
                list,
                pTextLayout,
                pLayoutRect->Left,
                pLayoutRect->Top,
                Color,
                pClipRect,
                pTransformMatrix,
                Flags
            );
            pTextLayout->Release();
        }
    }


// Measure text
    FW1_RECTF STDMETHODCALLTYPE CFW1FontWrapper::MeasureString(
        const WCHAR* pszString,
        const WCHAR* pszFontFamily,
        FLOAT FontSize,
        const FW1_RECTF* pLayoutRect,
        UINT Flags
    )
    {
        FW1_RECTF stringRect = {pLayoutRect->Left, pLayoutRect->Top, pLayoutRect->Left, pLayoutRect->Top};
        IDWriteTextLayout* pTextLayout = createTextLayout(pszString, pszFontFamily, FontSize, pLayoutRect, Flags);

        if (pTextLayout != NULL)
        {
            // Get measurements
            DWRITE_TEXT_METRICS overhangMetrics;
            HRESULT hResult = pTextLayout->GetMetrics(&overhangMetrics);

            if (SUCCEEDED(hResult))
            {
                stringRect.Left = overhangMetrics.left;// floor(pLayoutRect->Left - overhangMetrics.left);
                stringRect.Top = overhangMetrics.top;// floor(pLayoutRect->Top - overhangMetrics.top);
                stringRect.Right = overhangMetrics.width; //ceil(pLayoutRect->Left + overhangMetrics.right);
                stringRect.Bottom = overhangMetrics.height; //ceil(pLayoutRect->Top + overhangMetrics.bottom);
            }

            pTextLayout->Release();
        }

        return stringRect;
    }


// Create geometry from a string
    void STDMETHODCALLTYPE CFW1FontWrapper::AnalyzeString(
        Render::CommandList::ptr& list,
        const WCHAR* pszString,
        const WCHAR* pszFontFamily,
        FLOAT FontSize,
        const FW1_RECTF* pLayoutRect,
        UINT32 Color,
        UINT Flags,
        IFW1TextGeometry* pTextGeometry
    )
    {
        IDWriteTextLayout* pTextLayout = createTextLayout(pszString, pszFontFamily, FontSize, pLayoutRect, Flags);

        if (pTextLayout != NULL)
        {
            AnalyzeTextLayout(
                list,
                pTextLayout,
                pLayoutRect->Left,
                pLayoutRect->Top,
                Color,
                Flags,
                pTextGeometry
            );
            pTextLayout->Release();
        }
    }


// Create geometry from a text layout
    void STDMETHODCALLTYPE CFW1FontWrapper::AnalyzeTextLayout(
        Render::CommandList::ptr& list,
        IDWriteTextLayout* pTextLayout,
        FLOAT OriginX,
        FLOAT OriginY,
        UINT32 Color,
        UINT Flags,
        IFW1TextGeometry* pTextGeometry
    )
    {
        // Get a text renderer
        IFW1TextRenderer* pTextRenderer = NULL;
        EnterCriticalSection(&m_textRenderersCriticalSection);

        if (!m_textRenderers.empty())
        {
            pTextRenderer = m_textRenderers.top();
            m_textRenderers.pop();
        }

        LeaveCriticalSection(&m_textRenderersCriticalSection);

        if (pTextRenderer == NULL)
        {
            IFW1TextRenderer* pNewTextRenderer;
            HRESULT hResult = m_pFW1Factory->CreateTextRenderer(m_pGlyphProvider, &pNewTextRenderer);

            if (FAILED(hResult))
            {
            }
            else
                pTextRenderer = pNewTextRenderer;
        }

        // Create geometry
        if (pTextRenderer != NULL)
        {
            HRESULT hResult = pTextRenderer->DrawTextLayout(pTextLayout, OriginX, OriginY, Color, Flags, pTextGeometry);

            if (FAILED(hResult))
            {
            }

            // Flush the glyph atlas in case any new glyphs were added
            if ((Flags & FW1_NOFLUSH) == 0)
                m_pGlyphAtlas->Flush(list);

            // Keep the text renderer for future use
            EnterCriticalSection(&m_textRenderersCriticalSection);
            m_textRenderers.push(pTextRenderer);
            LeaveCriticalSection(&m_textRenderersCriticalSection);
        }
    }


// Draw vertices
    void STDMETHODCALLTYPE CFW1FontWrapper::DrawGeometry(
        Render::CommandList::ptr& list,
        IFW1TextGeometry* pGeometry,
        const FW1_RECTF* pClipRect,
        const FLOAT* pTransformMatrix,
        UINT Flags
    )
    {
        FW1_VERTEXDATA vertexData = pGeometry->GetGlyphVerticesTemp();

        if (vertexData.TotalVertexCount > 0 || (Flags & FW1_RESTORESTATE) == 0)
        {
            if (m_featureLevel < D3D_FEATURE_LEVEL_10_0 || m_pGlyphRenderStates->HasGeometryShader() == FALSE)
                Flags |= FW1_NOGEOMETRYSHADER;

            // Save state

            // Set shaders etc.
            if ((Flags & FW1_STATEPREPARED) == 0)
                m_pGlyphRenderStates->SetStates(list, Flags);

            if ((Flags & FW1_CONSTANTSPREPARED) == 0)
                m_pGlyphRenderStates->UpdateShaderConstants(list, pClipRect, pTransformMatrix);

            // Draw glyphs
            UINT temp = m_pGlyphVertexDrawer->DrawVertices(list, m_pGlyphAtlas, &vertexData, Flags, 0xffffffff);
            temp;
        }
    }


// Flush the glyph atlas
    void STDMETHODCALLTYPE CFW1FontWrapper::Flush(Render::CommandList::ptr& list)
    {
        m_pGlyphAtlas->Flush(list);
    }


}// namespace FW1FontWrapper
