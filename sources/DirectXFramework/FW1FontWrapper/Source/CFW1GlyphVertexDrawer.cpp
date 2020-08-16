// CFW1GlyphVertexDrawer.cpp

#include "pch.h"

#include "CFW1GlyphVertexDrawer.h"

#define SAFE_RELEASE(pObject) { if(pObject) { (pObject)->Release(); (pObject) = NULL; } }


namespace FW1FontWrapper
{


    // Construct
    CFW1GlyphVertexDrawer::CFW1GlyphVertexDrawer() :
        m_vertexBufferSize(0),
        m_maxIndexCount(0)
    {
    }


    // Destruct
    CFW1GlyphVertexDrawer::~CFW1GlyphVertexDrawer()
    {
    }
    // Init
    HRESULT CFW1GlyphVertexDrawer::initVertexDrawer(
        IFW1Factory* pFW1Factory,
        UINT vertexBufferSize
    )
    {
        HRESULT hResult = initBaseObject(pFW1Factory);

        if (FAILED(hResult))
            return hResult;

        //  D3D_FEATURE_LEVEL featureLevel = DX11::Device::get().get_native_device()->GetFeatureLevel();
        m_vertexBufferSize = 4096 * 16;

        if (vertexBufferSize >= 1024)
        {
            //  if (featureLevel < D3D_FEATURE_LEVEL_9_2)
            //     vertexBufferSize = std::min(vertexBufferSize, 512U * 1024U);
            m_vertexBufferSize = vertexBufferSize;
        }

        m_maxIndexCount = (m_vertexBufferSize * 3) / (2 * sizeof(QuadVertex));

        if (m_maxIndexCount < 64)
            m_maxIndexCount = 64;

        // Create device buffers
        hResult = createBuffers();

        if (SUCCEEDED(hResult))
            hResult = S_OK;

        return hResult;
    }


    // Create vertex/index buffers
    HRESULT CFW1GlyphVertexDrawer::createBuffers()
    {
        // Create vertex buffer
//        m_pVertexBuffer.reset(new Render::Buffer<FW1_GLYPHVERTEX>(Render::HeapType::DEFAULT, m_vertexBufferSize / sizeof(FW1_GLYPHVERTEX)));
        std::vector<unsigned short> indices(m_maxIndexCount);

        for (UINT i = 0; i < m_maxIndexCount / 6; ++i)
        {
            indices[i * 6] = static_cast<UINT16>(i * 4);
            indices[i * 6 + 1] = static_cast<UINT16>(i * 4 + 1);
            indices[i * 6 + 2] = static_cast<UINT16>(i * 4 + 2);
            indices[i * 6 + 3] = static_cast<UINT16>(i * 4 + 1);
            indices[i * 6 + 4] = static_cast<UINT16>(i * 4 + 3);
            indices[i * 6 + 5] = static_cast<UINT16>(i * 4 + 2);
        }

        m_pIndexBuffer.reset(new  Render::IndexBuffer(indices));
        return S_OK;
    }


    // Draw vertices
    UINT CFW1GlyphVertexDrawer::drawVertices(
        Render::CommandList::ptr& pContext,
        IFW1GlyphAtlas* pGlyphAtlas,
        const FW1_VERTEXDATA* vertexData,
        UINT preboundSheet
    )
    {
        if (vertexData->SheetCount == 0 || vertexData->TotalVertexCount == 0)
            return preboundSheet;

        UINT maxVertexCount = m_vertexBufferSize / sizeof(FW1_GLYPHVERTEX);
        UINT currentSheet = 0;
        UINT activeSheet = preboundSheet;
        UINT currentVertex = 0;
        UINT nextSheetStart = vertexData->pVertexCounts[0];
        //  m_pVertexBuffer->set_data(vertexData->pVertices, vertexData->TotalVertexCount);
        //  m_pVertexBuffer->update(pContext);

        while (currentVertex < vertexData->TotalVertexCount)
        {
            // Fill the vertex buffer
            UINT vertexCount = std::min(vertexData->TotalVertexCount - currentVertex, maxVertexCount);
            //   m_pVertexBuffer->set_data(pContext, 0, vertexData->pVertices + currentVertex, vertexCount);
            //   pContext->set_vertex_buffers(0, m_pVertexBuffer);
            std::vector<D3D12_VERTEX_BUFFER_VIEW> vbs;
            vbs.emplace_back(pContext->get_graphics().place_vertex_buffer(vertexData->pVertices + currentVertex, vertexCount));
            pContext->get_graphics().set_vertex_buffers(0, vbs);
            //     pContext->update_buffer(m_pVertexBuffer->get_resource(), 0, reinterpret_cast<const char*>(vertexData->pVertices + currentVertex), vertexCount);
            // Draw all glyphs in the buffer
            UINT drawnVertices = 0;

            while (drawnVertices < vertexCount)
            {
                while (currentVertex >= nextSheetStart)
                {
                    ++currentSheet;
                    nextSheetStart += vertexData->pVertexCounts[currentSheet];
                }

                if (currentSheet != activeSheet)
                {
                    // Bind sheet shader resources
                    pGlyphAtlas->BindSheet(pContext, currentSheet, 0);
                    activeSheet = currentSheet;
                }

                UINT drawCount = std::min(vertexCount - drawnVertices, nextSheetStart - currentVertex);
                pContext->get_graphics().draw(drawCount, drawnVertices);
                //    pContext.get_native()->Draw(drawCount, drawnVertices);
                drawnVertices += drawCount;
                currentVertex += drawCount;
            }
        }

        return activeSheet;
    }


