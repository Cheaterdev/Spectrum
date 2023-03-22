module;
import d3d12;
#include <pix3.h>
module HAL:Private.CommandListCompiler;

import :Device;
import Core;

namespace HAL
{

	
	namespace Private
	{

		void CommandListCompiler::create(CommandListType type)
		{
	assert(false);
		}


		CommandListCompiled CommandListCompiler::compile(CommandAllocator& allocator)
		{
			compiled.m_commandList->Close();
			return compiled;
		}

		void CommandListCompiler::reset()
		{
		assert(false);
		}



		void CommandListCompilerDelayed::create(CommandListType type)
		{
			D3D12_COMMAND_LIST_TYPE t = to_native(type);
			auto ca = Device::get().get_ca(type);
			TEST(HAL::Device::get(), HAL::Device::get().get_native_device()->CreateCommandList(0, t, ca->m_commandAllocator.Get(), nullptr, IID_PPV_ARGS(&compiled.m_commandList)));
			compiled.m_commandList->Close();

			Device::get().free_ca(ca);
		}


		CommandListCompiled CommandListCompilerDelayed::compile(HAL::CommandAllocator&allocator)
		{

			ID3D12CommandAllocator* a =allocator.m_commandAllocator.Get();
		

			TEST(HAL::Device::get(), compiled.m_commandList->Reset(a, nullptr));



			for (auto &t : tasks)
			{
				t();
			}
			tasks.clear();
			compiled.m_commandList->Close();
			return compiled;
		}

		void CommandListCompilerDelayed::reset()
		{
			
			write_offset = 0;
			read_offset = 0;
		}


			void CommandListCompilerDelayed::start_event(std::wstring str)
			{
				tasks.emplace_back([this,str]()
					{
						PIXBeginEvent(compiled.m_commandList.Get(), 0, str.c_str());
					});		
			}

			void CommandListCompilerDelayed::end_event()
			{
				tasks.emplace_back([this]()
					{
						PIXEndEvent(compiled.m_commandList.Get());
					});
				
			}

	}
}
