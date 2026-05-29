module HAL:API.CommandAllocator;
import Core;
import HAL;

import d3d12;
namespace HAL
{
	CommandAllocator::CommandAllocator(Device& device, const CommandListType type) : device(device), type(type)
	{
		D3D12_COMMAND_LIST_TYPE t = to_native(type);
		TEST(device,
		     device.get_native_device()->CreateCommandAllocator(t, IID_PPV_ARGS(&m_commandAllocator)));
	}

	void CommandAllocator::reset()
	{
		m_commandAllocator->Reset();
	}
}
