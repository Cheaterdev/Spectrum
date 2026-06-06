// Global module fragment — FreeType headers must live here (before the module
// declaration) so that the C macros they use are processed by the preprocessor
// before C++20 module parsing begins.
module;

#include <ft2build.h>
#include FT_FREETYPE_H

module TextSystem;

import Core;
import HAL;
import Graphics;

// ============================================================================
//  Internal implementation (anonymous namespace)
// ============================================================================
namespace
{

// ---------------------------------------------------------------------------
//  Constants
// ---------------------------------------------------------------------------
constexpr uint32_t ATLAS_W       = 2048;
constexpr uint32_t ATLAS_H       = 2048;
constexpr uint32_t MAX_GLYPHS    = 8192;
constexpr uint32_t GLYPH_PAD     = 1;   // 1-pixel gap around each glyph

// ---------------------------------------------------------------------------
//  GlyphAtlasEntry
//  Must match the layout the font shaders expect for Buffer<float4>:
//    entry 0  →  (tex_left, tex_top,  tex_right,  tex_bottom)  UV [0..1]
//    entry 1  →  (pos_left, pos_top,  pos_right,  pos_bottom)  pixel offsets
// ---------------------------------------------------------------------------
struct GlyphAtlasEntry
{
    float tex_left, tex_top, tex_right, tex_bottom;
    float pos_left, pos_top, pos_right, pos_bottom;

    // Required by HAL::StructuredBufferView<T> — the engine's ObjectTreeSerializer
    // calls cereal serialization on the element type via StructuredBufferView::describe().
    SERIALIZE()
    {
        ar & NVP(tex_left)  & NVP(tex_top)    & NVP(tex_right)  & NVP(tex_bottom)
           & NVP(pos_left)  & NVP(pos_top)    & NVP(pos_right)  & NVP(pos_bottom);
    }
};
static_assert(sizeof(GlyphAtlasEntry) == sizeof(float) * 8);

// ---------------------------------------------------------------------------
//  GlyphCacheKey
// ---------------------------------------------------------------------------
struct GlyphCacheKey
{
    FT_Face  face;
    uint32_t char_code;
    uint32_t pixel_size;

    bool operator<(const GlyphCacheKey& o) const noexcept
    {
        if (face      != o.face)      return face      < o.face;
        if (char_code != o.char_code) return char_code < o.char_code;
        return pixel_size < o.pixel_size;
    }
};

// ---------------------------------------------------------------------------
//  FontAtlas  — single R8_UNORM texture + coord buffer shared by all fonts
// ---------------------------------------------------------------------------
class FontAtlas
{
public:
    FontAtlas()
    {
        HAL::ResourceDesc desc =
            HAL::ResourceDesc::Tex2D(HAL::Format::R8_UNORM,
                                     {ATLAS_W, ATLAS_H}, 1, 1);
        m_texture.reset(new HAL::Texture(HAL::Device::get(), desc));
        m_texture->resource->set_name("FontAtlas::texture");

        m_coord_buf =
            HAL::StructuredBufferView<GlyphAtlasEntry>(HAL::Device::get(), MAX_GLYPHS);

        m_cpu.resize(ATLAS_W * ATLAS_H, 0u);
        m_entries.reserve(MAX_GLYPHS);
    }

