// NVIDIA Streamline — DLSS Super Resolution + DLSS Ray Reconstruction.
//
// Header-unit re-export, same shape as the assimp/magic_enum wrappers.
// The SL headers are self-contained: sl_core_types.h forward-declares
// `typedef struct ID3D12Resource ID3D12Resource;` rather than including
// d3d12.h, so this module does NOT drag the D3D12 headers in and can be
// imported from anywhere.
//
// IMPORTANT — do not call the bare `slInit` / `slEvaluateFeature` / ... that
// sl_core_api.h declares. Spectrum does not link sl.interposer.lib (the DLLs
// live in a `Streamline` subdirectory, and Windows does not search
// subdirectories for static imports), so those declarations have no definition
// and would fail at link time. Use the resolved entry points that the
// `streamline.loader` module hands back instead. The headers ship a
// `PFun_sl*` typedef for every entry point precisely for this case.
export module streamline;

// Via _streamline.h, not the SDK headers directly: the per-feature headers are
// not self-contained and cannot be imported as standalone header units.
export import "_streamline.h";
