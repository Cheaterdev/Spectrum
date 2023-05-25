export module HAL:API.Resource;
import d3d12;
import Core;

import :Types;
import :Sampler;

import :Utils;
import :API.Device;
import :Heap;


import :Format;


export namespace HAL
{
	struct PlacementAddress
	{
		Heap* heap;
		size_t offset;
	};

	namespace API
	{
		
	
	class Resource:public Object
	{
		GPUAddressPtr address;
	public:
		using ptr = std::shared_ptr<Resource>;
		void init(Device& device, const ResourceDesc& desc, const PlacementAddress& address, TextureLayout initialLayout = TextureLayout::COMMON);
		void init(D3D::Resource  resource);

		GPUAddressPtr get_address()
		{
	
			return address;
		}
	public:
		D3D::Resource native_resource;

		
			auto get_dx() const
			{
				return native_resource.Get();
			}
	};
	}
}


export
{

	namespace cereal
	{
		template<class Archive>
		void serialize(Archive& ar, HAL::API::Resource*& g)
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