    // Returns atlas glyph index, or 0xFFFF'FFFF on failure.
    // Rasterises the glyph from the FreeType face if not yet cached.
    uint32_t get_or_create(FT_Face face, uint32_t char_code, float size)
    {
        uint32_t px = static_cast<uint32_t>(std::ceil(size));
        GlyphCacheKey key{face, char_code, px};

        {
            std::lock_guard<std::mutex> lk(m_mtx);
            auto it = m_cache.find(key);
            if (it != m_cache.end()) return it->second;
        }

        // --- rasterise ---
        FT_Set_Pixel_Sizes(face, 0, px);
        FT_UInt gi = FT_Get_Char_Index(face, char_code);
        if (FT_Load_Glyph(face, gi, FT_LOAD_RENDER) != 0)
            return 0xFFFFFFFFu;

        FT_Bitmap& bm = face->glyph->bitmap;

        std::lock_guard<std::mutex> lk(m_mtx);

        // Re-check after acquiring the lock
        {
            auto it = m_cache.find(key);
            if (it != m_cache.end()) return it->second;
        }

        if (m_entries.size() >= MAX_GLYPHS)
            return 0xFFFFFFFFu;

        uint32_t idx = static_cast<uint32_t>(m_entries.size());
        GlyphAtlasEntry e{};

        if (bm.width > 0 && bm.rows > 0)
        {
            uint32_t w = bm.width;
            uint32_t h = bm.rows;

            // Advance to next row if needed
            if (m_cur_x + w + GLYPH_PAD > ATLAS_W)
            {
                m_cur_x    = GLYPH_PAD;
                m_cur_y   += m_row_h + GLYPH_PAD;
                m_row_h    = 0;
            }

            if (m_cur_y + h + GLYPH_PAD > ATLAS_H)
                return 0xFFFFFFFFu;   // atlas full

            // Copy bitmap into atlas
            for (uint32_t row = 0; row < h; ++row)
            {
                const uint8_t* src = bm.buffer + row * std::abs(bm.pitch);
                uint8_t*       dst = m_cpu.data() + (m_cur_y + row) * ATLAS_W + m_cur_x;
                std::memcpy(dst, src, w);
            }

            // UV coords
            e.tex_left   = static_cast<float>(m_cur_x)     / ATLAS_W;
            e.tex_top    = static_cast<float>(m_cur_y)     / ATLAS_H;
            e.tex_right  = static_cast<float>(m_cur_x + w) / ATLAS_W;
            e.tex_bottom = static_cast<float>(m_cur_y + h) / ATLAS_H;

            // Pixel offsets from pen origin (baseline)
            // FreeType: bitmap_left = left of bitmap relative to pen (positive = right)
            //           bitmap_top  = top  of bitmap relative to pen (positive = UP)
            // Shader uses Y-down screen space, so negate bitmap_top for top offset.
            e.pos_left   = static_cast<float>(face->glyph->bitmap_left);
            e.pos_top    = -static_cast<float>(face->glyph->bitmap_top);
            e.pos_right  = e.pos_left + static_cast<float>(w);
            e.pos_bottom = e.pos_top  + static_cast<float>(h);

            // Update dirty region
            m_dirty_l = std::min(m_dirty_l, m_cur_x);
            m_dirty_t = std::min(m_dirty_t, m_cur_y);
            m_dirty_r = std::max(m_dirty_r, m_cur_x + w);
            m_dirty_b = std::max(m_dirty_b, m_cur_y + h);

            m_cur_x += w + GLYPH_PAD;
            m_row_h  = std::max(m_row_h, h);
        }
        // else: whitespace / empty glyph — zero-size entry, positions stay 0

        m_entries.push_back(e);
        m_cache[key] = idx;
        m_dirty = true;
        return idx;
    }

    // Upload new glyph data to the GPU
    void flush(HAL::CommandList::ptr& list)
    {
        if (!m_dirty) return;
        std::lock_guard<std::mutex> lk(m_mtx);
        if (!m_dirty) return;

        // Upload dirty region of the atlas texture
        if (m_dirty_r > m_dirty_l && m_dirty_b > m_dirty_t)
        {
            list->get_copy().update_texture(
                m_texture->resource,
                ivec3(static_cast<int>(m_dirty_l), static_cast<int>(m_dirty_t), 0),
                ivec3(static_cast<int>(m_dirty_r - m_dirty_l),
                      static_cast<int>(m_dirty_b - m_dirty_t), 1),
                HAL::calc_subresource(0, 0, 0, 1, 1),
                reinterpret_cast<const char*>(
                    m_cpu.data() + m_dirty_t * ATLAS_W + m_dirty_l),
                static_cast<int>(ATLAS_W));
        }

        // Upload updated coord buffer entries
        if (!m_entries.empty())
        {
            list->get_copy().update(m_coord_buf, 0,
                std::span{m_entries.data(), m_entries.size()});
        }

        // Reset dirty state
        m_dirty   = false;
        m_dirty_l = ATLAS_W;
        m_dirty_t = ATLAS_H;
        m_dirty_r = 0;
        m_dirty_b = 0;
    }

