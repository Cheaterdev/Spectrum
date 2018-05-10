// CFW1GlyphRenderStatesInterface.cpp

#include "pch.h"

#include "CFW1GlyphRenderStates.h"


namespace FW1FontWrapper
{


// Query interface
    HRESULT STDMETHODCALLTYPE CFW1GlyphRenderStates::QueryInterface(REFIID riid, void** ppvObject)
    {
        if (ppvObject == NULL)
            return E_INVALIDARG;

        if (IsEqualIID(riid, __uuidof(IFW1GlyphRenderStates)))
        {
            *ppvObject = static_cast<IFW1GlyphRenderStates*>(this);
            AddRef();
            return S_OK;
        }

        return CFW1Object::QueryInterface(riid, ppvObject);
    }


// Set render states for glyph drawing
    void STDMETHODCALLTYPE CFW1GlyphRenderStates::SetStates(Render::CommandList::ptr& list, UINT Flags)
    {
        if ((Flags & FW1_CLIPRECT) != 0)
            list->get_graphics().set_pipeline(geometry_state_clip);
        else
            list->get_graphics().set_pipeline(geometry_state);

        list->get_graphics().set_topology(D3D_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
        //  list->set(0, const_buffer_table);
//           list->set(1, geometry_buffer_table);
        //       list->set(2, pixel_texture_table);
        list->get_graphics().set(3, pixel_sampler_table);
    }


// Update constant buffer
    void STDMETHODCALLTYPE CFW1GlyphRenderStates::UpdateShaderConstants(
        Render::CommandList::ptr& list,
        const FW1_RECTF* pClipRect,
        const FLOAT* pTransformMatrix
    )
    {
        // Shader constants
        ShaderConstants constants;
        ZeroMemory(&constants, sizeof(constants));

        // Transform matrix
        if (pTransformMatrix != NULL)
            CopyMemory(constants.TransformMatrix, pTransformMatrix, 16 * sizeof(FLOAT));
        else
        {
            // Get viewport size for orthographic transform
            FLOAT w = 512.0f;
            FLOAT h = 512.0f;
            auto& vps = list->get_graphics().get_viewports();

            if (vps.size())
            {
                if (vps[0].Width >= 1.0f && vps[0].Height >= 1.0f)
                {
                    w = vps[0].Width;
                    h = vps[0].Height;
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
        if (pClipRect != NULL)
        {
            constants.ClipRect[0] = -pClipRect->Left;
            constants.ClipRect[1] = -pClipRect->Top;
            constants.ClipRect[2] = pClipRect->Right;
            constants.ClipRect[3] = pClipRect->Bottom;
        }

        else
        {
            constants.ClipRect[0] = FLT_MAX;
            constants.ClipRect[1] = FLT_MAX;
            constants.ClipRect[2] = FLT_MAX;
            constants.ClipRect[3] = FLT_MAX;
        }

        list->get_graphics().set_const_buffer(0, constants);
        //  (*m_pConstantBuffer) = constants;
        //  m_pConstantBuffer->update(list);
    }


// Check for geometry shader
    BOOL STDMETHODCALLTYPE CFW1GlyphRenderStates::HasGeometryShader()
    {
        return (m_hasGeometryShader ? TRUE : FALSE);
    }


}// namespace FW1FontWrapper
