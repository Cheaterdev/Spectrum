module HAL:Resource.Buffer;

import :FrameManager;
import wrl;
import d3d12;
import Core;

#undef THIS

namespace HAL
{

    std::span<std::byte> Buffer::cpu_data()const
    {

        return { buffer_data,buffer_data + get_size() };
    }

    Buffer::~Buffer()
    {
        if (buffer_data)
        {
            get_dx()->Unmap(0, nullptr);
        }

    }

    std::byte* ResourceAddress::get_cpu_data() const
    {
        return resource->cpu_data().data() + resource_offset;
    }

}

HAL::GPUAddressPtr to_native(const HAL::ResourceAddress& address)
{
    return address.resource ? (address.resource->get_address() + address.resource_offset) : 0;
}