// Skribidi — bidi text layout, editing and glyph/icon rasterization
// (custom-overlay/skribidi). Header-unit re-export, same shape as the
// nrd/magic_enum wrappers; the C API is exposed as-is, RAII lives with the
// engine-side text system that owns these objects.
export module skribidi;

export import "_skribidi.h";
