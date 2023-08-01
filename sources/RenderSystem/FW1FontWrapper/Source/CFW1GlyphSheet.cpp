// CFW1GlyphSheet.cpp



#include "CFW1GlyphSheet.h"

#define SAFE_RELEASE(pObject) { if(pObject) { (pObject)->Release(); (pObject) = NULL; } }


namespace FW1FontWrapper
{


	// Construct
	CFW1GlyphSheet::CFW1GlyphSheet() :
		m_sheetWidth(0),
		m_sheetHeight(0),
		m_hardwareCoordBuffer(false),
		m_allowOversizedGlyph(false),

		m_textureData(0),
		m_glyphCoords(0),
		m_maxGlyphCount(0),
		m_glyphCount(0),
		m_mipLevelCount(0),
		m_alignWidth(0),

		m_pTexture(NULL),
		
		m_closed(false),
		m_static(false),

		m_heightRange(0),

		m_updatedGlyphCount(0)
	{
		ZeroMemory(&m_dirtyRect, sizeof(m_dirtyRect));
		InitializeCriticalSection(&m_sheetCriticalSection);
		InitializeCriticalSection(&m_flushCriticalSection);
	}


	// Destruct
	CFW1GlyphSheet::~CFW1GlyphSheet()
	{
		delete[] m_textureData;
		delete[] m_glyphCoords;
		delete m_heightRange;
		DeleteCriticalSection(&m_sheetCriticalSection);
		DeleteCriticalSection(&m_flushCriticalSection);
	}


	// Init
	HRESULT CFW1GlyphSheet::initGlyphSheet(
		IFW1Factory* pFW1Factory,
		UINT sheetWidth,
		UINT sheetHeight,
		bool coordBuffer,
		bool allowOversizedGlyph,
		UINT maxGlyphCount,
		UINT mipLevelCount
	)
	{
		HRESULT hResult = initBaseObject(pFW1Factory);

		if (FAILED(hResult))
			return hResult;

		// Sheet metrics
		m_sheetWidth = 512;

		if (sheetWidth > 0)
			m_sheetWidth = sheetWidth;

		m_sheetHeight = 512;

		if (sheetHeight > 0)
			m_sheetHeight = sheetHeight;

		if (coordBuffer)
		{
			/*       D3D_FEATURE_LEVEL featureLevel = DX11::Device::get().get_native_device()->GetFeatureLevel();

				   if (featureLevel >= D3D_FEATURE_LEVEL_10_0)*/
			m_hardwareCoordBuffer = true;
		}

		m_allowOversizedGlyph = allowOversizedGlyph;
		m_maxGlyphCount = 2048;

		if (maxGlyphCount > 0 && maxGlyphCount < 65535)
			m_maxGlyphCount = maxGlyphCount;

		if (mipLevelCount > 1)
		{
			m_mipLevelCount = std::min(mipLevelCount, 5U);// Reasonable mip limit considering borders
			m_alignWidth = (1 << (m_mipLevelCount - 1));
		}

		else  // 0 defaults to 1
		{
			m_mipLevelCount = 1;
			m_alignWidth = 1;
		}

		// Storage
		UINT textureSize = m_sheetWidth * m_sheetHeight;
		UINT mipSize = textureSize;

		for (UINT i = 1; i < m_mipLevelCount; ++i)
		{
			mipSize >>= 2;
			textureSize += mipSize;
		}

		m_textureData = new UINT8[textureSize];
		ZeroMemory(m_textureData, textureSize);
		m_glyphCoords = new FW1_GLYPHCOORDS[m_maxGlyphCount];
		m_heightRange = new HeightRange(m_sheetWidth / m_alignWidth);
		// Device texture/coord-buffer
		hResult = createDeviceResources();

		if (SUCCEEDED(hResult))
			hResult = S_OK;

		return hResult;
	}


	// Create sheet texture and (optionally) coord buffer
	HRESULT CFW1GlyphSheet::createDeviceResources()
	{
		// Create sheet texture
		HAL::ResourceDesc desc = HAL::ResourceDesc::Tex2D(HAL::Format::R8_UNORM, { m_sheetWidth, m_sheetHeight }, 1, m_mipLevelCount);
		m_pTexture.reset(new HAL::Texture(desc));
		m_pCoordBuffer = HAL::StructuredBufferView<FW1_GLYPHCOORDS>(m_maxGlyphCount);
		return S_OK;
	}


	// Height-range helper class, used to fit glyphs in the sheet

	CFW1GlyphSheet::HeightRange::HeightRange(UINT totalWidth) : m_totalWidth(totalWidth)
	{
		m_heights = new UINT[m_totalWidth];
		ZeroMemory(m_heights, m_totalWidth * sizeof(UINT));
	}

	CFW1GlyphSheet::HeightRange::~HeightRange()
	{
		delete[] m_heights;
	}

	UINT CFW1GlyphSheet::HeightRange::findMin(UINT width, UINT* outMin)
	{
		if (width > m_totalWidth)
			width = m_totalWidth;

		UINT currentMax = findMax(0, width);
		UINT currentMin = currentMax;
		UINT minX = 0;

		for (UINT i = 1; i < m_totalWidth - width; ++i)
		{
			if (m_heights[i + width - 1] >= currentMax)
				currentMax = m_heights[i + width - 1];
			else if (m_heights[i - 1] == currentMax)
			{
				currentMax = findMax(i, width);

				if (currentMax < currentMin)
				{
					currentMin = currentMax;
					minX = i;
				}
			}
		}

		*outMin = currentMin;
		return minX;
	}

	void CFW1GlyphSheet::HeightRange::update(UINT startX, UINT width, UINT newHeight)
	{
		if (width > m_totalWidth)
			width = m_totalWidth;

		for (UINT i = 0; i < width; ++i)
			m_heights[startX + i] = newHeight;
	}

	UINT CFW1GlyphSheet::HeightRange::findMax(UINT startX, UINT width)
	{
		UINT currentMax = m_heights[startX];

		for (UINT i = 1; i < width; ++i)
			currentMax = std::max(currentMax, m_heights[startX + i]);

		return currentMax;
	}


}// namespace FW1FontWrapper
