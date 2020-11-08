#pragma once
namespace DX12
{

	class ResourceView
	{
	protected:
		SRVHandle srv;
		UAVHandle uav;
		RTVHandle rtv;
		DSVHandle dsv;
		Handle cb;
		UAVHandle uav_clear;
	protected:
		ResourceViewDesc view_desc;

	public:
		Resource::ptr resource; //////////////
		ResourceView()
		{

		}

		auto get_desc()
		{
			return resource->get_desc();
		}

		ResourceView(Resource::ptr resource) :resource(resource)
		{

		}

		void init_desc()
		{
			auto& desc = resource->get_desc();

			view_desc.format = desc.Format;

			if (desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
			{
				view_desc.type = ResourceType::BUFFER;
				view_desc.Buffer.Offset = 0;
				view_desc.Buffer.Size = desc.Width;
				view_desc.Buffer.Stride = 0;
			}

			if (desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE1D)
			{
				view_desc.type = ResourceType::TEXTURE1D;
			}

			if (desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)
			{



				{
					view_desc.type = ResourceType::TEXTURE2D;
					view_desc.Texture2D.ArraySize = desc.ArraySize();
					view_desc.Texture2D.MipSlice = 0;
					view_desc.Texture2D.FirstArraySlice = 0;
					view_desc.Texture2D.PlaneSlice = 0;
					view_desc.Texture2D.MipLevels = desc.MipLevels;
				}





			}

			if (desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D)
			{
				view_desc.type = ResourceType::TEXTURE3D;

				view_desc.Texture2D.ArraySize = 1;
				view_desc.Texture2D.MipSlice = 0;
				view_desc.Texture2D.FirstArraySlice = 0;
				view_desc.Texture2D.PlaneSlice = 0;
				view_desc.Texture2D.MipLevels = desc.MipLevels;
			}


		}
		void init_views(FrameResources& frame);

		virtual void place_srv(Handle& h) { assert(false); }
		virtual void place_uav(Handle& h) { assert(false); }
		virtual void place_rtv(Handle& h) { assert(false); }
		virtual void place_dsv(Handle& h) { assert(false); }
		virtual void place_cb(Handle& h) { assert(false); }


		Handle get_srv() { return srv; }
		Handle get_uav() { return uav; }
		Handle get_uav_clear() { return uav_clear; }
		Handle get_rtv() { return rtv; }
		Handle get_dsv() { return dsv; }
		Handle get_cb() { return cb; }

		operator bool() const
		{
			return !!resource;
		}
	};

	class RTXSceneView :public ResourceView
	{
	public:
		RTXSceneView() = default;
		RTXSceneView(Resource::ptr resource, FrameResources& frame) :ResourceView(resource)
		{
			init_desc();
			init_views(frame);
		}

		virtual void place_srv(Handle& h) override;
		virtual void place_uav(Handle& h) { }
		virtual void place_rtv(Handle& h) { assert(false); }
		virtual void place_dsv(Handle& h) { assert(false); }
		virtual void place_cb(Handle& h) { assert(false); }

	};
	class TextureView :public ResourceView
	{

	public:
		TextureView() = default;
		TextureView(Resource::ptr resource, FrameResources& frame, bool cube = false) :ResourceView(resource)
		{
			init_desc();

			auto& desc = resource->get_desc();

			if (cube && desc.ArraySize() % 6 == 0)
			{

				view_desc.type = ResourceType::CUBE;
				view_desc.Texture2D.ArraySize = desc.ArraySize() / 6;
				view_desc.Texture2D.MipSlice = 0;
				view_desc.Texture2D.FirstArraySlice = 0;
				view_desc.Texture2D.PlaneSlice = 0;
				view_desc.Texture2D.MipLevels = desc.MipLevels;
			}


			init_views(frame);
		}

		TextureView(Resource::ptr resource, FrameResources& frame, ResourceViewDesc desc) :ResourceView(resource)
		{

			view_desc = desc;
			init_views(frame);
		}

		virtual void place_srv(Handle& h) override;
		virtual void place_uav(Handle& h)  override;
		virtual void place_rtv(Handle& h)  override;
		virtual void place_dsv(Handle& h)  override;
		virtual void place_cb(Handle& h) { assert(false); }



