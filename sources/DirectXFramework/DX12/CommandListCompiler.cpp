#pragma once

#include "pch.h"

namespace DX12
{
	

		void CommandListCompiler::create(CommandListType type)
		{
			D3D12_COMMAND_LIST_TYPE t = static_cast<D3D12_COMMAND_LIST_TYPE>(type);
			Device::get().get_native_device()->CreateCommandAllocator(t, IID_PPV_ARGS(&compiled.m_commandAllocator));
			Device::get().get_native_device()->CreateCommandList(0, t, compiled.m_commandAllocator.Get(), nullptr, IID_PPV_ARGS(&compiled.m_commandList));
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
			TEST(compiled.m_commandList->Reset(compiled.m_commandAllocator.Get(), nullptr));
		}



	void CommandListCompilerDelayed::create(CommandListType type)
	{
		D3D12_COMMAND_LIST_TYPE t = static_cast<D3D12_COMMAND_LIST_TYPE>(type);
		Device::get().get_native_device()->CreateCommandAllocator(t, IID_PPV_ARGS(&compiled.m_commandAllocator));
		Device::get().get_native_device()->CreateCommandList(0, t, compiled.m_commandAllocator.Get(), nullptr, IID_PPV_ARGS(&compiled.m_commandList));
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
		TEST(compiled.m_commandList->Reset(compiled.m_commandAllocator.Get(), nullptr));

		 write_offset = 0;
		 read_offset = 0;
	}
}
