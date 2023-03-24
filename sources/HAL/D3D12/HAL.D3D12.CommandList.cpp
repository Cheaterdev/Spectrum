module HAL:API.CommandList;
import Core;

import :Device;
import :FrameManager;
import :CommandAllocator;
namespace HAL
{
	namespace API
	{

		void CommandList::create(CommandListType type)
		{
			this->type=type;
		}

		void CommandList::begin(HAL::CommandAllocator& allocator)
		{
			if (!m_commandList)
			{
				TEST(HAL::Device::get(), HAL::Device::get().get_native_device()->CreateCommandList(0, to_native(type), allocator.m_commandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_commandList)));
			}
			else
			{
				TEST(HAL::Device::get(), m_commandList->Reset(allocator.m_commandAllocator.Get(), nullptr));
			}

			native_topology = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
		}

		void CommandList::end()
		{
			m_commandList->Close();
		}
	}
}