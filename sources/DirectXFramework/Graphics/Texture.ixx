module;
#include "helper.h"
//#include "dx12_types.h"
export module Graphics:Texture;

import serialization;
import :CommandList;
import ResourceManager;

import :Resource;
import :Device;
import :TextureData;
import :Descriptors;
import :HLSLDescriptors;

//using namespace HAL;

import :Types;
import :Definitions;
export
{
namespace Graphics
{

    struct texure_header
    {
            std::filesystem::path name;
            bool force_srgb;
            bool mips = true;
            texure_header() = default;
			virtual ~texure_header() = default;

            texure_header(std::filesystem::path name, bool force_srgb = false, bool mips = true);
			auto  operator<=>(const  texure_header& r)  const = default;
        private:
			SERIALIZE()
            {
                ar& NVP(name)&NVP(force_srgb)&NVP(mips);
            }
    };
 
	enum class PixelSpace : int {
		NONE,
		MAKE_LINERAR,
		MAKE_SRGB
	};

	class View
	{
		protected:
		HandleTable rtvs;
		HandleTable srvs;
		HandleTable uavs;

		HandleTable static_srv;
		HandleTable static_uav;

		 Resource* resource;
		 View( Resource* resource):resource(resource)
		 {
			 
		 }

		virtual Handle get_srv() = 0;
	};

	class Texture2DView:public View
	{
		HandleTable hlsl;
	public:
		HLSL::Texture2D<> texture2D;
		std::vector<HLSL::RWTexture2D<>> rwTexture2D;
		std::vector<HLSL::Texture2D<>> texture2DMips;
	private:
		int single_count;
		std::vector<Viewport> p;
		std::vector<sizer_long> scissor;
		//     std::vector<HandleTable> array_rtv;

		int array_index = 0;
	

	public:
		using ptr = std::shared_ptr<Texture2DView>;
		Texture2DView( Resource* _resource, HandleTable t);
		Texture2DView( Resource* _resource, int array_index = -1);

		Handle get_rtv(UINT mip = 0);

		void place_srv(Handle h);
		void place_srv(Handle h, UINT mip);

		void place_rtv(Handle h, int i = 0);


		void place_dsv(Handle h, UINT mip = 0);

		void place_uav(Handle h, UINT mip = 0, UINT slice = 0);


		std::function<void(Handle)>uav(UINT mip = 0, UINT slice = 0);


		std::function<void(Handle)>srv(PixelSpace space = PixelSpace::NONE);

		std::function<void(Handle)>srv(UINT mip, UINT levels = 1);

		std::function<void(Handle)>rtv(UINT mip);

		Handle get_srv(UINT mip);
		Handle get_srv();
		Handle get_static_srv();

		Handle get_static_uav();
		Handle get_uav(UINT mip = 0);

		Viewport get_viewport(UINT mip = 0);

		sizer_long get_scissor(UINT mip = 0);
		/*
		const HandleTable& get_rtv(UINT index, UINT mip)
		{
		return rtv[mip][index];
		}

		const HandleTable& get_rtv(UINT mip)
		{
		return array_rtv[mip];
		}*/
	};

	class CubemapView :public View
	{

		int single_count;
		std::vector<Viewport> p;
		std::vector<sizer_long> scissor;
		std::array<Texture2DView::ptr, 6> faces;
		//     std::vector<HandleTable> array_rtv;
	public:
		using ptr = std::shared_ptr<CubemapView>;
		CubemapView(Resource* _resource, int offset = 0);

		Handle get_rtv(UINT index, UINT mip);
		Handle get_srv(UINT mip);

		Handle get_srv();
		void place_srv(Handle h);

		void place_srv(Handle h, UINT mip);

		Texture2DView::ptr face(int i)
		{
			return  faces[i];
		}
			Viewport get_viewport(UINT mip = 0);
			sizer_long get_scissor(UINT mip = 0);
			Handle get_static_srv();
            /*
            const HandleTable& get_rtv(UINT index, UINT mip)
            {
                return rtv[mip][index];
            }

            const HandleTable& get_rtv(UINT mip)
            {
                return array_rtv[mip];
            }*/
    };

	class CubemapArrayView :public View
	{
		
	
		std::vector<CubemapView::ptr> views;
		int offset = 0;
		//     std::vector<HandleTable> array_rtv;

	
	public:
		using ptr = std::shared_ptr<CubemapArrayView>;
		CubemapArrayView(Resource* _resource);


		CubemapView::ptr cubemap(int i)
		{
			return views[i];
		}
		Handle get_srv(UINT i)
		{
			return srvs[1 + i];
		}

		Handle get_srv()
		{
			return srvs[0];
		}


		void place_srv(Handle h);
		void place_srv(Handle h, UINT mip);

	

		/*
		const HandleTable& get_rtv(UINT index, UINT mip)
		{
		return rtv[mip][index];
		}

		const HandleTable& get_rtv(UINT mip)
		{
		return array_rtv[mip];
		}*/
	};

    class Array2DView :public View
    {
		
	private:
            int single_count;

			std::vector<Texture2DView::ptr> views;
        public:
            using ptr = std::shared_ptr<Array2DView>;

            Array2DView(Resource* _resource);

			Handle get_rtv(UINT index, UINT mip);

			void place_srv(Handle h);


			void place_dsv(Handle h, UINT mip = 0, UINT slice = 0, UINT slice_count = 1);

			void place_uav(Handle h, UINT first = 0, UINT count = 0, UINT mip = 0, UINT slice = 0);



			Texture2DView::ptr tex2d(int i)
			{
				return views[i];
			}

			std::function<void(Handle)> uav(UINT first = 0, UINT count = 0, UINT mip = 0);

			Handle get_srv();

			Handle get_uav();
			Handle get_static_srv();

			Handle get_static_uav();
            /*
            const HandleTable& get_rtv(UINT index, UINT mip)
            {
            return rtv[mip][index];
            }

            const HandleTable& get_rtv(UINT mip)
            {
            return array_rtv[mip];
            }*/
    };


    class Texture3DView :public View
    {

		HandleTable hlsl;
	public:
		HLSL::Texture3D<float4> texture3D;
		std::vector<HLSL::RWTexture3D<>> rwTexture3D;
		std::vector<HLSL::Texture3D<>> texture3DMips;
	private:
            int single_count;
            //     std::vector<HandleTable> array_rtv;
        public:

	

            using ptr = std::shared_ptr<Texture3DView>;
            Texture3DView(Resource* _resource);

			Handle get_rtv(UINT index, UINT mip);

			void place_srv(Handle h, int level = -1);



			void place_uav(Handle h, int level = 0);
			Handle get_srv(int i);
			Handle get_uav(int i = 0);
			Handle get_static_srv();
			Handle get_srv();


			Handle get_static_uav();
			std::function<void(Handle)>srv(int level = -1, int levels = -1);

			std::function<void(Handle)>uav(int level = 0);
            /*
            const HandleTable& get_rtv(UINT index, UINT mip)
            {
            return rtv[mip][index];
            }

            const HandleTable& get_rtv(UINT mip)
            {
            return array_rtv[mip];
            }*/
    };


    class Texture : public resource_manager<Texture, texure_header>, public Resource
    {
            friend class resource_manager<Texture, texure_header>;
        protected:

            Texture();

            Texture(void*);


			Format original_format;

            CubemapView::ptr cubemap_view;
            Array2DView::ptr array_2d_view;
            Texture2DView::ptr texture_2d_view;
            Texture3DView::ptr texture_3d_view;
			CubemapArrayView::ptr array_cubemap_view;
            void init();
        public:
            using ptr = s_ptr<Texture>;

			ivec3 get_size(int mip = 0);
			CubemapView::ptr& cubemap();

			Texture2DView::ptr& texture_2d();

			Texture3DView::ptr& texture_3d();

			Array2DView::ptr& array2d();

			CubemapArrayView::ptr & array_cubemap()
			{
				return array_cubemap_view;
			}
            static ptr load_native(const texure_header& header, resource_file_depender& depender);

            bool is_rt();


            static const ptr null;
          //  CD3DX12_RESOURCE_DESC get_desc() const;


            Texture(ComPtr<ID3D12Resource> native, HandleTable rtv, ResourceState state);
            Texture(ComPtr<ID3D12Resource> native, ResourceState state);
            Texture(CD3DX12_RESOURCE_DESC desc, ResourceState state = ResourceState::PIXEL_SHADER_RESOURCE, HeapType heap_type = HeapType::DEFAULT, std::shared_ptr<texture_data> data = nullptr);


			texture_data::ptr get_data() const;

			void upload_data(texture_data::ptr);
			Texture::ptr compress();

			SERIALIZE()
            {

				if constexpr(Archive::is_saving::value)
				{
					auto data = get_data();
					ar& NVP(data);
				}
				else
				{
					texture_data::ptr data;
					ar& NVP(data);
					upload_data(data);
				}
            }
    };
}


}