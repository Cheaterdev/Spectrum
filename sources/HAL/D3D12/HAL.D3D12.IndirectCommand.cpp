module HAL:API.IndirectCommand;
import <HAL.h>;
import <d3d12/d3d12_includes.h>;
import :Types;
import :Utils;
import :RootSignature;
import :Slots;

namespace HAL
{
	IndirectCommand::IndirectCommand(D3D::CommandSignature command_signature, const UsedSlots& slots)
		: slots(slots), command_signature(command_signature)
	{
	}

	IndirectCommand::IndirectCommand()
	{
	}
}
