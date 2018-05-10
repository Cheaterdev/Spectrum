#include "pch.h"

namespace DX11
{

    ConstBufferState::helper ConstBufferState::operator[](unsigned int i)
    {
        if (i <= data.size())
        {
            data.resize(i + 1);
            buffers.resize(i + 1);
        }
		changed = true;
        return helper(data[i], buffers[i]);
    }


    ConstBufferState::ConstBufferState()
    {
        data = {};
		changed = false;
    }

    ConstBufferState::helper::helper(ID3D11Buffer* &_data, BufferBase* &_buffer) : data(_data), buffer(_buffer)
    {
    }

}