module;


export module Graphics:Layout;

import :RootSignature;
import :Concepts;
import :Samplers;
import :Types;
import :Device;
//import :Utils;
export
{

	template<class T>
	struct AutoGenSignatureDesc
	{
		Graphics::RootSignatureDesc desc;


		template<class T>
		void process_one()
		{
			desc[T::ID] = Graphics::DescriptorConstBuffer(2, Graphics::ShaderVisibility::ALL, T::ID);
		}

		template< class ...A>
		void process(std::initializer_list<Graphics::SamplerDesc> samplers)
		{

			(process_one<A>(), ...);

			int i = 0;
			for (auto& s : samplers)
			{
				desc.set_sampler(i++, 0, Graphics::ShaderVisibility::ALL, s);
			}
		}

		AutoGenSignatureDesc()
		{
			T::for_each(*this);

		}


		Graphics::RootLayout::ptr create_signature(Layouts layout)
		{
			return std::make_shared<Graphics::RootLayout>(Graphics::Device::get(), desc, layout);
		}
	};

	template<class T>
	void process_one(Graphics::RootSignatureDesc& desc)
	{
		if constexpr (HasSlot<T>)
			desc[T::Slot::ID] = Graphics::DescriptorConstBuffer(2, Graphics::ShaderVisibility::ALL, T::Slot::ID);
	}

	template< class ...A>
	Graphics::RootSignature::ptr create_local_signature()
	{
		Graphics::RootSignatureDesc desc;

		(process_one<A>(desc), ...);

		desc.set_type(Graphics::RootSignatureType::Local);

		return std::make_shared<Graphics::RootSignature>(Graphics::Device::get(), desc);
	}


}