    // Bind the atlas texture and coord buffer to the FontRendering slot
    void bind(HAL::CommandList::ptr& list)
    {
        Slots::FontRendering rendering;
        rendering.GetTex0() = m_texture->texture_2d().texture2D;
        rendering.GetPositions() =
            m_coord_buf.resource->create_view<
                HAL::FormattedBufferView<float4, HAL::Format::R32G32B32A32_FLOAT>>(*list)
            .buffer;
        list->get_graphics().set(rendering);
    }

private:
    HAL::Texture::ptr                           m_texture;
    HAL::StructuredBufferView<GlyphAtlasEntry>  m_coord_buf;

    std::vector<uint8_t>        m_cpu;
    std::vector<GlyphAtlasEntry> m_entries;
    std::map<GlyphCacheKey, uint32_t> m_cache;

    uint32_t m_cur_x  = GLYPH_PAD;
    uint32_t m_cur_y  = GLYPH_PAD;
    uint32_t m_row_h  = 0;

    // Dirty rect (tracks changed area in the atlas)
    uint32_t m_dirty_l = ATLAS_W, m_dirty_t = ATLAS_H;
    uint32_t m_dirty_r = 0,       m_dirty_b = 0;
    bool m_dirty = false;

    std::mutex m_mtx;
};

// ---------------------------------------------------------------------------
//  Module-level singletons (created lazily)
// ---------------------------------------------------------------------------
static FontAtlas*  s_atlas     = nullptr;
static FT_Library  s_ft_lib    = nullptr;

static FontAtlas& get_atlas()
{
    if (!s_atlas) s_atlas = new FontAtlas();
    return *s_atlas;
}

static FT_Library get_ft_library()
{
    if (!s_ft_lib) FT_Init_FreeType(&s_ft_lib);
    return s_ft_lib;
}

// ---------------------------------------------------------------------------
//  Font file resolver  (name  →  file path)
// ---------------------------------------------------------------------------
static std::string resolve_font_path(const std::string& font_name)
{
    namespace fs = std::filesystem;

    // 1. Direct path (absolute or relative)
    if (fs::exists(font_name)) return font_name;

    // 2. Common display-name → filename mapping
    static const std::map<std::string, std::string> k_names =
    {
        {"Segoe UI Light",    "segoeuil.ttf"},
        {"Segoe UI",          "segoeui.ttf" },
        {"Segoe UI Semibold", "seguisb.ttf" },
        {"Arial",             "Arial.ttf"   },
        {"Arial Bold",        "arialbd.ttf" },
        {"Courier New",       "cour.ttf"    },
        {"Verdana",           "verdana.ttf" },
        {"Tahoma",            "tahoma.ttf"  },
        {"Calibri",           "calibri.ttf" },
    };

    auto it = k_names.find(font_name);
    std::string filename = (it != k_names.end()) ? it->second : font_name;

    // 3. Working directory
    if (fs::exists(filename)) return filename;

#ifdef _WIN32
    // 4. Windows system font folder
    std::string win = "C:\\Windows\\Fonts\\" + filename;
    if (fs::exists(win)) return win;
#elif defined(__ANDROID__)
    std::string android = "/system/fonts/" + filename;
    if (fs::exists(android)) return android;
#endif

    // 5. Last resort — Arial.ttf in working directory
    if (fs::exists("Arial.ttf")) return "Arial.ttf";

    return {};
}

// ---------------------------------------------------------------------------
//  Text layout helper types
// ---------------------------------------------------------------------------

// A single glyph vertex — layout must match Table::Glyph exactly.
struct GlyphVtx
{
    float2   pos;         // pen position (pixel space)
    uint32_t atlas_idx;   // index into the atlas coord buffer
    float4   color;
};
static_assert(sizeof(GlyphVtx) == sizeof(Table::Glyph));

// ---------------------------------------------------------------------------
//  layout_text
//  Converts a wstring into a list of GlyphVtx ready for GPU upload.
// ---------------------------------------------------------------------------
static std::vector<GlyphVtx> layout_text(
    FT_Face        face,
    const std::wstring& str,
    float          size,
    const sizer&   area,
    const float4&  color,
    unsigned int   flags,
    FontAtlas&     atlas)
{
    uint32_t px = static_cast<uint32_t>(std::ceil(size));
    FT_Set_Pixel_Sizes(face, 0, px);

    float line_height = static_cast<float>(face->size->metrics.height   >> 6);
    float ascender    = static_cast<float>(face->size->metrics.ascender  >> 6);

    std::vector<GlyphVtx> out;
    out.reserve(str.size());

    float y = area.top + ascender;

    // Split by newlines, then lay out each line
    size_t start = 0;
    while (start <= str.size())
    {
        size_t nl  = str.find(L'\n', start);
        size_t end = (nl != std::wstring::npos) ? nl : str.size();
        std::wstring line = str.substr(start, end - start);
        start = end + 1;

        if (y > area.bottom) break;

        // --- Measure line width for alignment ---
        float line_w = 0.f;
        {
            FT_UInt prev = 0;
            for (wchar_t wc : line)
            {
                FT_UInt gi = FT_Get_Char_Index(face, static_cast<uint32_t>(wc));
                if (prev && gi)
                {
                    FT_Vector kern{};
                    FT_Get_Kerning(face, prev, gi, FT_KERNING_DEFAULT, &kern);
                    line_w += static_cast<float>(kern.x >> 6);
                }
                if (FT_Load_Glyph(face, gi, FT_LOAD_DEFAULT) == 0)
                    line_w += static_cast<float>(face->glyph->advance.x >> 6);
                prev = gi;
            }
        }

        float box_w = area.right - area.left;
        float x;
        if ((flags & FW1_RIGHT) != 0)
            x = area.right - line_w;
        else if ((flags & FW1_CENTER) != 0)
            x = area.left + (box_w - line_w) * 0.5f;
        else
            x = area.left;

        // --- Emit glyph vertices ---
        FT_UInt prev = 0;
        for (wchar_t wc : line)
        {
            uint32_t char_code = static_cast<uint32_t>(wc);
            FT_UInt  gi        = FT_Get_Char_Index(face, char_code);

            // Kerning
            if (prev && gi)
            {
                FT_Vector kern{};
                FT_Get_Kerning(face, prev, gi, FT_KERNING_DEFAULT, &kern);
                x += static_cast<float>(kern.x >> 6);
            }

            // Word-wrap (simple: wrap on advance overflow)
            if (!(flags & FW1_NOWORDWRAP))
            {
                if (FT_Load_Glyph(face, gi, FT_LOAD_DEFAULT) == 0)
                {
                    float adv = static_cast<float>(face->glyph->advance.x >> 6);
                    if (x + adv > area.right)
                    {
                        x  = area.left;
                        y += line_height;
                        if (y > area.bottom) goto done;
                    }
                }
            }

            {
                uint32_t aidx = atlas.get_or_create(face, char_code, size);
                if (aidx != 0xFFFFFFFFu)
                    out.push_back({float2{x, y}, aidx, color});
            }

            // Advance pen
            if (FT_Load_Glyph(face, gi, FT_LOAD_DEFAULT) == 0)
                x += static_cast<float>(face->glyph->advance.x >> 6);

            prev = gi;
        }

        y += line_height;
        if (start > str.size()) break;
    }
done:
    return out;
}

// ---------------------------------------------------------------------------
//  ShaderConstants  — matches Table::FontRenderingConstants layout
// ---------------------------------------------------------------------------
struct ShaderConstants
{
    float TransformMatrix[16];
    float ClipRect[4];
};
static_assert(sizeof(ShaderConstants) == sizeof(Table::FontRenderingConstants));

// ---------------------------------------------------------------------------
//  draw_vertices  — uploads vertices and issues the draw call
// ---------------------------------------------------------------------------
static void draw_vertices(
    HAL::CommandList::ptr&     list,
    const std::vector<GlyphVtx>& verts,
    const sizer*               clip_rect,
    const float*               transform_matrix,
    unsigned int               /*flags*/)
{
    if (verts.empty()) return;

    FontAtlas& atlas = get_atlas();

    // 1. Flush any newly rasterised glyphs to the GPU
    atlas.flush(list);

    // 2. Bind atlas texture + coord buffer
    atlas.bind(list);

    // 3. Set pipeline state and topology
    auto formats = list->get_graphics().get_formats();
    if (formats.empty()) return;
    list->get_graphics().set_pipeline<PSOS::FontRender>(
        PSOS::FontRender::Format(formats[0]));
    list->get_graphics().set_topology(HAL::PrimitiveTopologyType::POINT,
                                      HAL::PrimitiveTopologyFeed::LIST);

    // 4. Shader constants (transform + clip rect)
    ShaderConstants sc{};

    if (transform_matrix)
    {
        std::memcpy(sc.TransformMatrix, transform_matrix, sizeof(sc.TransformMatrix));
    }
    else
    {
        // Orthographic screen-space transform
        float w = 512.f, h = 512.f;
        auto vps = list->get_graphics().get_viewports();
        if (!vps.empty() && vps[0].size.x >= 1.f && vps[0].size.y >= 1.f)
        {
            w = vps[0].size.x;
            h = vps[0].size.y;
        }
        sc.TransformMatrix[0]  =  2.f / w;
        sc.TransformMatrix[12] = -1.f;
        sc.TransformMatrix[5]  = -2.f / h;
        sc.TransformMatrix[13] =  1.f;
        sc.TransformMatrix[10] =  1.f;
        sc.TransformMatrix[15] =  1.f;
    }

    if (clip_rect)
    {
        // Convention matches old FW1FontWrapper: negate left/top
        sc.ClipRect[0] = -clip_rect->left;
        sc.ClipRect[1] = -clip_rect->top;
        sc.ClipRect[2] =  clip_rect->right;
        sc.ClipRect[3] =  clip_rect->bottom;
    }
    else
    {
        constexpr float INF = std::numeric_limits<float>::max();
        sc.ClipRect[0] = sc.ClipRect[1] = sc.ClipRect[2] = sc.ClipRect[3] = INF;
    }

    Slots::FontRenderingConstants gpu_consts;
    std::memcpy(&gpu_consts, &sc, sizeof(sc));
    list->get_graphics().set(gpu_consts);

    // 5. Upload glyph vertex buffer (transient placement)
    uint32_t count = static_cast<uint32_t>(verts.size());
    auto placed = list->place_data(sizeof(Table::Glyph) * count, sizeof(Table::Glyph));
    list->write(placed, std::span{verts.data(), count});

    auto view = placed.resource->create_view<HAL::StructuredBufferView<Table::Glyph>>(
        *list,
        HAL::StructuredBufferViewDesc{
            static_cast<uint>(placed.resource_offset),
            static_cast<uint>(placed.size),
            HAL::counterType::NONE});

    Slots::FontRenderingGlyphs glyphs_slot;
    glyphs_slot.GetData() = view;
    list->get_graphics().set(glyphs_slot);

    // 6. Draw
    list->get_graphics().draw(count, 0);
}

} // anonymous namespace


