// NVIDIA Streamline — Vulkan half, stubbed. resolve_api() reports failure, so
// nvidia::Streamline never initializes and available() stays false: DLSS and
// DLSS-RR simply don't enable on this backend. A real port would resolve
// slSetVulkanInfo here and bind the VkInstance/VkPhysicalDevice/VkDevice.
export module HAL:API.Streamline;

import :Utils;
import vulkan;

export namespace nvidia
{
	namespace API
	{
		class Streamline
		{
		public:
			using NativeDevice = VkDevice;

		protected:
			bool resolve_api(void*) { return false; }
			bool bind_device_native(NativeDevice) { return false; }
		};
	}
}
