module HAL:API.CommandAllocator;
import Core;
import HAL;
import <HAL.h>;
import <d3d12/d3d12_includes.h>;
namespace HAL
{
	CommandAllocator::CommandAllocator(CommandListType type) :type(type)
	{
		D3D12_COMMAND_LIST_TYPE t = to_native(type);
		TEST(HAL::Device::get(), HAL::Device::get().get_native_device()->CreateCommandAllocator(t, IID_PPV_ARGS(&m_commandAllocator)));
	}

	void CommandAllocator::reset()
	{
		m_commandAllocator->Reset();
	}
}