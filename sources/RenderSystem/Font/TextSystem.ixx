export module  TextSystem;

import Core;
import HAL;

// ---------------------------------------------------------------------------
// Text flags  (same numeric values as the old FW1_TEXT_FLAG — all callers
// continue to compile without change)
// ---------------------------------------------------------------------------
export enum FW1_TEXT_FLAG : unsigned int
{
    FW1_LEFT              = 0x0,
    FW1_CENTER            = 0x1,
    FW1_RIGHT             = 0x2,
    FW1_TOP               = 0x0,
    FW1_VCENTER           = 0x4,
    FW1_BOTTOM            = 0x8,
    FW1_NOWORDWRAP        = 0x10,
    FW1_ALIASED           = 0x20,
    FW1_CLIPRECT          = 0x40,
    FW1_NOGEOMETRYSHADER  = 0x80,
    FW1_CONSTANTSPREPARED = 0x100,
    FW1_BUFFERSPREPARED   = 0x200,
    FW1_STATEPREPARED     = 0x400,
    FW1_IMMEDIATECALL     = FW1_CONSTANTSPREPARED | FW1_BUFFERSPREPARED | FW1_STATEPREPARED,
    FW1_RESTORESTATE      = 0x800,
    FW1_NOFLUSH           = 0x1000,
    FW1_CACHEONLY         = 0x2000,
    FW1_NONEWGLYPHS       = 0x4000,
    FW1_ANALYZEONLY       = 0x8000,
    FW1_UNUSED            = 0xffffffff
};

export namespace Fonts
{
    class FontSystem;
    class FontGeometry;

    // -----------------------------------------------------------------------
    // Font  — wraps a FreeType face; same public API as before
    // -----------------------------------------------------------------------
    class Font
    {
        friend class FontSystem;
        friend class FontGeometry;

        struct FTData;
        std::unique_ptr<FTData> m_data;
        std::string font_name;

    public:
        ~Font();
        using ptr = s_ptr<Font>;

        void draw(HAL::CommandList::ptr& list, std::string  str, float size, vec2   pos,  float4 color, unsigned int flags = 0);
        void draw(HAL::CommandList::ptr& list, std::wstring str, float size, vec2   pos,  float4 color, unsigned int flags = 0);
        void draw(HAL::CommandList::ptr& list, std::string  str, float size, sizer  area, float4 color, unsigned int flags = 0);
        void draw(HAL::CommandList::ptr& list, std::wstring str, float size, sizer  area, float4 color, unsigned int flags = 0);
        void draw(HAL::CommandList::ptr& list, std::string  str, float size, sizer  area, sizer clip_rect, float4 color, unsigned int flags = 0);
        void draw(HAL::CommandList::ptr& list, std::wstring str, float size, sizer  area, sizer clip_rect, float4 color, unsigned int flags = 0);

        vec2 measure(std::string str, float size, unsigned int flags = 0);

        // Sets the font render pipeline + topology on the command list
        void set_states(HAL::CommandList::ptr& list);

    private:
        static Font::ptr create_new(const std::string& name);
    };

    // -----------------------------------------------------------------------
    // FontSystem  — singleton factory / cache
    // -----------------------------------------------------------------------
    class FontSystem : public Singleton<FontSystem>
    {
        friend class Singleton<FontSystem>;
        friend class Font;
        friend class FontGeometry;

        Cache<std::string, Font::ptr> fonts;
        FontSystem();

    public:
        Font::ptr get_font(std::string font_name);
    };

    // -----------------------------------------------------------------------
    // FontGeometry  — deferred / pre-analysed text geometry
    // -----------------------------------------------------------------------
    class FontGeometry
    {
        struct Impl;
        std::unique_ptr<Impl> m_impl;
        std::mutex m;

    public:
        using ptr = s_ptr<FontGeometry>;

        FontGeometry();
        ~FontGeometry();

        void clear();

        // Append text and return updated area (left edge advanced by text width)
        sizer add(HAL::CommandList::ptr& list, std::wstring str, Font::ptr font,
                  float size, sizer area, float4 color, unsigned int flags = 0);

        // Replace geometry with this string
        void set(HAL::CommandList::ptr& list, std::wstring str, Font::ptr font,
                 float size, sizer area, float4 color, unsigned int flags = 0);

        // Draw the stored geometry
        void draw(HAL::CommandList::ptr& list, sizer clip_rect,
                  unsigned int flags = 0, vec2 offset = {0, 0}, float scale = 1);

        // Cursor helpers for text editing
        unsigned int get_index(vec2 at);
        float        get_size();
        vec2         get_pos(unsigned int index);
    };

} // namespace Fonts
