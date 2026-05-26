// CFW1TextRendererInterface.cpp


#include "CFW1TextRenderer.h"


namespace FW1FontWrapper {


// Query interface
HRESULT STDMETHODCALLTYPE CFW1TextRenderer::QueryInterface(const IID& riid, void **ppvObject) {
	if(ppvObject == nullptr)
		return E_INVALIDARG;
	
	if((riid == __uuidof(IDWritePixelSnapping))) {
		*ppvObject = static_cast<IDWritePixelSnapping*>(m_pDWriteTextRendererProxy);
		AddRef();
		return S_OK;
	}
	else if((riid == __uuidof(IDWriteTextRenderer))) {
		*ppvObject = static_cast<IDWriteTextRenderer*>(m_pDWriteTextRendererProxy);
		AddRef();
		return S_OK;
	}
	else if((riid == __uuidof(IFW1TextRenderer))) {
		*ppvObject = static_cast<IFW1TextRenderer*>(this);
		AddRef();
		return S_OK;
	}
	
	return CFW1Object::QueryInterface(riid, ppvObject);
}


// IDWritePixelSnapping method
HRESULT CFW1TextRenderer::IsPixelSnappingDisabled(
	[[maybe_unused]] void *clientDrawingContext,
	BOOL *isDisabled
) {
	
	*isDisabled = false;
	
	return S_OK;
}


// IDWritePixelSnapping method
HRESULT CFW1TextRenderer::GetCurrentTransform(
	[[maybe_unused]] void *clientDrawingContext,
	DWRITE_MATRIX *transform
) {
	
	transform->dx = 0.0f;
	transform->dy = 0.0f;
	transform->m11 = 1.0f;
	transform->m12 = 0.0f;
	transform->m21 = 0.0f;
	transform->m22 = 1.0f;
	
	return S_OK;
}


// IDWritePixelSnapping method
HRESULT CFW1TextRenderer::GetPixelsPerDip([[maybe_unused]] void *clientDrawingContext, FLOAT *pixelsPerDip) {
	
	*pixelsPerDip = 96.0f;
	
	return S_OK;
}


// IDWriteTextRenderer method
// Convert a run of glyphs to vertices
HRESULT CFW1TextRenderer::DrawGlyphRun(
	void *clientDrawingContext,
	FLOAT baselineOriginX,
	FLOAT baselineOriginY,
	[[maybe_unused]] DWRITE_MEASURING_MODE measuringMode,
	const DWRITE_GLYPH_RUN *glyphRun,
	[[maybe_unused]] const DWRITE_GLYPH_RUN_DESCRIPTION *glyphRunDescription,
	IUnknown *clientDrawingEffect
) {
	
	const UINT flags = m_currentFlags;
	
	// Get glyph map for the current font
	const void *glyphMap;
	if(glyphRun->fontFace == m_pCachedGlyphMapFontFace && glyphRun->fontEmSize == m_cachedGlyphMapFontSize)
		glyphMap = m_cachedGlyphMap;
	else {
		glyphMap = m_pGlyphProvider->GetGlyphMapFromFont(glyphRun->fontFace, glyphRun->fontEmSize, flags);
		
		// Cache the glyph map as it's likely to be used in subsequent glyph runs
		m_cachedGlyphMap = glyphMap;
		m_pCachedGlyphMapFontFace = glyphRun->fontFace;
		m_cachedGlyphMapFontSize = glyphRun->fontEmSize;
	}
	
	// Skip if not interested in the actual glyphs
	if((flags & FW1_ANALYZEONLY) != 0)
		return S_OK;
	
	if((flags & FW1_CACHEONLY) != 0) {
		// Only request the glyphs from the provider to have them drawn to the atlas
		for(UINT i=0; i < glyphRun->glyphCount; ++i) {
			UINT atlasId = m_pGlyphProvider->GetAtlasIdFromGlyphIndex(
				glyphMap,
				glyphRun->glyphIndices[i],
				glyphRun->fontFace,
				flags
			);
			atlasId;
		}
	}
	else {
		// Glyph vertex
		FW1_GLYPHVERTEX glyphVertex;
		glyphVertex.pos.y = floor(baselineOriginY + 0.5f);
		glyphVertex.color =  m_currentColor;
		
		float positionX = floor(baselineOriginX + 0.5f);
		
		// Optional drawing effect
		if(clientDrawingEffect != nullptr) {
			IFW1ColorRGBA *pColor;
			HRESULT hResult = clientDrawingEffect->QueryInterface(&pColor);
			if(SUCCEEDED(hResult)) {
				glyphVertex.color = pColor->GetColor();
				pColor->Release();
			}
		}
		
		// Add a vertex for each glyph in the run
		IFW1TextGeometry *pTextGeometry = static_cast<IFW1TextGeometry*>(clientDrawingContext);
		if(pTextGeometry != nullptr) {
			for(UINT i=0; i < glyphRun->glyphCount; ++i) {
				glyphVertex.GlyphIndex = m_pGlyphProvider->GetAtlasIdFromGlyphIndex(
					glyphMap,
					glyphRun->glyphIndices[i],
					glyphRun->fontFace,
					flags
				);
				
				if((glyphRun->bidiLevel & 0x1) != 0)
					positionX -= glyphRun->glyphAdvances[i];
				
				glyphVertex.pos.x = floor(positionX + 0.5f);
				pTextGeometry->AddGlyphVertex(&glyphVertex);
				
				if((glyphRun->bidiLevel & 0x1) == 0)
					positionX += glyphRun->glyphAdvances[i];
			}
		}
	}
	
	return S_OK;
}


// IDWriteTextRenderer method
HRESULT CFW1TextRenderer::DrawUnderline(
	[[maybe_unused]] void *clientDrawingContext,
	[[maybe_unused]] FLOAT baselineOriginX,
	[[maybe_unused]] FLOAT baselineOriginY,
	[[maybe_unused]] const DWRITE_UNDERLINE *underline,
	[[maybe_unused]] IUnknown *clientDrawingEffect
) {
	
	return E_NOTIMPL;
}


// IDWriteTextRenderer method
HRESULT CFW1TextRenderer::DrawStrikethrough(
	[[maybe_unused]] void *clientDrawingContext,
	[[maybe_unused]] FLOAT baselineOriginX,
	[[maybe_unused]] FLOAT baselineOriginY,
	[[maybe_unused]] const DWRITE_STRIKETHROUGH *strikethrough,
	[[maybe_unused]] IUnknown *clientDrawingEffect
) {
	
	return E_NOTIMPL;
}


// IDWriteTextRenderer method
HRESULT CFW1TextRenderer::DrawInlineObject(
	[[maybe_unused]] void *clientDrawingContext,
	[[maybe_unused]] FLOAT originX,
	[[maybe_unused]] FLOAT originY,
	[[maybe_unused]] IDWriteInlineObject *inlineObject,
	[[maybe_unused]] BOOL isSideways,
	[[maybe_unused]] BOOL isRightToLeft,
	[[maybe_unused]] IUnknown *clientDrawingEffect
) {
	
	return E_NOTIMPL;
}


// Get glyph provider
HRESULT STDMETHODCALLTYPE CFW1TextRenderer::GetGlyphProvider(IFW1GlyphProvider **ppGlyphProvider) {
	if(ppGlyphProvider == nullptr)
		return E_INVALIDARG;
	
	m_pGlyphProvider->AddRef();
	*ppGlyphProvider = m_pGlyphProvider;
	
	return S_OK;
}


// Draw a text layout
HRESULT STDMETHODCALLTYPE CFW1TextRenderer::DrawTextLayout(
	IDWriteTextLayout *pTextLayout,
	FLOAT OriginX,
	FLOAT OriginY,
	float4 Color,
	UINT Flags,
	IFW1TextGeometry *pTextGeometry
) {
	m_currentFlags = Flags;
	m_currentColor = Color;
	
	m_cachedGlyphMap = 0;
	m_pCachedGlyphMapFontFace = nullptr;
	m_cachedGlyphMapFontSize = 0.0f;
	
	return pTextLayout->Draw(pTextGeometry, m_pDWriteTextRendererProxy, OriginX, OriginY);
}


}// namespace FW1FontWrapper
