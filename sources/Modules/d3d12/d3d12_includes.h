//#include <ddcompiler.h>

#include <directx\d3d12.h>
#include <directx\d3d12shader.h>
#include <directx\d3dx12.h>
#include <directx\dxgiformat.h>

 #include <DWrite.h>
#include <dxgi1_6.h>
#include <DirectXTex.h>
#include <dstorage.h>
#undef OPAQUE

//#include <pix3.h>

#include <dxgidebug.h>

// SDK macros that are pure constants — re-exported as constexpr so that
// named-module consumers (import d3d12;) can see them without macro leakage.
// HRESULT and all SDK types are defined by the headers included above.

#ifdef D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES
#  undef D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES
#endif
export inline constexpr unsigned D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES = 32u;

#ifdef D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING
#  undef D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING
#endif
export inline constexpr unsigned D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING = 0x1688u; // D3D12_ENCODE_SHADER_4_COMPONENT_MAPPING(0,1,2,3)

#ifdef D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT
#  undef D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT
#endif
export inline constexpr unsigned D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT = 512u;

#ifdef D3D12_TEXTURE_DATA_PITCH_ALIGNMENT
#  undef D3D12_TEXTURE_DATA_PITCH_ALIGNMENT
#endif
export inline constexpr unsigned D3D12_TEXTURE_DATA_PITCH_ALIGNMENT = 256u;

#ifdef D3D12_SMALL_RESOURCE_PLACEMENT_ALIGNMENT
#  undef D3D12_SMALL_RESOURCE_PLACEMENT_ALIGNMENT
#endif
export inline constexpr unsigned D3D12_SMALL_RESOURCE_PLACEMENT_ALIGNMENT = 4096u;

#ifdef D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT
#  undef D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT
#endif
export inline constexpr unsigned D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT = 65536u;

#ifdef DXGI_ERROR_NOT_FOUND
#  undef DXGI_ERROR_NOT_FOUND
#endif
export inline constexpr HRESULT DXGI_ERROR_NOT_FOUND = static_cast<HRESULT>(0x887A0002L);

#ifdef DXGI_USAGE_RENDER_TARGET_OUTPUT
#  undef DXGI_USAGE_RENDER_TARGET_OUTPUT
#endif
export inline constexpr unsigned DXGI_USAGE_RENDER_TARGET_OUTPUT = 0x00000020u;

#ifdef DXGI_USAGE_BACK_BUFFER
#  undef DXGI_USAGE_BACK_BUFFER
#endif
export inline constexpr unsigned DXGI_USAGE_BACK_BUFFER = 0x00000040u;

#ifdef DSTORAGE_MAX_QUEUE_CAPACITY
#  undef DSTORAGE_MAX_QUEUE_CAPACITY
#endif
export inline constexpr unsigned DSTORAGE_MAX_QUEUE_CAPACITY = 0x2000u;

export namespace DXGI
{
	inline auto DEBUG_ALL = DXGI_DEBUG_ALL;

	inline auto DEBUG_RLO_ALL = DXGI_DEBUG_RLO_ALL;

}