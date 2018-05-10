#include "pch.h"

namespace DX11
{
	SamplerState::helper SamplerState::operator[](unsigned int i)
	{
		changed = true;
		return helper(data[i]);
	}

	SamplerState::SamplerState()
	{
		data = {};
		changed = true;
	}

	SamplerState::helper::helper(ID3D11SamplerState* &_data) : data(_data)
	{
	}

}