		Viewport get_viewport()
		{

			auto desc = resource->get_desc();
			UINT scaler = 1 << view_desc.Texture2D.MipSlice;


			Viewport p;
			p.Width = std::max(1.0f, static_cast<float>(resource->get_desc().Width / scaler));
			p.Height = std::max(1.0f, static_cast<float>(resource->get_desc().Height / scaler));
			p.TopLeftX = 0;
			p.TopLeftY = 0;
			p.MinDepth = 0;
			p.MaxDepth = 1;

			return p;
		}


		sizer_long get_scissor()
		{
			auto desc = resource->get_desc();
			UINT scaler = 1 << view_desc.Texture2D.MipSlice;


			return { 0,0, std::max(1ull,desc.Width / scaler),std::max(1u,desc.Height / scaler) };
		}
		UINT get_mip_count()
		{
			return view_desc.Texture2D.MipLevels;
		}

		ivec2 get_size()
		{
			auto desc = resource->get_desc();
			UINT scaler = 1 << view_desc.Texture2D.MipSlice;


			return { std::max(1ull,desc.Width / scaler),std::max(1u,desc.Height / scaler) };
		}

		TextureView create_2d_slice(UINT slice, FrameResources& frame)
		{
			ResourceViewDesc desc = view_desc;
			desc.type = ResourceType::TEXTURE2D;
			desc.Texture2D.ArraySize = 1;
			desc.Texture2D.FirstArraySlice = slice;

			return TextureView(resource, frame, desc);
		}

		TextureView create_mip(UINT mip, FrameResources& frame)
		{
			ResourceViewDesc desc = view_desc;

			desc.Texture2D.MipSlice += mip;
			desc.Texture2D.MipLevels = 1;

			return TextureView(resource, frame, desc);
		}


	};


	class BufferView : public ResourceView
	{

	public:
		BufferView() = default;

		BufferView(Resource::ptr resource, FrameResources& frame) :ResourceView(resource)
		{
			init_desc();
			init_views(frame);
		}

		~BufferView()
		{

		}
		virtual void place_srv(Handle& h) override;
		virtual void place_uav(Handle& h) override;
		virtual void place_rtv(Handle& h) { assert(false); }
		virtual void place_dsv(Handle& h) { assert(false); }
		virtual void place_cb(Handle& h)override;
		template<class T>
		void write(UINT64 offset, T* data, UINT64 count)
		{
			memcpy(resource->buffer_data + offset, data, sizeof(T) * count);
		}


	};



	template<class T>
	class StructuredBufferView :public ResourceView
	{

	public:
		StructuredBufferView() = default;

		StructuredBufferView(Resource::ptr resource, FrameResources& frame, UINT offset = 0, UINT64 size = 0) :ResourceView(resource)
		{
			init_desc();
			view_desc.Buffer.Offset = offset;
			if (size) view_desc.Buffer.Size = size;

			init_views(frame);
		}

		~StructuredBufferView()
		{

		}
		virtual void place_srv(Handle& h) override {
			if (!resource) return;

			D3D12_SHADER_RESOURCE_VIEW_DESC  desc = {};
			desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
			desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			desc.Format = DXGI_FORMAT_UNKNOWN;

			desc.Buffer.StructureByteStride = sizeof(T);
			desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

			if (desc.Buffer.StructureByteStride > 0) {
				desc.Buffer.NumElements = static_cast<UINT>(view_desc.Buffer.Size / desc.Buffer.StructureByteStride);
				desc.Buffer.FirstElement = view_desc.Buffer.Offset / desc.Buffer.StructureByteStride;

			}
			else {
				desc.Buffer.NumElements = 0;
				desc.Buffer.FirstElement = 0;

			}
			Device::get().create_srv(h, resource.get(), desc);



		}

