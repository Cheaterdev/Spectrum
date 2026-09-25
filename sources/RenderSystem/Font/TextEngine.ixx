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

    enum class Family
    {
        Sans,   // Segoe UI
        Mono    // Consolas
    };

    struct Style
    {
        float  size = 16;   // pixels
        Weight weight = Weight::Normal;
        Family family = Family::Sans;
    };

    // Per-codepoint colors computed from the text (syntax highlighting): packed
    // RGBA8 (r in the low byte), 0 = no color. Applied at render time, so they
    // never enter the document or its undo history; an explicit color span in
    // the document wins over them.
    using Highlighter = std::function<void(std::u32string_view text, std::vector<uint32_t>& colors)>;

    struct Quad
    {
        float4   rect;     // pixels, relative to the layout's top-left
        float4   uv;       // normalized (u0, v0, u1, v1)
        float4   color;    // own color (span paint or syntax), if has_color
        uint32_t atlas;    // index into the engine's atlas textures
        bool     is_color;
        // With its own color the glyph ignores the draw() tint's rgb (a dark
        // editor tint would otherwise multiply syntax colors to black) and
        // keeps only its alpha.
        bool     has_color;
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

    class Editor;

    class Engine : public Singleton<Engine>
    {
        friend class Singleton<Engine>;
        friend class Editor;

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

    // Editable rich text (skb_editor): Unicode input, bidi-aware caret movement,
    // mouse selection with double/triple click, undo/redo. Every call must come
    // from the UI tree-walk thread, like Engine::build. Positions are logical
    // units relative to the editor's top-left; offsets are codepoints.
    class Editor
    {
        struct Impl;
        std::unique_ptr<Impl> impl;

    public:
        enum class Key { Left, Right, Up, Down, Home, End, Backspace, Delete, Enter };

        explicit Editor(Style style);
        ~Editor();

        // Skribidi's input filter callback holds the address of `filter`.
        Editor(const Editor&) = delete;
        Editor& operator=(const Editor&) = delete;

        // Replaces the text and clears undo history. Does not count as a change.
        void        set_text(std::string_view utf8);
        std::string get_text() const;

        // True once after the text changed through editing (typing, paste, undo...).
        bool take_changed();

        void key(Key key, bool shift, bool ctrl);
        void insert(char32_t codepoint);
        void insert(std::string_view utf8);

        bool        has_selection() const;
        std::string get_selected_text() const;
        void        delete_selection();
        void        select_all();
        void        select_none();

        void undo();
        void redo();
        bool can_undo() const;
        bool can_redo() const;

        // time_seconds lets the editor detect double and triple clicks.
        void mouse_click(vec2 pos, bool shift, double time_seconds);
        void mouse_drag(vec2 pos);

        // Drag-and-drop of the selection (skb_editor only does drag-to-select).
        bool  selection_contains(vec2 pos) const;
        Caret caret_at(vec2 pos) const;
        // Moves (or copies) the selected text to the caret position under pos,
        // as one undo step, and leaves the dropped text selected.
        void  move_selection(vec2 pos, bool copy);
        // Inserts utf8 (dropped from elsewhere) at the caret position under pos,
        // as one undo step, and leaves it selected.
        void  drop_text(vec2 pos, std::string_view utf8);

        // Codepoints for which this returns false never enter the text, whether
        // typed or pasted.
        std::function<bool(char32_t)> filter;

        // Run over the whole text whenever it changes.
        Highlighter highlighter;

        // Formatting of the selection; with no selection they set what the
        // next typed text gets.
        void toggle_bold();
        void toggle_italic();
        void set_color(float4 color);
        void clear_color();

        // Glyph quads at scale pixels per logical unit; requests missing glyphs.
        void build(float scale, Layout& out);

        // Extent of all paragraphs, logical units.
        vec2 content_size() const;

        Caret              caret() const;
        std::vector<float4> selection_rects() const;   // (x0, y0, x1, y1)
    };
}
