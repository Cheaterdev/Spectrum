export module d3d12;
export import "d3d12_includes.h";

export import <wrl/client.h>;
export using Microsoft::WRL::ComPtr;



export extern "C" { _declspec(dllexport) extern const UINT D3D12SDKVersion = 4; }

export extern "C" { _declspec(dllexport) extern const char* D3D12SDKPath = ".\\D3D12\\"; }