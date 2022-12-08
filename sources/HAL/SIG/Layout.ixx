export module HAL:Layout;

import :RootSignature;
import :Concepts;
import :Types;
import :Device;

export
{

	template<class T>
	struct AutoGenSignatureDesc
	{
		HAL::RootSignatureDesc desc;


		template<class T>
		void process_one()
		{
			desc[T::ID] = HAL::DescriptorConstants(2, 1, HAL::ShaderVisibility::ALL, T::ID);
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


		HAL::RootLayout::ptr create_signature(HAL::Device& device, Layouts layout)
		{
			return std::make_shared<HAL::RootLayout>(device, desc, layout);
		}
	};

	template<class T>
	void process_one(HAL::RootSignatureDesc& desc)
	{
		if constexpr (HasSlot<T>)
			desc[T::Slot::ID] = HAL::DescriptorConstants(2, 1, HAL::ShaderVisibility::ALL, T::Slot::ID);
	}

	template< class ...A>
	HAL::RootSignature::ptr create_local_signature(HAL::Device& device)
	{
		HAL::RootSignatureDesc desc;

		(process_one<A>(desc), ...);

		desc.set_type(HAL::RootSignatureType::Local);

		return std::make_shared<HAL::RootSignature>(device, desc);
	}


}