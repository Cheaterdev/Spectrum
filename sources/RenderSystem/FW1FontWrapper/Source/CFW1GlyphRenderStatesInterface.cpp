// CFW1GlyphRenderStatesInterface.cpp

#include <Core_defs.h>
#include "CFW1GlyphRenderStates.h"

namespace FW1FontWrapper
{


// Query interface
    HRESULT STDMETHODCALLTYPE CFW1GlyphRenderStates::QueryInterface(const IID& riid, void** ppvObject)
    {
        if (ppvObject == nullptr)
            return E_INVALIDARG;

        if ((riid == __uuidof(IFW1GlyphRenderStates)))
        {
            *ppvObject = static_cast<IFW1GlyphRenderStates*>(this);
            AddRef();
            return S_OK;
        }

        return CFW1Object::QueryInterface(riid, ppvObject);
    }


// Set render states for glyph drawing
    void STDMETHODCALLTYPE CFW1GlyphRenderStates::SetStates(HAL::CommandList::ptr& list, unsigned int Flags)
    {
        auto formats = list->get_graphics().get_formats();
        ASSERT(formats.size()==1);
        list->get_graphics().set_pipeline<PSOS::FontRender>(PSOS::FontRender::Format(formats[0]));
        list->get_graphics().set_topology(HAL::PrimitiveTopologyType::POINT, HAL::PrimitiveTopologyFeed::LIST);
    }


// Update constant buffer
    void STDMETHODCALLTYPE CFW1GlyphRenderStates::UpdateShaderConstants(
        HAL::CommandList::ptr& list,
        const FW1_RECTF* pClipRect,
        const float* pTransformMatrix
    )
    {
        // Shader constants
        ShaderConstants constants;
        memset(&constants, 0, sizeof(constants));

        // Transform matrix
        if (pTransformMatrix != nullptr)
            memcpy(constants.TransformMatrix, pTransformMatrix, 16 * sizeof(float));
        else
        {
            // Get viewport size for orthographic transform
            float w = 512.0f;
            float h = 512.0f;
            auto vps = list->get_graphics().get_viewports();

            if (vps.size())
            {
                if (vps[0].size.x >= 1.0f && vps[0].size.y >= 1.0f)
                {
                    w = vps[0].size.x;
                    h = vps[0].size.y;
                }
            }

            constants.TransformMatrix[0] = 2.0f / w;
            constants.TransformMatrix[12] = -1.0f;
            constants.TransformMatrix[5] = -2.0f / h;
            constants.TransformMatrix[13] = 1.0f;
            constants.TransformMatrix[10] = 1.0f;
            constants.TransformMatrix[15] = 1.0f;
        }

        // Clip rect
        if (pClipRect != nullptr)
        {
            constants.ClipRect[0] = -pClipRect->Left;
            constants.ClipRect[1] = -pClipRect->Top;
            constants.ClipRect[2] = pClipRect->Right;
            constants.ClipRect[3] = pClipRect->Bottom;
        }

        else
        {
            constants.ClipRect[0] = std::numeric_limits<float>::max();
            constants.ClipRect[1] = std::numeric_limits<float>::max();
            constants.ClipRect[2] = std::numeric_limits<float>::max();
            constants.ClipRect[3] = std::numeric_limits<float>::max();
        }

        Slots::FontRenderingConstants gpu_constants;

        memcpy(&gpu_constants, &constants, sizeof(ShaderConstants));
        list->get_graphics().set(gpu_constants);

	//	list->get_graphics().get_shader_data<FontSig>().geometry.set_raw(constants);
  
    }


// Check for geometry shader
    int STDMETHODCALLTYPE CFW1GlyphRenderStates::HasGeometryShader()
    {
        return m_hasGeometryShader ? 1 : 0;
    }


}// namespace FW1FontWrapper
