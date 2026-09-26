# UI Refactor Backlog

Ordered by implementation complexity (simplest first).

---

## ~~1. EditText — missing `on_change` callback~~ ✓
**Complexity:** trivial  
Add a `std::function<void(const std::string&)> on_change` callback fired on every keystroke inside `process_keys()`.

## ~~2. Separator~~ ✓
**Complexity:** trivial  
A thin `colored_rect` wrapper (1–2px, configurable color/orientation). Useful as a divider in layouts and menus.

## ~~3. ProgressBar~~ ✓
**Complexity:** low  
Draws a filled rect scaled by `float value` (0..1) over a background track. No interaction needed.

## ~~4. VerticalLayout~~ ✓
**Complexity:** low  
Mirror of `HorizontalLayout` — stacks children with `dock::TOP` instead of `dock::LEFT`.

## ~~5. Float Slider~~ ✓
**Complexity:** low  
Extract `color_slider` from `ColorPicker` into a proper standalone widget with:
- `float min`, `float max`, `float value`
- `std::function<void(float)> on_change`
- Horizontal and vertical orientation

## ~~6. EditText — placeholder text~~ ✓
**Complexity:** low  
Draw a grayed-out hint string when `text` is empty and the widget is unfocused.

## ~~7. EditText — input mask / validation~~ ✓
**Complexity:** low-medium  
`std::function<bool(char)> filter` called per keystroke; reject characters that don't pass. Enables numeric-only, hex-only, etc.

## ~~8. FloatBox~~ ✓ (`float_box`: edit_text subclass, commits on Enter/focus loss, Up/Down + wheel while focused; used by the Properties panel for unconstrained float/int Variables)
**Complexity:** low-medium  
Editable float value: `EditText` with numeric filter + scroll-wheel increment/decrement + configurable step, min, max.  
Replaces the int-only `ValueBox` pattern for float data.

## ~~9. Toggle Button~~ ✓ (implemented as toggle_switch — mobile-style pill with animated thumb)
**Complexity:** low-medium  
A `Button` that remembers pressed/unpressed state across clicks (latching). Fires `on_toggle(bool)`. Different from `CheckBox` in that it looks like a button.

## ~~10. Collapsible Section / Accordion~~ ✓
**Complexity:** medium  
A header row (clickable label + arrow icon) that shows/hides a child container. Commonly used in property inspectors. Needs `MATCH_CHILDREN` height to collapse to zero.

## ~~11. Multi-select ListBox~~ ✓ (`list_box::multi_select`, `selected_indices()`, `on_selection_changed`)
**Complexity:** medium  
Extend `ListBox` with shift-click range select and ctrl-click toggle. Expose `std::vector<int> selected_indices` and `on_selection_changed`.

## ~~12. Tooltip~~ ✓
**Complexity:** medium  
On hover (after a short delay), show a floating label near the cursor. Requires a timer in `think()`, a top-level overlay element, and a `std::string tooltip` field on `base` (or per-widget).

## ~~13. Vec2 / Vec3 / Vec4 Input~~ ✓ (`vector_box(components)`)
**Complexity:** medium  
Row of 2–4 `FloatBox` instances with X/Y/Z/W labels, grouped in a `HorizontalLayout`. Fires `on_change(floatN)`. Bread-and-butter for 3D transform/color editing.

## ~~14. ComboBox — searchable/filterable variant~~ ✓ (`combo_box::set_searchable(true)`, before add_item)
**Complexity:** medium  
Embed an `EditText` at the top of the dropdown that filters the item list as you type.

## ~~15. Splitter / SplitView~~ ✓ (`split_view(horizontal)`, ratio-based, `first()`/`second()`)
**Complexity:** medium-high  
Two child panels separated by a thin draggable `Resizer`. Distributes available space between the two halves (ratio stored as float). Supports horizontal and vertical split.

## ~~16. Toast / Notification~~ ✓ (`toast_manager::show(text, kind, seconds)`, any thread; manager added in main.cpp)
**Complexity:** medium-high  
Timed overlay messages that appear in a corner, stack vertically, and fade out after N seconds. Requires a global manager attached to `user_interface`.

## 17. ~~Context Menu (right-click)~~ ✓ (edit_text has one)
Already covered by `menu_list`: build it on right-button up, set `menu->pos` to the cursor, `menu->self_open(user_ui)`. `edit_text` now does this (Undo/Redo/Cut/Copy/Paste/Delete/Select All); FlowGraph canvas and AssetExplorer use the same pattern.

---

## Skribidi Text Stack

**Goal:** Replace the FreeType font/label system with Skribidi as the text backend, then build formatting, icons and an HLSL/Prism code editor on top.

**Why:** Skribidi provides shaping, BiDi, font fallback, color emoji, cursor navigation, selection, undo, text attributes (colored/bold spans) and SVG icon rasterization — none of which the old `label` / `edit_text` had.

### Done (2026-09-25)

