// CFW1GlyphVertexDrawer.cpp



#include "CFW1GlyphVertexDrawer.h"

//import HAL;

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
//        m_pVertexBuffer.reset(new HAL::Buffer<FW1_GLYPHVERTEX>(HAL::HeapType::DEFAULT, m_vertexBufferSize / sizeof(FW1_GLYPHVERTEX)));
		std::vector<unsigned int> indices(m_maxIndexCount);

		for (UINT i = 0; i < m_maxIndexCount / 6; ++i)
		{
			indices[i * 6] = static_cast<UINT16>(i * 4);
			indices[i * 6 + 1] = static_cast<UINT16>(i * 4 + 1);
			indices[i * 6 + 2] = static_cast<UINT16>(i * 4 + 2);
			indices[i * 6 + 3] = static_cast<UINT16>(i * 4 + 1);
			indices[i * 6 + 4] = static_cast<UINT16>(i * 4 + 3);
			indices[i * 6 + 5] = static_cast<UINT16>(i * 4 + 2);
		}

		m_pIndexBuffer = HAL::IndexBuffer::make_buffer(indices);
		return S_OK;
	}


	// Draw vertices
	UINT CFW1GlyphVertexDrawer::drawVertices(
		HAL::CommandList::ptr& pContext,
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


			auto data = pContext->place_data(sizeof(FW1_GLYPHVERTEX) * vertexCount, sizeof(FW1_GLYPHVERTEX));

			pContext->write(data, std::span{ vertexData->pVertices + currentVertex, vertexCount });

			auto view = data.resource->create_view<HAL::StructuredBufferView<Table::Glyph>>(*pContext, HAL::StructuredBufferViewDesc{ (uint)data.resource_offset, (uint)data.size, false });


			{
				Slots::FontRenderingGlyphs glyphs;

				glyphs.GetData() = view.structuredBuffer;
				glyphs.set(pContext->get_graphics());

			}


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
		HAL::CommandList::ptr& pContext,
		IFW1GlyphAtlas* pGlyphAtlas,
		const FW1_VERTEXDATA* vertexData,
		UINT preboundSheet
	)
	{
		return preboundSheet;
	}


}// namespace FW1FontWrapper
