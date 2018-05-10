#include "pch.h"
namespace DX11
{
	RenderTargetState::helper RenderTargetState::operator[](unsigned int i)
    {
		changed = true;
        return helper(data[i]);
    }

	void RenderTargetState::set(DX11_DeviceContext context, bool force)
	{
		if (changed){
			context->OMSetRenderTargets(data.size(), data.data(), depth?**depth:nullptr);
		changed = false;
	}
    }

    RenderTargetState::RenderTargetState()
    {
        data = {};
		changed = true;
		depth = nullptr;
    }

    RenderTargetState::helper::helper(ID3D11RenderTargetView* &_data) : data(_data)
    {
    }

}