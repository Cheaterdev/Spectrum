#pragma once
#include "FW1FontWrapper/Source/FW1FontWrapper.h"
typedef ComPtr<IFW1Factory>			FW1_Factory;
typedef ComPtr<IFW1FontWrapper>		FW1_Font;
typedef ComPtr<IFW1TextGeometry>		FW1_TextGeometry;


import Data;
import Utils;

namespace Fonts
{
    class FontSystem;
    class FontGeometry;
    class Font
    {
            //typedef resource_manager_base<Font, std::string, Font> res_manager;
            friend class FontSystem;
            friend class FontGeometry;

            friend class res_manager;

            FW1_Font native_font;
            std::string font_name;
        public:
            ~Font()
            {
                pRenderStates->Release();
            }
            using ptr = s_ptr<Font>;

            //      static Font::ptr get_resource(const std::string& h);

            void draw(Graphics::CommandList::ptr& list, std::string str, float size, vec2 pos, float4 color, unsigned int flags = 0);
            void draw(Graphics::CommandList::ptr& list, std::wstring str, float size, vec2 pos, float4 color, unsigned int flags = 0);

            void draw(Graphics::CommandList::ptr& list, std::string str, float size, sizer area, float4 color, unsigned int flags = 0);
            void draw(Graphics::CommandList::ptr& list, std::wstring str, float size, sizer area, float4 color, unsigned int flags = 0);

            void draw(Graphics::CommandList::ptr&, std::string str, float size, sizer area, sizer clip_rect, float4 color, unsigned int flags = 0);
            void draw(Graphics::CommandList::ptr&, std::wstring str, float size, sizer area, sizer clip_rect, float4 color, unsigned int flags = 0);

            vec2 measure(std::string str, float size, unsigned int flags = 0);

            void set_states(Graphics::CommandList::ptr& list)
            {
                // Set the default rendering states
                pRenderStates->SetStates(list, 0);
            }

        private:
            static Font::ptr create_new(const std::string& header);

            IFW1GlyphRenderStates* pRenderStates;

    };

    class FontSystem : public Singleton<FontSystem>
    {
            friend class Singleton<FontSystem>;
            friend class Font;
            friend class FontGeometry;

            Cache < std::string, Font::ptr  > fonts;
            FW1_Factory  pFW1Factory;
            FontSystem();

            FW1_TextGeometry create_geometry()
            {
                FW1_TextGeometry geometry;
                pFW1Factory->CreateTextGeometry(&geometry);
                return geometry;
            }
        public:

            Font::ptr get_font(std::string font_name);

    };


    class FontGeometry
    {
            FW1_TextGeometry geometry;
            Font::ptr font;
            float size;
            std::mutex m;
        public:
            using ptr = s_ptr<FontGeometry>;

            FontGeometry();

            void set(Graphics::CommandList::ptr& list, std::wstring str, Font::ptr font, float size, sizer area, float4 color, unsigned int flags = 0);
            void draw(Graphics::CommandList::ptr& list, sizer clip_rect, unsigned int flags = 0, vec2 offset = {0, 0}, float scale = 1);

            unsigned int get_index(vec2 at);
            float get_size();
            vec2 get_pos(unsigned int index);
    };
}