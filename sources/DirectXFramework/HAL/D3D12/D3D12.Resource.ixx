export module HAL.Resource;
import d3d12;
import Vectors;
import Utils;

import HAL.Types;
import HAL.Sampler;

import D3D12.Types;
import HAL.Device;
import HAL.Heap;

using namespace HAL;


export namespace HAL
{

	enum class ResourceDimension :uint
	{
		BUFFER,
		TEXTURE
	};
	
	struct TextureDesc
	{
		Vector<vec3_t<uint, false>> Dimensions;
		uint ArraySize;
		DXGI_FORMAT Format;
	};

	struct BufferDesc
	{
		size_t SizeInBytes;
	};

	struct ResourceDesc
	{
		ResourceDimension Type;

		union
		{
			struct
			{
			//	Vector<vec3_t<uint, false>> Dimensions;
				uint ArraySize;
				DXGI_FORMAT Format;
			};
			struct
			{
				BufferDesc Buffer;
			};
			
		};

	};

	struct PlacementAddress
	{
		Heap::ptr heap;
		size_t offset;
	};

	class Resource
	{
		ResourceDesc desc;
	public:
		using ptr = std::shared_ptr<Resource>;
		Resource(Device& device, const TextureDesc& desc, const PlacementAddress &address, ResourceState initialState = ResourceState::GEN_READ);
		Resource(Device& device, const BufferDesc& desc, const PlacementAddress& address, ResourceState initialState = ResourceState::GEN_READ);

		std::span<std::byte> cpu_data();

	public:
		D3D::Resource native_resource;
	};
}

module:private;


namespace HAL
{
	Resource::Resource(Device& device, const BufferDesc& bufferDesc, const PlacementAddress& address, ResourceState initialState)
	{
		desc.Type = ResourceDimension::BUFFER;
		desc.Buffer = bufferDesc;

		CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferDesc.SizeInBytes);
		device.native_device->CreatePlacedResource(
			address.heap->native_heap.Get(),
			0,
			&resourceDesc,
			static_cast<D3D12_RESOURCE_STATES>(initialState),
			nullptr,
			IID_PPV_ARGS(&native_resource));
	}

	

}