		virtual void place_uav(Handle& h) {

			if (!resource) return;
			{
				D3D12_UNORDERED_ACCESS_VIEW_DESC  desc = {};
				desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;

				desc.Format = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;

				desc.Buffer.StructureByteStride = sizeof(T);
				desc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;


				desc.Buffer.NumElements = static_cast<UINT>(view_desc.Buffer.Size / sizeof(T));
				desc.Buffer.FirstElement = view_desc.Buffer.Offset / sizeof(T);
				desc.Buffer.CounterOffsetInBytes = 0;

				Device::get().create_uav(h, resource.get(), desc);
			}

			{

				D3D12_UNORDERED_ACCESS_VIEW_DESC  desc = {};
				desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;

				desc.Format = DXGI_FORMAT::DXGI_FORMAT_R8_UINT;

				desc.Buffer.StructureByteStride = 0;
				desc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;


				desc.Buffer.NumElements = static_cast<UINT>(view_desc.Buffer.Size / sizeof(char));
				desc.Buffer.FirstElement = view_desc.Buffer.Offset / sizeof(char);
				desc.Buffer.CounterOffsetInBytes = 0;
				Device::get().create_uav(uav_clear, resource.get(), desc);

			}




		}
		virtual void place_rtv(Handle& h) { assert(false); }
		virtual void place_dsv(Handle& h) { assert(false); }
		virtual void place_cb(Handle& h)override {
			if (!resource) return;

			D3D12_CONSTANT_BUFFER_VIEW_DESC  desc = {};
			desc.BufferLocation = resource->get_gpu_address();
			desc.SizeInBytes = view_desc.Buffer.Size;
			assert(desc.SizeInBytes < 65536);

			Device::get().create_cbv(h, resource.get(), desc);
		}
		void write(UINT64 offset, T* data, UINT64 count)
		{
			memcpy(resource->buffer_data + offset, data, sizeof(T) * count);
		}
	};


	template<class T, DXGI_FORMAT format>
	class FormattedBufferView :public ResourceView
	{

	public:
		FormattedBufferView() = default;

		FormattedBufferView(Resource::ptr resource, FrameResources& frame, UINT offset = 0, UINT64 size = 0) :ResourceView(resource)
		{
			init_desc();
			view_desc.Buffer.Offset = offset;
			if (size) view_desc.Buffer.Size = size;

			init_views(frame);
		}

		~FormattedBufferView()
		{

		}
		virtual void place_srv(Handle& h) override {
			if (!resource) return;

			D3D12_SHADER_RESOURCE_VIEW_DESC  desc = {};
			desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
			desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			desc.Format = format;

			desc.Buffer.StructureByteStride = 0;
			desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;


			desc.Buffer.NumElements = static_cast<UINT>(view_desc.Buffer.Size / sizeof(T));
			desc.Buffer.FirstElement = view_desc.Buffer.Offset / sizeof(T);

			Device::get().create_srv(h, resource.get(), desc);


		}

		virtual void place_uav(Handle& h) {

			if (!resource) return;

			D3D12_UNORDERED_ACCESS_VIEW_DESC  desc = {};
			desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;

			desc.Format = format;

			desc.Buffer.StructureByteStride = 0;
			desc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;


			desc.Buffer.NumElements = static_cast<UINT>(view_desc.Buffer.Size / sizeof(T));
			desc.Buffer.FirstElement = view_desc.Buffer.Offset / sizeof(T);
			desc.Buffer.CounterOffsetInBytes = 0;

			Device::get().create_uav(h, resource.get(), desc);
			Device::get().create_uav(uav_clear, resource.get(), desc);


		}
		virtual void place_rtv(Handle& h) { assert(false); }
		virtual void place_dsv(Handle& h) { assert(false); }
		virtual void place_cb(Handle& h)override {
			if (!resource) return;

			D3D12_CONSTANT_BUFFER_VIEW_DESC  desc = {};
			desc.BufferLocation = resource->get_gpu_address();
			desc.SizeInBytes = view_desc.Buffer.Size;
			assert(desc.SizeInBytes < 65536);

			Device::get().create_cbv(h, resource.get(), desc);
		}
		void write(UINT64 offset, T* data, UINT64 count)
		{
			memcpy(resource->buffer_data + offset, data, sizeof(T) * count);
		}
	};


}