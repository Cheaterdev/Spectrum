module HAL:API.CommandAllocator;
import Core;

import :Private.CommandListCompiler;
import HAL;

//using namespace HAL;
namespace HAL

{

		CommandAllocator::CommandAllocator(CommandListType type):type(type)
		{
			D3D12_COMMAND_LIST_TYPE t = to_native(type);
			TEST(HAL::Device::get(), HAL::Device::get().get_native_device()->CreateCommandAllocator(t, IID_PPV_ARGS(&m_commandAllocator)));

		}
	void CommandAllocator::reset()
		{
			m_commandAllocator->Reset();
		}


}