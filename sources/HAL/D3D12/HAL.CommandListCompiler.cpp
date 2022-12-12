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
			D3D12_COMMAND_LIST_TYPE t = to_native(type);
			TEST(HAL::Device::get(), HAL::Device::get().get_native_device()->CreateCommandAllocator(t, IID_PPV_ARGS(&compiled.m_commandAllocator)));
			TEST(HAL::Device::get(), HAL::Device::get().get_native_device()->CreateCommandList(0, t, compiled.m_commandAllocator.Get(), nullptr, IID_PPV_ARGS(&compiled.m_commandList)));
			compiled.m_commandList->Close();
		}


		CommandListCompiled CommandListCompiler::compile()
		{
			compiled.m_commandList->Close();
			return compiled;
		}

		void CommandListCompiler::reset()
		{
			compiled.m_commandAllocator->Reset();
			TEST(HAL::Device::get(), compiled.m_commandList->Reset(compiled.m_commandAllocator.Get(), nullptr));
		}



		void CommandListCompilerDelayed::create(CommandListType type)
		{
			D3D12_COMMAND_LIST_TYPE t = to_native(type);
			TEST(HAL::Device::get(), HAL::Device::get().get_native_device()->CreateCommandAllocator(t, IID_PPV_ARGS(&compiled.m_commandAllocator)));
			TEST(HAL::Device::get(), HAL::Device::get().get_native_device()->CreateCommandList(0, t, compiled.m_commandAllocator.Get(), nullptr, IID_PPV_ARGS(&compiled.m_commandList)));
			compiled.m_commandList->Close();
		}


		CommandListCompiled CommandListCompilerDelayed::compile()
		{
			for (auto t : tasks)
			{
				t();
			}
			tasks.clear();
			compiled.m_commandList->Close();
			return compiled;
		}

		void CommandListCompilerDelayed::reset()
		{
			compiled.m_commandAllocator->Reset();
			TEST(HAL::Device::get(), compiled.m_commandList->Reset(compiled.m_commandAllocator.Get(), nullptr));

			write_offset = 0;
			read_offset = 0;
		}


			void CommandListCompilerDelayed::start_event(std::wstring str)
			{
						PIXBeginEvent(compiled.m_commandList.Get(), 0, str.c_str());
		
			}

			void CommandListCompilerDelayed::end_event()
			{
			PIXEndEvent(compiled.m_commandList.Get());
				
			}

	}
}
