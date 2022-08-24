// CFW1GlyphAtlas.cpp

#include "pch_dx.h"

#include "CFW1GlyphAtlas.h"

#define SAFE_RELEASE(pObject) { if(pObject) { (pObject)->Release(); (pObject) = NULL; } }


namespace FW1FontWrapper {


// Construct
CFW1GlyphAtlas::CFW1GlyphAtlas() :
	m_sheetWidth(0),
	m_sheetHeight(0),
	m_hardwareCoordBuffer(false),
	m_allowOversizedGlyph(false),
	m_maxGlyphCount(0),
	m_mipLevelCount(0),
	
	m_glyphSheets(0),
	m_sheetCount(0),
	m_maxSheetCount(0),
	m_currentSheetIndex(0),
	m_flushedSheetIndex(0)
{
	InitializeCriticalSection(&m_glyphSheetsCriticalSection);
}


// Destruct
CFW1GlyphAtlas::~CFW1GlyphAtlas() {
		
	for(UINT i=0; i < m_sheetCount; ++i)
		m_glyphSheets[i]->Release();
	delete[] m_glyphSheets;
	
	DeleteCriticalSection(&m_glyphSheetsCriticalSection);
}


// Init
HRESULT CFW1GlyphAtlas::initGlyphAtlas(
	IFW1Factory *pFW1Factory,
	UINT sheetWidth,
	UINT sheetHeight,
	bool coordBuffer,
	bool allowOversizedGlyph,
	UINT maxGlyphCount,
	UINT mipLevelCount,
	UINT maxSheetCount
) {
	HRESULT hResult = initBaseObject(pFW1Factory);
	if(FAILED(hResult))
		return hResult;
	
		m_sheetWidth = sheetWidth;
	m_sheetHeight = sheetHeight;
	m_hardwareCoordBuffer = coordBuffer;
	m_allowOversizedGlyph = allowOversizedGlyph;
	m_mipLevelCount = mipLevelCount;
	m_maxGlyphCount = maxGlyphCount;
	
	m_maxSheetCount = 4096;
	if(maxSheetCount > 0 && maxSheetCount < 655536)
		m_maxSheetCount = maxSheetCount;
	m_glyphSheets = new IFW1GlyphSheet* [m_maxSheetCount];
	
	// Default glyph
	BYTE glyph0Pixels[256];
	FillMemory(glyph0Pixels, 256, 0xff);
	
	FW1_GLYPHMETRICS glyph0Metrics;
	glyph0Metrics.OffsetX = 0.0f;
	glyph0Metrics.OffsetY = 0.0f;
	glyph0Metrics.Width = 16;
	glyph0Metrics.Height = 16;
	
	UINT glyph0 = InsertGlyph(&glyph0Metrics, glyph0Pixels, 16, 1);
	if(glyph0 == 0xffffffff)
		return E_FAIL;
	else
		return S_OK;
}


// Create new glyph sheet
HRESULT CFW1GlyphAtlas::createGlyphSheet(IFW1GlyphSheet **ppGlyphSheet) {
	IFW1GlyphSheet *pGlyphSheet;
	HRESULT hResult = m_pFW1Factory->CreateGlyphSheet(
		m_sheetWidth,
		m_sheetHeight,
		m_hardwareCoordBuffer,
		m_allowOversizedGlyph,
		m_maxGlyphCount,
		m_mipLevelCount,
		&pGlyphSheet
	);
	if(FAILED(hResult)) {
	}
	else {
		*ppGlyphSheet = pGlyphSheet;
		
		hResult = S_OK;
	}
	
	return hResult;
}


}// namespace FW1FontWrapper
