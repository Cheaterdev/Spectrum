module;

import d3d12;
#include <pix3.h>

module HAL:API.CommandList;
import Core;

import :Private.CommandListCompiler;
import HAL;

//using namespace HAL;
namespace HAL

{
	namespace API
	{



	ComPtr<ID3D12GraphicsCommandList4> TransitionCommandList::get_native()
	{
		return m_commandList;
	}


		}


		void TransitionCommandList::create_transition_list(const HAL::Barriers& transitions, std::vector<HAL::Resource*>& discards)
		{
			m_commandAllocator->Reset();
			m_commandList->Reset(m_commandAllocator.Get(), nullptr);

			auto& barriers = transitions.get_barriers();
			if (!barriers.empty())
			{

				std::vector<D3D12_RESOURCE_BARRIER> native(barriers.size());

				for (uint i = 0; i < native.size(); i++)
				{

					auto& e = barriers[i];
					std::visit(overloaded{
				[&](const BarrierUAV& barrier) {
				native[i] = (CD3DX12_RESOURCE_BARRIER::UAV((barrier.resource)->get_dx()));
				},
				[&](const BarrierAlias& barrier) {
					auto native_from = barrier.before ? (barrier.before)->get_dx() : nullptr;
					auto native_to = barrier.after ? (barrier.after)->get_dx() : nullptr;
					native[i] = (CD3DX12_RESOURCE_BARRIER::Aliasing(native_from, native_to));
				},[&](const BarrierTransition& barrier) {
					D3D12_RESOURCE_BARRIER_FLAGS native_flags = D3D12_RESOURCE_BARRIER_FLAGS::D3D12_RESOURCE_BARRIER_FLAG_NONE;

										if (barrier.flags == BarrierFlags::BEGIN) native_flags = D3D12_RESOURCE_BARRIER_FLAGS::D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY;
										if (barrier.flags == BarrierFlags::END) native_flags = D3D12_RESOURCE_BARRIER_FLAGS::D3D12_RESOURCE_BARRIER_FLAG_END_ONLY;

										native[i] = (CD3DX12_RESOURCE_BARRIER::Transition((barrier.resource)->get_dx(),
											static_cast<D3D12_RESOURCE_STATES>(barrier.before),
											static_cast<D3D12_RESOURCE_STATES>(barrier.after),
											barrier.subres == ALL_SUBRESOURCES ? D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES : barrier.subres,
											native_flags));
				},
				[&](auto other) {
					assert(false);
				}
						}, e);



				}

				m_commandList->ResourceBarrier((UINT)native.size(), native.data());
			}

				
			for (auto e : discards)
			{
				//	compiler.DiscardResource(e->get_native().Get(), nullptr);
			}
			m_commandList->Close();
		}



		TransitionCommandList::TransitionCommandList(CommandListType type) :type(type)
		{
			D3D12_COMMAND_LIST_TYPE t = ::to_native(type);

			TEST(HAL::Device::get(), HAL::Device::get().get_native_device()->CreateCommandAllocator(t, IID_PPV_ARGS(&m_commandAllocator)));
			TEST(HAL::Device::get(), HAL::Device::get().get_native_device()->CreateCommandList(0, t, m_commandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_commandList)));
			m_commandList->Close();
			m_commandList->SetName(L"TransitionCommandList");
		}

}