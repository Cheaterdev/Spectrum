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

## 6. EditText — placeholder text
**Complexity:** low  
Draw a grayed-out hint string when `text` is empty and the widget is unfocused.

## 7. EditText — input mask / validation
**Complexity:** low-medium  
`std::function<bool(char)> filter` called per keystroke; reject characters that don't pass. Enables numeric-only, hex-only, etc.

## 8. FloatBox
**Complexity:** low-medium  
Editable float value: `EditText` with numeric filter + scroll-wheel increment/decrement + configurable step, min, max.  
Replaces the int-only `ValueBox` pattern for float data.

## 9. Toggle Button
**Complexity:** low-medium  
A `Button` that remembers pressed/unpressed state across clicks (latching). Fires `on_toggle(bool)`. Different from `CheckBox` in that it looks like a button.

## 10. Collapsible Section / Accordion
**Complexity:** medium  
A header row (clickable label + arrow icon) that shows/hides a child container. Commonly used in property inspectors. Needs `MATCH_CHILDREN` height to collapse to zero.

## 11. Multi-select ListBox
**Complexity:** medium  
Extend `ListBox` with shift-click range select and ctrl-click toggle. Expose `std::vector<int> selected_indices` and `on_selection_changed`.

## 12. Tooltip
**Complexity:** medium  
On hover (after a short delay), show a floating label near the cursor. Requires a timer in `think()`, a top-level overlay element, and a `std::string tooltip` field on `base` (or per-widget).

## 13. Vec2 / Vec3 / Vec4 Input
**Complexity:** medium  
Row of 2–4 `FloatBox` instances with X/Y/Z/W labels, grouped in a `HorizontalLayout`. Fires `on_change(floatN)`. Bread-and-butter for 3D transform/color editing.

## 14. ComboBox — searchable/filterable variant
**Complexity:** medium  
Embed an `EditText` at the top of the dropdown that filters the item list as you type.

## 15. Splitter / SplitView
**Complexity:** medium-high  
Two child panels separated by a thin draggable `Resizer`. Distributes available space between the two halves (ratio stored as float). Supports horizontal and vertical split.

## 16. Toast / Notification
**Complexity:** medium-high  
Timed overlay messages that appear in a corner, stack vertically, and fade out after N seconds. Requires a global manager attached to `user_interface`.

## 17. ~~Context Menu (right-click)~~
Already covered by `menu_list` — the widget exists, it's just a matter of spawning it at the cursor position on right-click.

---

---

## Skribidi Integration (Rich Text + Icon Rasterization)

**Goal:** Replace the current font/label system with Skribidi as the text backend, and build an HLSL/SIG code editor widget on top.

**Why:** Skribidi provides cursor navigation, selection, line breaking, BiDi, text attributes (colored spans) and PicoSVG icon rasterization — all things the current `label` / `edit_text` lack. Icons would benefit the whole UI, not just the editor.

### Step 1 — Build the sample *(do this first)*
Clone Skribidi, build its own examples, understand:
- How the context is created and torn down
- How the glyph atlas is structured (format, dirty-region updates)
- How quads/draw commands are emitted
- How cursor and selection APIs work
- How text attributes (color spans) are set per range
- How PicoSVG icon rasterization works

### Step 2 — vcpkg dependencies
Add to `vcpkg.json`: `harfbuzz`, `sheenbidi`, `libunibreak`, `budouxc`.  
Wrap Skribidi itself as a git submodule or vcpkg overlay in `custom-overlay/`.

### Step 3 — D3D12 rendering backend
- `HAL::Texture2D` for the glyph atlas; upload dirty regions each frame
- Textured quad pipeline (UV + per-vertex color) — close to the existing NinePatch pipeline, likely reusable

### Step 4 — C++ module wrapper
`sources/Modules/skribidi/skribidi.ixx` — thin C++ wrapper over the C API, following the existing Modules pattern.

### Step 5 — Replace / extend `label`
New `rich_label` (or updated `label`) backed by Skribidi. Inline icon support in text comes for free here.

### Step 6 — `code_editor` widget
Built on top of Skribidi cursor + selection APIs.
- HLSL tokenizer: keywords, types, semantics, preprocessor, comments, strings, numbers
- SIG tokenizer: simpler subset
- Map token types → Skribidi text attribute colors per span

### Open questions (answer after Step 1)
- Atlas update pattern vs FrameGraph resource lifetime
- API stability risk (early stage, 202 commits)
- Whether existing `Fonts::Font` system is kept alongside or fully replaced

---

## Done
- [x] ColorPicker (RGB + alpha sliders, hex/rgb display, gradient GPU draw)