**Phase 0 — keyboard input.** `Window.cpp` handles `WM_CHAR` (Unicode window, UTF-16 surrogates joined) and captures `key_mods` with each key message; `InputHandler`/`base` gained `on_char` and a `key_mods` argument on `on_key_action`; clipboard read/write hooks on `user_interface` (`get_clipboard`/`set_clipboard`, wired in `main.cpp`).

**Phase 1 — sample.** Skribidi built and its tests passing at the pinned commit (`dee63d6`, 2026-08-17). Answered the open questions:
- Atlas vs FrameGraph: atlas textures are persistent resources outside the graph, like the old `FontAtlas`.
- API stability: pinned commit + one wrapper module keeps churn contained.
- `Fonts::Font`: fully replaced (removal still pending, see below).

**Phase 2 — dependencies.** `custom-overlay/skribidi`: own `CMakeLists.txt` (upstream FetchContents its deps and forces the static CRT). SheenBidi, libunibreak 6.1 and budouxc are compiled into `skribidi.lib` at Skribidi's pinned commits; harfbuzz comes from vcpkg (baseline sheenbidi 3.0 / libunibreak 7.0 are major versions ahead of what Skribidi targets).

**Phase 3 — module wrapper.** `sources/Modules/skribidi/skribidi.ixx` (header-unit re-export). Macros and `static inline` helpers (`SKB_ATTRIBUTE_SET_FROM_STATIC_ARRAY`, `skb_rgba`, `SKB_CURRENT_SELECTION`, `INT32_MIN`) don't cross the module boundary — build those values by hand.

**Phase 4 — D3D12 backend.** `Text::Engine` (`RenderSystem/Font/TextEngine.*`): Segoe UI Light/Regular/Bold + Symbol + Emoji from `%WINDIR%\Fonts`, 2048² atlas created at full size (never grows), `UI::Text::GlyphRender` PSO (`font_render.prism`, `shaders/gui/glyph.hlsl`). Threading split: layout + glyph requests on the tree walk (`on_pre_render`), uploads in `UI_PreDraw`, draws from the parallel `UI_Render` lists. 1:1 glyphs are pixel-snapped; bilinear sampling for scaled ones.

**Phase 5 — widgets.**
- `label` draws through `Text::Engine` (no per-label cache texture); sizes on text/font-size change, since `on_pre_render` never runs for an element with empty bounds.
- `edit_text` runs on `Text::Editor` (`skb_editor`): full Unicode, bidi-aware caret, double/triple-click, Ctrl+Z/Y, clipboard, context menu, drag-and-drop of selected text within and between fields via the engine DnD system (Ctrl = copy). Childless by design: `add_child` is UI-thread only and a child covering the field would swallow drag moves.
- Test > Editor menu page in `main.cpp` for trying it.

### Remaining

**Phase 5 cleanup**
- Remove FreeType + `TextSystem`: move `FW1_*` alignment flags into `GUI:Label`, port the remaining `Fonts::` users (BinaryAsset preview, ParameterWindow, GUI tests), drop the "Skribidi bring-up check" line in `main.cpp`.
- Button label height: Skribidi line box is taller than the old FreeType measure, so button labels overflow and fall back to top-left anchoring.
- `edit_text` horizontal scroll for long text (Skribidi `SKB_OVERFLOW_SCROLL` + editor width).
- Upstream: a fresh `skb_editor` has 0 paragraphs and `skb_rich_layout_get_text_range_bounds` reads `paragraphs[-1]`; worked around by seeding `set_text("")`. Patch in the overlay port or report upstream.

**Formatting** *(requested next)*
- Bold / italic / color spans in `edit_text` via `skb_editor_toggle_attribute`; Ctrl+B / Ctrl+I.
- Glyph quads already carry per-run paint color, so colored spans need no renderer change.
- Drag-and-drop and paste to carry rich text (`get_rich_text_in_range` / `insert_rich_text`) instead of plain UTF-8.

**Portability**
- Engine-level key enum: `Window.cpp` translates `VK_*` once, so Win32 key codes stop leaking into `edit_text` and other GUI code.
- Platform-specific font list (Android: `/system/fonts`, Roboto/Noto).

**Phase 6 — built on the new stack**
- Icons through `skb_icon_collection` (collapsible arrows, tree toggles, button glyphs).
- Backlog widgets on the new `edit_text`: #8 FloatBox, #13 Vec2/3/4 Input, #14 searchable ComboBox.
- `code_editor`: HLSL and Prism tokenizers mapping token types to attribute colors.
- Optional SDF glyphs for zoomable text (FlowGraph canvas).

---

## Done
- [x] ColorPicker (RGB + alpha sliders, hex/rgb display, gradient GPU draw)
- [x] Keyboard input: WM_CHAR, modifiers, clipboard
- [x] Skribidi text engine (port, module, D3D12 atlas + glyph PSO)
- [x] label on Text::Engine
- [x] edit_text on skb_editor (Unicode, undo, context menu, drag-and-drop)
