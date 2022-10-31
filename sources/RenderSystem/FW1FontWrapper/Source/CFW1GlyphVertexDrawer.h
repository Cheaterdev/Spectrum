// CFW1GlyphVertexDrawer.h

#ifndef IncludeGuard__FW1_CFW1GlyphVertexDrawer
#define IncludeGuard__FW1_CFW1GlyphVertexDrawer

#include "CFW1Object.h"
import HAL;


namespace FW1FontWrapper
{


// Draws glyph-vertices from system memory using a dynamic vertex buffer
    class CFW1GlyphVertexDrawer : public CFW1Object<IFW1GlyphVertexDrawer>
    {
        public:
            // IUnknown
            virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject);


            virtual UINT STDMETHODCALLTYPE DrawVertices(
                HAL::CommandList::ptr& pContext,
                IFW1GlyphAtlas* pGlyphAtlas,
                const FW1_VERTEXDATA* pVertexData,
                UINT Flags,
                UINT PreboundSheet
            );

            // Public functions
        public:
            CFW1GlyphVertexDrawer();

            HRESULT initVertexDrawer(IFW1Factory* pFW1Factory, UINT vertexBufferSize);

            // Internal types
        private:
            struct QuadVertex
            {
                FLOAT						positionX;
                FLOAT						positionY;
                FLOAT						texCoordX;
                FLOAT						texCoordY;
                float4						color;
            };

            // Internal functions
        private:
            virtual ~CFW1GlyphVertexDrawer();

            HRESULT createBuffers();

            UINT drawVertices(
                HAL::CommandList::ptr& pContext,
                IFW1GlyphAtlas* pGlyphAtlas,
                const FW1_VERTEXDATA* vertexData,
                UINT preboundSheet
            );
            UINT drawGlyphsAsQuads(
                HAL::CommandList::ptr& pContext,
                IFW1GlyphAtlas* pGlyphAtlas,
                const FW1_VERTEXDATA* vertexData,
                UINT preboundSheet
            );

            // Internal data
        private:
            std::wstring					m_lastError;

            //  HAL::Buffer<FW1_GLYPHVERTEX>::ptr m_pVertexBuffer;
            HAL::IndexBuffer::ptr m_pIndexBuffer;

            UINT							m_vertexBufferSize;
            UINT							m_maxIndexCount;
    };


}// namespace FW1FontWrapper


#endif// IncludeGuard__FW1_CFW1GlyphVertexDrawer
