export module HAL:API.IndirectCommand;
import <HAL.h>;
import <d3d12/d3d12_includes.h>;
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
		static void process_one(UsedSlots& slots, uint& total_size)
		{
			if constexpr (HasID<T>)
			{
				slots.merge(T::ID);
				total_size+=sizeof(uint);
			}else
					total_size+=sizeof(Underlying<T>);
				
		}



	public:
		UsedSlots slots;
		// TODO: make private 
		D3D::CommandSignature command_signature;
		IndirectCommand()
		{

		}

	
		template<class ...Args>
		static IndirectCommand create_command(Device& device,  RootSignature* layout = nullptr)
		{
			D3D::CommandSignature command_signature;

			D3D12_INDIRECT_ARGUMENT_DESC argumentDescs[] = {
					Args::create_indirect()...
			};

			UsedSlots slots;
			uint total_size = 0;
			(process_one<Args>(slots,total_size), ...);

			D3D12_COMMAND_SIGNATURE_DESC commandSignatureDesc = {};
			commandSignatureDesc.pArgumentDescs = argumentDescs;
			commandSignatureDesc.NumArgumentDescs = _countof(argumentDescs);
			commandSignatureDesc.ByteStride = total_size;//(0 + ... + sizeof(Underlying<Args>));

		//	assert(commandSignatureDesc.ByteStride == size);
			TEST(device,device.native_device->CreateCommandSignature(&commandSignatureDesc, layout ? layout->get_native().Get() : nullptr, IID_PPV_ARGS(&command_signature)));

			return IndirectCommand(command_signature,slots);
		}

		template<class ...Args>
		static IndirectCommand create_command_layout(Device& device)
		{
			return create_command<Args...>(device, nullptr);
		}

		template<class ...Args>
		static IndirectCommand create_command_layout(Device& device,  auto layout)
		{
			return create_command<Args...>(device,  device.get_engine_pso_holder().GetSignature(layout).get());
		}
	};
}