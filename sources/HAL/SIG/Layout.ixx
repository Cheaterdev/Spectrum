export module HAL:Layout;

import :RootSignature;
import :Concepts;
import :Types;
import :Device;
import :API.IndirectCommand;

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


	template<class T>
	struct AutoGenIndirectCommand
	{
		HAL::Device& device;
		HAL::IndirectCommand command;
		std::optional<Layouts> layout;


		template<SIG_TYPES::Slot T>
		void process_one()
		{
			//if constexpr (SIG_TYPES::SLOT<T>)
				layout = Layouts::DefaultLayout;  // TODO: make command per each supported layout
		}

		
		template<class T>
		void process_one()
		{
			//if constexpr (SIG_TYPES::SLOT<T>)
			//	layout = Layouts::DefaultLayout;  // TODO: make command per each supported layout
		}

		template< class ...A>
		void process()
		{


			(process_one<A>(), ...);

			if (layout)
				command = HAL::IndirectCommand::template create_command_layout<A...>(device, *layout);
			else
				command = HAL::IndirectCommand::template create_command_layout<A...>(device);
		}

		AutoGenIndirectCommand(HAL::Device& device) :device(device)
		{
			T::for_each(*this);
		}


		HAL::IndirectCommand create_command()
		{
			return command;
		}
	};

}