    // Draw vertices as quads
    UINT CFW1GlyphVertexDrawer::drawGlyphsAsQuads(
        Render::CommandList::ptr& pContext,
        IFW1GlyphAtlas* pGlyphAtlas,
        const FW1_VERTEXDATA* vertexData,
        UINT preboundSheet
    )
    {
        if (vertexData->SheetCount == 0 || vertexData->TotalVertexCount == 0)
            return preboundSheet;

        UINT maxVertexCount = m_vertexBufferSize / sizeof(QuadVertex);

        if (maxVertexCount > 4 * (m_maxIndexCount / 6))
            maxVertexCount = 4 * (m_maxIndexCount / 6);

        if (maxVertexCount % 4 != 0)
            maxVertexCount -= (maxVertexCount % 4);

        UINT currentSheet = 0;
        UINT activeSheet = preboundSheet;
        const FW1_GLYPHCOORDS* sheetGlyphCoords = 0;

        if (activeSheet < vertexData->SheetCount)
            sheetGlyphCoords = pGlyphAtlas->GetGlyphCoords(activeSheet);

        UINT currentVertex = 0;
        UINT nextSheetStart = vertexData->pVertexCounts[0];

        while (currentVertex < vertexData->TotalVertexCount)
        {
            // Fill the vertex buffer
            UINT vertexCount = std::min((vertexData->TotalVertexCount - currentVertex) * 4, maxVertexCount);
            QuadVertex* bufferVertices = 0;//reinterpret_cast<QuadVertex*>(m_pVertexBuffer->data());
            // Convert to quads when filling the buffer
            UINT savedCurrentSheet = currentSheet;
            UINT savedActiveSheet = activeSheet;
            UINT savedNextSheetStart = nextSheetStart;
            UINT savedCurrentVertex = currentVertex;
            UINT drawnVertices = 0;

            while (drawnVertices < vertexCount)
            {
                while (currentVertex >= nextSheetStart)
                {
                    ++currentSheet;
                    nextSheetStart += vertexData->pVertexCounts[currentSheet];
                }

                if (currentSheet != activeSheet)
                {
                    sheetGlyphCoords = pGlyphAtlas->GetGlyphCoords(currentSheet);
                    activeSheet = currentSheet;
                }

                UINT drawCount = std::min(vertexCount - drawnVertices, (nextSheetStart - currentVertex) * 4);

                for (UINT i = 0; i < drawCount / 4; ++i)
                {
                    FW1_GLYPHVERTEX glyphVertex = vertexData->pVertices[currentVertex + i];
                    glyphVertex.PositionX = glyphVertex.PositionX;
                    glyphVertex.PositionY = glyphVertex.PositionY;
                    const FW1_GLYPHCOORDS& glyphCoords = sheetGlyphCoords[glyphVertex.GlyphIndex];
                    QuadVertex quadVertex;
                    quadVertex.color = glyphVertex.GlyphColor;
                    quadVertex.positionX = glyphVertex.PositionX + glyphCoords.PositionLeft;
                    quadVertex.positionY = glyphVertex.PositionY + glyphCoords.PositionTop;
                    quadVertex.texCoordX = glyphCoords.TexCoordLeft;
                    quadVertex.texCoordY = glyphCoords.TexCoordTop;
                    bufferVertices[drawnVertices + i * 4 + 0] = quadVertex;
                    quadVertex.positionX = glyphVertex.PositionX + glyphCoords.PositionRight;
                    quadVertex.texCoordX = glyphCoords.TexCoordRight;
                    bufferVertices[drawnVertices + i * 4 + 1] = quadVertex;
                    quadVertex.positionY = glyphVertex.PositionY + glyphCoords.PositionBottom;
                    quadVertex.texCoordY = glyphCoords.TexCoordBottom;
                    bufferVertices[drawnVertices + i * 4 + 3] = quadVertex;
                    quadVertex.positionX = glyphVertex.PositionX + glyphCoords.PositionLeft;
                    quadVertex.texCoordX = glyphCoords.TexCoordLeft;
                    bufferVertices[drawnVertices + i * 4 + 2] = quadVertex;
                }

                drawnVertices += drawCount;
                currentVertex += drawCount / 4;
            }

            // m_pVertexBuffer->update(pContext);
            // Draw all glyphs in the buffer
            currentSheet = savedCurrentSheet;
            activeSheet = savedActiveSheet;
            nextSheetStart = savedNextSheetStart;
            currentVertex = savedCurrentVertex;
            drawnVertices = 0;

            while (drawnVertices < vertexCount)
            {
                while (currentVertex >= nextSheetStart)
                {
                    ++currentSheet;
                    nextSheetStart += vertexData->pVertexCounts[currentSheet];
                }

                if (currentSheet != activeSheet)
                {
                    // Bind sheet shader resources
                    pGlyphAtlas->BindSheet(pContext, currentSheet, FW1_NOGEOMETRYSHADER);
                    activeSheet = currentSheet;
                }

                UINT drawCount = std::min(vertexCount - drawnVertices, (nextSheetStart - currentVertex) * 4);
                //     pContext.draw_indexed((drawCount / 2) * 3, 0, drawnVertices);
                //  pContext.get_native()->DrawIndexed((drawCount / 2) * 3, 0, drawnVertices);
                drawnVertices += drawCount;
                currentVertex += drawCount / 4;
            }
        }

        return activeSheet;
    }


}// namespace FW1FontWrapper
