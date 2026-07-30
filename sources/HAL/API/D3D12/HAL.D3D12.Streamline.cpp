module;

#include <Windows.h>

module HAL:API.Streamline;

import Core;
import :Utils;
import streamline;

namespace nvidia::API
{
	bool Streamline::resolve_api(void* library)
	{
		if (!library) return false;

		// extern "C", so no name mangling to worry about.
		fn_set_d3d_device = reinterpret_cast<PFun_slSetD3DDevice*>(
			::GetProcAddress(static_cast<HMODULE>(library), "slSetD3DDevice"));

		return fn_set_d3d_device != nullptr;
	}

	bool Streamline::bind_device_native(D3D::Device native_device)
	{
		if (!fn_set_d3d_device || !native_device) return false;

		// slSetD3DDevice's parameter is void* in the SDK (shared with the Vulkan
		// entry point's calling convention); the cast is confined to this line.
		const sl::Result res = fn_set_d3d_device(native_device.Get());
		if (res != sl::Result::eOk)
		{
			Log::get() << (std::string("[Streamline] slSetD3DDevice failed (")
			              + std::to_string((int)res) + ")") << Log::endl;
			return false;
		}
		return true;
	}
}
