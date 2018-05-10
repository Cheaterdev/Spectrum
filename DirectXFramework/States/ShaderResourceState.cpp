#include "pch.h"

namespace DX11
{
	ShaderResourceState::helper ShaderResourceState::operator[](unsigned int i)
	{
		changed = true;
		return helper(data[i]);
	}

	
	ShaderResourceState::ShaderResourceState()
	{
		data = {};
		changed = true;
	}

	ShaderResourceState::helper::helper(ID3D11ShaderResourceView* &_data) : data(_data)
	{
	}

}