// CFW1GlyphVertexDrawerInterface.cpp

#include "pch_dx.h"

#include "CFW1GlyphVertexDrawer.h"


namespace FW1FontWrapper
{


// Query interface
    HRESULT STDMETHODCALLTYPE CFW1GlyphVertexDrawer::QueryInterface(REFIID riid, void** ppvObject)
    {
        if (ppvObject == NULL)
            return E_INVALIDARG;

        if (IsEqualIID(riid, __uuidof(IFW1GlyphVertexDrawer)))
        {
            *ppvObject = static_cast<IFW1GlyphVertexDrawer*>(this);
            AddRef();
            return S_OK;
        }

        return CFW1Object::QueryInterface(riid, ppvObject);
    }


// Draw vertices
    UINT STDMETHODCALLTYPE CFW1GlyphVertexDrawer::DrawVertices(
        Graphics::CommandList::ptr& pContext,
        IFW1GlyphAtlas* pGlyphAtlas,
        const FW1_VERTEXDATA* pVertexData,
        UINT Flags,
        UINT PreboundSheet
    )
    {
        UINT stride;
        UINT offset = 0;

        if ((Flags & FW1_NOGEOMETRYSHADER) == 0)
            stride = sizeof(FW1_GLYPHVERTEX);
        else
        {
            stride = sizeof(QuadVertex);

            if ((Flags & FW1_BUFFERSPREPARED) == 0)
                pContext->get_graphics().set_index_buffer(m_pIndexBuffer->get_index_buffer_view(true));
        }

        //   if ((Flags & FW1_BUFFERSPREPARED) == 0)
        //     pContext->get_graphics().set_vertex_buffers(0, m_pVertexBuffer);

        if ((Flags & FW1_NOGEOMETRYSHADER) == 0)
            return drawVertices(pContext, pGlyphAtlas, pVertexData, PreboundSheet);
        else
            return drawGlyphsAsQuads(pContext, pGlyphAtlas, pVertexData, PreboundSheet);
    }


}// namespace FW1FontWrapper
