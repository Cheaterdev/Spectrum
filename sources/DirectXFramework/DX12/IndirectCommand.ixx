module;
#include "pch_dx.h"
#include "Utils/utils_macros.h"
export module IndirectCommand;

import RootSignature;
import Device;
import d3d12;

export
{

	namespace DX12
	{
		class IndirectCommand
		{
		public:
			ComPtr<ID3D12CommandSignature> command_signature;

			IndirectCommand()
			{

			}

			template<class ...Args>
			static IndirectCommand create_command(UINT size, DX12::RootLayout::ptr layout = nullptr)
			{
				IndirectCommand result;

				D3D12_INDIRECT_ARGUMENT_DESC argumentDescs[] = {
						Args::create_indirect()...
				};

				D3D12_COMMAND_SIGNATURE_DESC commandSignatureDesc = {};
				commandSignatureDesc.pArgumentDescs = argumentDescs;
				commandSignatureDesc.NumArgumentDescs = _countof(argumentDescs);
				commandSignatureDesc.ByteStride = size;// (0 + ... + sizeof(Underlying<Args>));

				assert(commandSignatureDesc.ByteStride == size);
				TEST(Device::get().get_native_device()->CreateCommandSignature(&commandSignatureDesc, layout ? layout->get_native().Get() : nullptr, IID_PPV_ARGS(&result.command_signature)));

				return result;
			}

		};
	}
}