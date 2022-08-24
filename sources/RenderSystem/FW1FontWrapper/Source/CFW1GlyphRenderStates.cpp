// CFW1GlyphRenderStates.cpp

#include "pch_dx.h"

#include "CFW1GlyphRenderStates.h"

#define SAFE_RELEASE(pObject) { if(pObject) { (pObject)->Release(); (pObject) = NULL; } }


namespace FW1FontWrapper
{



    // Construct
    CFW1GlyphRenderStates::CFW1GlyphRenderStates() :
        m_featureLevel(D3D_FEATURE_LEVEL_9_1),

        m_hasGeometryShader(false)

    {
    }


    // Destruct
    CFW1GlyphRenderStates::~CFW1GlyphRenderStates()
    {
    }


    // Init
    HRESULT CFW1GlyphRenderStates::initRenderResources(
        IFW1Factory* pFW1Factory,
        bool wantGeometryShader,
        bool anisotropicFiltering
    )
    {
        HRESULT hResult = initBaseObject(pFW1Factory);

		if (FAILED(hResult))
			return hResult;

        m_featureLevel = D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_11_1;

 
        return hResult;
    }


    // Create quad shaders
    HRESULT CFW1GlyphRenderStates::createQuadShaders()
    {
        return S_OK;
    }

    // Create point to quad geometry shader
    HRESULT CFW1GlyphRenderStates::createGlyphShaders()
    {
        if (m_featureLevel < D3D_FEATURE_LEVEL_10_0)
            return E_FAIL;

        m_hasGeometryShader = true;
        return S_OK;
    }

    // Create pixel shaders
    HRESULT CFW1GlyphRenderStates::createPixelShaders()
    {
        return S_OK;
    }


    // Create constant buffer
    HRESULT CFW1GlyphRenderStates::createConstantBuffer()
    {
        return S_OK;
    }


    // Create render states
    HRESULT CFW1GlyphRenderStates::createRenderStates(bool anisotropicFiltering)
    {
        return S_OK;
    }
}// namespace FW1FontWrapper
