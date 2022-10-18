export module HAL:API.Resource;
import d3d12;
import Math;
import Utils;

import :Types;
import :Sampler;

import :Utils;
import :API.Device;
import :Heap;

import stl.core;

import :Format;


export namespace HAL
{
	struct PlacementAddress
	{
		Heap* heap;
		size_t offset;
	};

	class Resource
	{
		GPUAddressPtr address;
	public:
		using ptr = std::shared_ptr<Resource>;
		void init(Device& device, const ResourceDesc& desc, const PlacementAddress& address, ResourceState initialState = ResourceState::GEN_READ);
		void init(D3D::Resource  resource);
		std::span<std::byte> cpu_data();

		GPUAddressPtr get_address()
		{
	
			return address;
		}
	public:
		D3D::Resource native_resource;


	};
}


export
{

	namespace cereal
	{
		template<class Archive>
		void serialize(Archive& ar, HAL::Resource*& g)
		{
		/*	if (g)
			{
				auto desc = g->get_desc();
				auto native_desc = g->native_resource->GetDesc();

				ar& NVP(desc);
				ar& NVP(native_desc);
			}*/
		}
	}
}