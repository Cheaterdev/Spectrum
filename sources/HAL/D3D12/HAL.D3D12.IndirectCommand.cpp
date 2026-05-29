module HAL:API.IndirectCommand;

import d3d12;
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
