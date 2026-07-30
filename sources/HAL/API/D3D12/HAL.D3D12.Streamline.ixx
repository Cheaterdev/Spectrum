// NVIDIA Streamline — D3D12-specific half. nvidia::Streamline (HAL:Streamline)
// owns the API-agnostic part. Split the same way HAL:API.Device / HAL:Device are.
export module HAL:API.Streamline;

import :Utils;
import streamline;

export namespace nvidia
{
	namespace API
	{
		class Streamline
		{
		protected:
			PFun_slSetD3DDevice* fn_set_d3d_device = nullptr;

			// Resolve the backend-specific entry points from an already-loaded
			// sl.interposer module. False if any are missing.
			bool resolve_api(void* library);

			// slSetD3DDevice on the native device. False (and logs) on failure.
			bool bind_device_native(D3D::Device native_device);
		};
	}
}