// ============================================================================
//  Font::FTData  — pimpl
// ============================================================================
namespace Fonts
{

struct Font::FTData
{
    FT_Face face = nullptr;
};

// ============================================================================
//  Font
// ============================================================================

Font::~Font()
{
    if (m_data && m_data->face)
        FT_Done_Face(m_data->face);
}

Font::ptr Font::create_new(const std::string& name)
{
    return FontSystem::get().get_font(name);
}

// --- Immediate draw helpers ---

void Font::draw(HAL::CommandList::ptr& list,
                std::wstring str, float size,
                vec2 pos, float4 color, unsigned int flags)
{
    sizer area{pos.x, pos.y,
               pos.x + 1e6f,   // unbounded width
               pos.y + 1e6f};  // unbounded height
    draw(list, str, size, area, area, color, flags | FW1_NOWORDWRAP);
}

void Font::draw(HAL::CommandList::ptr& list,
                std::string str, float size,
                vec2 pos, float4 color, unsigned int flags)
{
    draw(list, convert(str), size, pos, color, flags);
}

void Font::draw(HAL::CommandList::ptr& list,
                std::wstring str, float size,
                sizer area, float4 color, unsigned int flags)
{
    draw(list, str, size, area, area, color, flags);
}

void Font::draw(HAL::CommandList::ptr& list,
                std::string str, float size,
                sizer area, float4 color, unsigned int flags)
{
    draw(list, convert(str), size, area, color, flags);
}

void Font::draw(HAL::CommandList::ptr& list,
                std::string str, float size,
                sizer area, sizer clip_rect,
                float4 color, unsigned int flags)
{
    draw(list, convert(str), size, area, clip_rect, color, flags);
}

void Font::draw(HAL::CommandList::ptr& list,
                std::wstring str, float size,
                sizer area, sizer clip_rect,
                float4 color, unsigned int flags)
{
    if (!m_data || !m_data->face) return;

    auto verts = layout_text(m_data->face, str, size, area, color, flags, get_atlas());
    draw_vertices(list, verts, &clip_rect, nullptr, flags | FW1_CLIPRECT);
}

vec2 Font::measure(std::string str, float size, unsigned int flags)
{
    if (!m_data || !m_data->face || str.empty()) return {0.f, 0.f};

    std::wstring wstr = convert(str);
    FT_Face face = m_data->face;
    uint32_t px = static_cast<uint32_t>(std::ceil(size));
    FT_Set_Pixel_Sizes(face, 0, px);

    float line_w    = 0.f;
    float max_w     = 0.f;
    float line_h    = static_cast<float>(face->size->metrics.height >> 6);
    float total_h   = line_h;  // at least one line

    FT_UInt prev = 0;
    for (wchar_t wc : wstr)
    {
        if (wc == L'\n')
        {
            max_w   = std::max(max_w, line_w);
            line_w  = 0.f;
            total_h += line_h;
            prev = 0;
            continue;
        }

        FT_UInt gi = FT_Get_Char_Index(face, static_cast<uint32_t>(wc));
        if (prev && gi)
        {
            FT_Vector kern{};
            FT_Get_Kerning(face, prev, gi, FT_KERNING_DEFAULT, &kern);
            line_w += static_cast<float>(kern.x >> 6);
        }
        if (FT_Load_Glyph(face, gi, FT_LOAD_DEFAULT) == 0)
            line_w += static_cast<float>(face->glyph->advance.x >> 6);

        prev = gi;
    }
    max_w = std::max(max_w, line_w);

    return {max_w, total_h};
}

void Font::set_states(HAL::CommandList::ptr& list)
{
    // Set the font render PSO and point topology
    auto formats = list->get_graphics().get_formats();
    if (formats.empty()) return;
    list->get_graphics().set_pipeline<PSOS::FontRender>(
        PSOS::FontRender::Format(formats[0]));
    list->get_graphics().set_topology(HAL::PrimitiveTopologyType::POINT,
                                      HAL::PrimitiveTopologyFeed::LIST);
}

// ============================================================================
//  FontSystem
// ============================================================================

FontSystem::FontSystem()
{
    fonts.create_func = [](const std::string& name) -> Font::ptr
    {
        std::string path = resolve_font_path(name);
        if (path.empty()) return nullptr;

        auto font     = Font::ptr(new Font());
        font->font_name = name;
        font->m_data    = std::make_unique<Font::FTData>();

        if (FT_New_Face(get_ft_library(), path.c_str(), 0, &font->m_data->face) != 0)
        {
            font->m_data->face = nullptr;
        }

        return font;
    };
}

Font::ptr FontSystem::get_font(std::string font_name)
{
    return fonts[font_name];
}

// ============================================================================
//  FontGeometry::Impl
// ============================================================================

struct FontGeometry::Impl
{
    std::vector<GlyphVtx> verts;
    Font::ptr             font;
    float                 size = 0.f;
};

// ============================================================================
//  FontGeometry
// ============================================================================

FontGeometry::FontGeometry()
    : m_impl(std::make_unique<Impl>())
{}

FontGeometry::~FontGeometry() = default;

void FontGeometry::clear()
{
    std::lock_guard<std::mutex> lk(m);
    m_impl->verts.clear();
}

void FontGeometry::set(HAL::CommandList::ptr& /*list*/,
                       std::wstring str, Font::ptr font,
                       float size, sizer area,
                       float4 color, unsigned int flags)
{
    std::lock_guard<std::mutex> lk(m);
    m_impl->verts.clear();
    m_impl->font = font;
    m_impl->size = size;

    if (!font || !font->m_data || !font->m_data->face) return;

    m_impl->verts = layout_text(font->m_data->face, str, size,
                                 area, color, flags, get_atlas());
}

sizer FontGeometry::add(HAL::CommandList::ptr& /*list*/,
                         std::wstring str, Font::ptr font,
                         float size, sizer area,
                         float4 color, unsigned int flags)
{
    std::lock_guard<std::mutex> lk(m);
    m_impl->font = font;
    m_impl->size = size;

    if (!font || !font->m_data || !font->m_data->face) return area;

    auto new_verts = layout_text(font->m_data->face, str, size,
                                  area, color, flags, get_atlas());
    for (auto& v : new_verts)
        m_impl->verts.push_back(v);

    // Advance the left edge by the measured text width
    sizer result = area;
    result.left += font->measure(convert(str), size, flags).x;
    return result;
}

void FontGeometry::draw(HAL::CommandList::ptr& list,
                         sizer clip_rect,
                         unsigned int flags,
                         vec2 offset,
                         float scale)
{
    std::lock_guard<std::mutex> lk(m);

    if (!m_impl->font || m_impl->verts.empty()) return;

    // Build the orthographic transform with offset + scale
    float w = 1.f, h = 1.f;
    auto vps = list->get_graphics().get_viewports();
    if (!vps.empty() && vps[0].size.x >= 1.f && vps[0].size.y >= 1.f)
    {
        w = vps[0].size.x;
        h = vps[0].size.y;
    }

    float mtx[16] = {};
    mtx[0]  = scale *  2.f / w;
    mtx[12] = -1.f + 2.f * offset.x / w;
    mtx[5]  = scale * -2.f / h;
    mtx[13] =  1.f - 2.f * offset.y / h;
    mtx[10] =  1.f;
    mtx[15] =  1.f;

    draw_vertices(list, m_impl->verts, &clip_rect, mtx, flags | FW1_CLIPRECT);
}

float FontGeometry::get_size()
{
    std::lock_guard<std::mutex> lk(m);
    return m_impl->size;
}

vec2 FontGeometry::get_pos(unsigned int index)
{
    std::lock_guard<std::mutex> lk(m);
    const auto& verts = m_impl->verts;
    if (verts.empty()) return {};

    if (index >= verts.size())
    {
        // Past the end: return position after the last glyph
        const auto& last = verts.back();
        return {last.pos.x + m_impl->size * 0.5f,
                last.pos.y - m_impl->size * 0.5f};
    }

    return {verts[index].pos.x,
            verts[index].pos.y - m_impl->size * 0.5f};
}

unsigned int FontGeometry::get_index(vec2 at)
{
    std::lock_guard<std::mutex> lk(m);
    const auto& verts = m_impl->verts;
    if (verts.empty()) return 0;

    unsigned int best = 0;
    float        best_dist = std::numeric_limits<float>::max();

    for (unsigned int i = 0; i < static_cast<unsigned int>(verts.size()); ++i)
    {
        vec2  p = {verts[i].pos.x, verts[i].pos.y - m_impl->size * 0.5f};
        float d = (at - p).length_squared();
        if (d < best_dist)
        {
            best_dist = d;
            best = i;
        }
    }

    // If the cursor is past the last glyph's centre, return index+1
    if (best == static_cast<unsigned int>(verts.size()) - 1)
    {
        if (verts[best].pos.x + m_impl->size * 0.5f < at.x)
            return best + 1;
    }

    return best;
}

} // namespace Fonts
