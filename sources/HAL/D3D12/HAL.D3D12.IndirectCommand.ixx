export module HAL:API.IndirectCommand;

import :Types;
import :Utils;

import :RootSignature;
import :Slots;

export namespace HAL
{
	class IndirectCommand
	{

		IndirectCommand(D3D::CommandSignature command_signature,const UsedSlots &slots) :command_signature(command_signature),slots(slots)
		{

		}

		
		template<class T>
		static void process_one(UsedSlots& slots)
		{
			if constexpr (HasID<T>)
				slots.merge(T::ID);
		}



	public:
		UsedSlots slots;
		// TODO: make private 
		D3D::CommandSignature command_signature;
		IndirectCommand()
		{

		}

	
		template<class ...Args>
		static IndirectCommand create_command(Device& device, UINT size, RootSignature* layout = nullptr)
		{
			D3D::CommandSignature command_signature;

			D3D12_INDIRECT_ARGUMENT_DESC argumentDescs[] = {
					Args::create_indirect()...
			};

			UsedSlots slots;
			(process_one<Args>(slots), ...);

			D3D12_COMMAND_SIGNATURE_DESC commandSignatureDesc = {};
			commandSignatureDesc.pArgumentDescs = argumentDescs;
			commandSignatureDesc.NumArgumentDescs = _countof(argumentDescs);
			commandSignatureDesc.ByteStride = size;// (0 + ... + sizeof(Underlying<Args>));

			assert(commandSignatureDesc.ByteStride == size);
			TEST(device,device.native_device->CreateCommandSignature(&commandSignatureDesc, layout ? layout->get_native().Get() : nullptr, IID_PPV_ARGS(&command_signature)));

			return IndirectCommand(command_signature,slots);
		}
	template<class ...Args>
		static IndirectCommand create_command_layout(Device& device, UINT size, auto layout)
		{
			return create_command<Args...>(device, size, device.get_engine_pso_holder().GetSignature(layout).get());
		}
	};
}