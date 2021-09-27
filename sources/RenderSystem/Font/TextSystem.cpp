#include "pch.h"
#include "TextSystem.h"

namespace Fonts
{


    Font::ptr Font::create_new(const std::string& header)
    {
        return FontSystem::get().get_font(header);
    }

    /*   Font::ptr Font::get_resource(const std::string& h)
       {
           return res_manager::get().get_resource(h);
       }
    */
    void Font::draw(Render::CommandList::ptr& command_list, std::wstring str, float size, vec2 pos, float4 color,  unsigned int flags /*= 0*/)
    {

        native_font->DrawString(
            command_list,
            str.c_str(),
            size,
            pos.x,
            pos.y,
            color,
            flags
        );
    }


    void Font::draw(Render::CommandList::ptr& command_list, std::wstring str, float size, sizer area, sizer clip_rect, float4 color, unsigned int flags /*= 0*/)
    {

        native_font->DrawString(
            command_list,
            str.c_str(),
            nullptr,
            size,
            reinterpret_cast<FW1_RECTF*>(&area),
            color,
            reinterpret_cast<FW1_RECTF*>(&clip_rect),
            nullptr,
            flags | FW1_CLIPRECT
        );
    }

    void Font::draw(Render::CommandList::ptr& command_list, std::wstring str, float size, sizer area, float4 color, unsigned int flags /*= 0*/)
    {
        draw(command_list, str, size, area, area, color, flags);
    }




    void Font::draw(Render::CommandList::ptr& command_list, std::string str, float size, vec2 pos, float4 color, unsigned int flags /*=0*/)
    {
        draw(command_list, convert(str), size, pos, color, flags);
    }


    void Font::draw(Render::CommandList::ptr& command_list, std::string str, float size, sizer area, float4 color, unsigned int flags /*=0*/)
    {
        draw(command_list, convert(str), size, area, color, flags);
    }

    void Font::draw(Render::CommandList::ptr& command_list, std::string str, float size, sizer area, sizer clip_rect, float4 color, unsigned int flags /*=0*/)
    {
        draw(command_list, convert(str), size, area, clip_rect, color, flags);
    }

    vec2 Font::measure(std::string str, float size, unsigned int flags /*= 0*/)
    {
        FW1_RECTF inputRect = { 0, 0, 0, 0 };
        // wtf with the sizes....
        FW1_RECTF dimensions = native_font->MeasureString(convert(str).c_str(), convert(font_name).c_str(), size, &inputRect, FW1_NOWORDWRAP);
        return vec2(dimensions.Right /*- dimensions.Left**/, dimensions.Bottom/* - dimensions.Top*/) * 16 / size + vec2(2, 2);
    }

    FontSystem::FontSystem()
    {
        TEST(FW1CreateFactory(FW1_VERSION, &pFW1Factory));
        fonts.create_func = [this](const std::string & font_name)-> Font::ptr
        {
            Font::ptr font(new Font());
            TEST(pFW1Factory->CreateFontWrapper(convert(font_name).c_str(), &font->native_font));
            font->font_name = font_name;
            font->native_font->GetRenderStates(&font->pRenderStates);
            return font;
        };
    }

    Font::ptr FontSystem::get_font(std::string font_name)
    {
        return fonts[font_name];
    }


    vec2 FontGeometry::get_pos(unsigned int index)
    {
        std::lock_guard<std::mutex> guard(m);
        FW1_VERTEXDATA data = geometry->GetGlyphVerticesTemp();

        if (index >= data.TotalVertexCount)
        {
            const FW1_GLYPHVERTEX v = data.pVertices[data.TotalVertexCount - 1];
            return{ v.pos.x + size / 2, v.pos.y - size / 2 };
        }

        const FW1_GLYPHVERTEX v = data.pVertices[index];
        return{ v.pos.x, v.pos.y - size / 2 };
    }

    float FontGeometry::get_size()
    {
        std::lock_guard<std::mutex> guard(m);
        return size;
    }

    unsigned int FontGeometry::get_index(vec2 at)
    {
        std::lock_guard<std::mutex> guard(m);
        unsigned int index = 0;
        float len = 0;
        FW1_VERTEXDATA data = geometry->GetGlyphVerticesTemp();

        for (unsigned int i = 0; i < data.TotalVertexCount; i++)
        {
            const FW1_GLYPHVERTEX v = data.pVertices[i];
            vec2 pos = { v.pos.x, v.pos.y - size / 2 };
            float l = (at - pos).length_squared();

            if (i == 0 || (l < len))
            {
                len = l;
                index = i;
            }
        }

        if (index == data.TotalVertexCount - 1)
        {
            if (data.pVertices[index].pos.x + size / 2 < at.x)
                return index + 1;
        }

        return index;
    }

    void FontGeometry::draw(Render::CommandList::ptr& command_list, sizer clip_rect, unsigned int flags /*= 0*/, vec2 offset /*= {0,0}*/, float scale  /*= 1*/)
    {
        std::lock_guard<std::mutex> guard(m);

        if (!font)
            return;

        mat4x4 m;
        m.identity();
        float w = 1, h = 1;
        auto vps = command_list->get_graphics().get_viewports();

        if (vps.size())
        {
            if (vps[0].Width >= 1.0f && vps[0].Height >= 1.0f)
            {
                w = vps[0].Width;
                h = vps[0].Height;
            }
        }

        m.raw()[0] = scale * 2.0f / w;
        m.raw()[12] = -1.0f + 2 * offset.x / w;
        m.raw()[5] = -2.0f / h * scale;
        m.raw()[13] = 1.0f - 2 * offset.y / h;
        m.raw()[10] = 1.0f;
        m.raw()[15] = 1.0f;
        //   m.raw()[12] =+;
        //   m.raw()[13] = offset.y / h;
        font->native_font->DrawGeometry(
            command_list,
            geometry,
            reinterpret_cast<FW1_RECTF*>(&clip_rect),
            m.raw(),
            flags | FW1_CLIPRECT
        );
    }

    void FontGeometry::set(Render::CommandList::ptr& command_list, std::wstring str, Font::ptr font, float size, sizer area, float4 color, unsigned int flags /*= 0*/)
    {
        std::lock_guard<std::mutex> guard(m);
        this->size = size;
        this->font = font;

        geometry->Clear();
        font->native_font->AnalyzeString(command_list, str.c_str(),
                                         nullptr, size, reinterpret_cast<FW1_RECTF*>(&area),
            color, flags | FW1_CLIPRECT/*| FW1_NOFLUSH*/, geometry);
    }

    FontGeometry::FontGeometry()
    {
        geometry = FontSystem::get().create_geometry();
    }

}
