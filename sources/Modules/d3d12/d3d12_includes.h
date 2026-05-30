#include <directx\d3d12.h>
#include <directx\d3d12shader.h>
#include <directx\d3dx12.h>
#include <directx\dxgiformat.h>

#include <DWrite.h>
#include <dxgi1_6.h>
#include <DirectXTex.h>
#include <dstorage.h>


//#include <pix3.h>

#include <dxgidebug.h>

export namespace D3D12
{
	// Simple integer macros — safe to use as constexpr initializers
	inline constexpr unsigned SHADER_IDENTIFIER_SIZE_IN_BYTES      = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
	inline constexpr unsigned TEXTURE_DATA_PLACEMENT_ALIGNMENT     = D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT;
	inline constexpr unsigned TEXTURE_DATA_PITCH_ALIGNMENT         = D3D12_TEXTURE_DATA_PITCH_ALIGNMENT;
	inline constexpr unsigned SMALL_RESOURCE_PLACEMENT_ALIGNMENT   = D3D12_SMALL_RESOURCE_PLACEMENT_ALIGNMENT;
	inline constexpr unsigned DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
	// Complex macro (D3D12_ENCODE_SHADER_4_COMPONENT_MAPPING bitwise expression) — use literal
	inline constexpr unsigned DEFAULT_SHADER_4_COMPONENT_MAPPING   = 0x1688u;
}

#undef ERROR_NOT_FOUND   // winerror.h defines this as Win32 error code 1168 — would mangle the identifier
export namespace DXGI
{
	// _HRESULT_TYPEDEF_ and (1L<<n) macros are not constexpr-safe — use literals
	inline constexpr HRESULT  ERROR_NOT_FOUND            = static_cast<HRESULT>(0x887A0002L);
	inline constexpr unsigned USAGE_RENDER_TARGET_OUTPUT = 0x00000020u;
	inline constexpr unsigned USAGE_BACK_BUFFER          = 0x00000040u;
	// GUIDs — not constexpr, copy by value from the SDK extern
	inline auto               DEBUG_ALL                  = DXGI_DEBUG_ALL;
	inline auto               DEBUG_RLO_ALL              = DXGI_DEBUG_RLO_ALL;
}

export namespace DStorage
{
	inline constexpr unsigned MAX_QUEUE_CAPACITY = DSTORAGE_MAX_QUEUE_CAPACITY;
}