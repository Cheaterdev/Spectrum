export module TextEngine;

import Core;
import HAL;

// Skribidi-backed text: shaping, bidi, fallback fonts and a glyph atlas.
//
// Threading: layout and atlas requests (build) run on the thread that walks the
// UI tree, atlas uploads (upload) in UI_PreDraw's single command list, and
// draws from any of UI_Render's parallel lists. Skribidi's atlas is not
// thread-safe and its UVs are handed out before the glyph is rasterized, so
// every glyph a draw can see must have been requested and uploaded before
// UI_Render starts -- which is what this split guarantees.
//
// All positions are relative to the top-left of the laid-out text's bounds.
export namespace Text
{
    enum class Weight
    {
        Light,
        Normal,
        Bold
    };

    struct Style
    {
        float  size = 16;   // pixels
        Weight weight = Weight::Normal;
    };

    struct Quad
    {
        float4   rect;     // pixels, relative to the layout's top-left
        float4   uv;       // normalized (u0, v0, u1, v1)
        uint32_t atlas;    // index into the engine's atlas textures
        bool     is_color;
    };

    struct Layout
    {
        std::vector<Quad> quads;
        vec2              size;
    };

    struct Caret
    {
        vec2  center;      // horizontal caret position, vertical middle of its line
        float height;
    };

    class Engine : public Singleton<Engine>
    {
        friend class Singleton<Engine>;

        struct Impl;
        std::unique_ptr<Impl> impl;

        Engine();
        ~Engine();

    public:
        // Once per frame, before any build(): evicts unused atlas items and layouts.
        void begin_frame();

        // Layout-cache queries: request no glyphs, so they are safe to call
        // outside the UI tree walk. Offsets are in codepoints.
        vec2         measure(std::string_view utf8, Style style);
        Caret        caret(std::string_view utf8, Style style, uint32_t offset);
        uint32_t     hit_test(std::string_view utf8, Style style, vec2 at);

        // Lays out utf8, requests its glyphs from the atlas and rasterizes any
        // that are missing (CPU side only).
        void build(std::string_view utf8, Style style, Layout& out);

        // True when rasterized glyphs are waiting for upload().
        bool has_pending_upload();
        void upload(HAL::CommandList::ptr& list);

        // pos is the layout's top-left in window pixels; quads are clipped to clip.
        void draw(HAL::CommandList::ptr& list, const Layout& layout, vec2 pos, float4 color, sizer clip, vec2 window_size);
    };
}
