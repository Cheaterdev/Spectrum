export module HAL:Layout;

import :RootSignature;
import :Concepts;
import :Types;

export
{

	template<class T>
	struct AutoGenSignatureDesc
	{
		HAL::RootSignatureDesc desc;


		template<class T>
		void process_one()
		{
			desc[T::ID] = HAL::DescriptorConstBuffer(2, HAL::ShaderVisibility::ALL, T::ID);
		}

		template< class ...A>
		void process(std::initializer_list<HAL::SamplerDesc> samplers)
		{

			(process_one<A>(), ...);

			int i = 0;
			for (auto& s : samplers)
			{
				desc.set_sampler(i++, 0, HAL::ShaderVisibility::ALL, s);
			}
		}

		AutoGenSignatureDesc()
		{
			T::for_each(*this);

		}


		HAL::RootLayout::ptr create_signature(Layouts layout)
		{
			return std::make_shared<HAL::RootLayout>(HAL::Device::get(), desc, layout);
		}
	};

	template<class T>
	void process_one(HAL::RootSignatureDesc& desc)
	{
		if constexpr (HasSlot<T>)
			desc[T::Slot::ID] = HAL::DescriptorConstBuffer(2, HAL::ShaderVisibility::ALL, T::Slot::ID);
	}

	template< class ...A>
	HAL::RootSignature::ptr create_local_signature()
	{
		HAL::RootSignatureDesc desc;

		(process_one<A>(desc), ...);

		desc.set_type(HAL::RootSignatureType::Local);

		return std::make_shared<HAL::RootSignature>(HAL::Device::get(), desc);
	}


}