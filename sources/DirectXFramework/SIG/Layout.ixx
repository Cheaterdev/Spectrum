module;
#include "pch_dx.h"

export module Graphics:Layout;

import :RootSignature;
import :Concepts;
import :Samplers;
import :Types;

import D3D12.Utils;
export
{

	template<class T>
	struct AutoGenSignatureDesc
	{
		Graphics::RootSignatureDesc desc;


		template<class T>
		void process_one()
		{
		//	if constexpr (TableHasSRV<T>) desc[T::SRV_ID] = Graphics::DescriptorTable(Graphics::DescriptorRange::SRV, Graphics::ShaderVisibility::ALL, 0, -1, T::ID);
		//	if constexpr (TableHasUAV<T>) desc[T::UAV_ID] = Graphics::DescriptorTable(Graphics::DescriptorRange::UAV, Graphics::ShaderVisibility::ALL, 0, T::UAV, T::ID);
		//	if constexpr (TableHasSMP<T>) desc[T::SMP_ID] = Graphics::DescriptorTable(Graphics::DescriptorRange::SAMPLER, Graphics::ShaderVisibility::ALL, 0, T::SMP, T::ID);
		//	desc[T::CB_ID] = Graphics::DescriptorConstBuffer(0, Graphics::ShaderVisibility::ALL, T::ID);
			desc[T::ID] = Graphics::DescriptorConstBuffer(2, Graphics::ShaderVisibility::ALL, T::ID);


			//	if constexpr (T)	desc[T::CB_ID] = Graphics::DescriptorTable(Graphics::DescriptorRange::CBV, Graphics::ShaderVisibility::ALL, 0, T, T::ID);

		}

		template< class ...A>
		void process(std::initializer_list<Graphics::SamplerDesc> samplers)
		{

			(process_one<A>(), ...);
			for (auto& s : samplers)
			{
				desc.add_sampler(0, Graphics::ShaderVisibility::ALL, to_native(s));
			}
		}

		AutoGenSignatureDesc()
		{
			T::for_each(*this);

		}


		Graphics::RootLayout::ptr create_signature(Layouts layout)
		{
			return std::make_shared<Graphics::RootLayout>(desc, layout);
		}
	};

	template<class T>
	void process_one(Graphics::RootSignatureDesc& desc)
	{
		/*if constexpr (HasSRV<T>)
			desc[T::Slot::SRV_ID] = Graphics::DescriptorTable(Graphics::DescriptorRange::SRV, Graphics::ShaderVisibility::ALL, 0, -1, T::Slot::ID);
		if constexpr (HasUAV<T>)
			desc[T::Slot::UAV_ID] = Graphics::DescriptorTable(Graphics::DescriptorRange::UAV, Graphics::ShaderVisibility::ALL, 0, T::Slot::UAV, T::Slot::ID);
		if constexpr (HasSMP<T>)
			desc[T::Slot::SMP_ID] = Graphics::DescriptorTable(Graphics::DescriptorRange::SAMPLER, Graphics::ShaderVisibility::ALL, 0, T::Slot::SMP, T::Slot::ID);*/
		if constexpr (HasSlot<T>)
			desc[T::Slot::ID] = Graphics::DescriptorConstBuffer(2, Graphics::ShaderVisibility::ALL, T::Slot::ID);


		//RTX!!!

		//if constexpr (HasSRV<T> || HasSMP<T> || HasUAV<T>)
		//desc[T::Slot::CB_ID + 1] = Graphics::DescriptorConstBuffer(2, Graphics::ShaderVisibility::ALL, T::Slot::ID);


		// no changes here for now as its done through CB
		//desc[T::Slot::CB_ID + 1] = Graphics::DescriptorConstBuffer(2, Graphics::ShaderVisibility::ALL, T::Slot::ID);

	}

	template< class ...A>
	Graphics::RootSignature::ptr create_local_signature()
	{
		Graphics::RootSignatureDesc desc;

		(process_one<A>(desc), ...);

		return std::make_shared<Graphics::RootSignature>(desc, D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE);
	}


}