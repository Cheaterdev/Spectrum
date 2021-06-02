#pragma once
namespace DX12
{

	class ResourceView
	{
	protected:
		RTVHandle rtv;
		DSVHandle dsv;
	
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
		template<class F>
		void init_views(F& frame)
		{

			auto& desc = resource->get_desc();

			if (desc.Flags & D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET)
			{
				rtv = frame.get_cpu_heap(DescriptorHeapType::RTV).place();
				place_rtv(rtv);
			}

			if (desc.Flags & D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL) {
				dsv = frame.get_cpu_heap(DescriptorHeapType::DSV).place();
				place_dsv(dsv);
			}
		}

		virtual void place_rtv(Handle h) { assert(false); }
		virtual void place_dsv(Handle h) { assert(false); }
	
		operator bool() const
		{
			return !!resource;
		}

		Handle get_rtv() { return rtv; }
		Handle get_dsv() { return dsv; }
		

	private:


	};

	class RTXSceneView :public ResourceView
	{

	public:
		HLSL::RaytracingAccelerationStructure srv_handle;

		RTXSceneView() = default;
		RTXSceneView(Resource::ptr resource, FrameResources& frame);

		virtual void place_srv(Handle h) ;
		virtual void place_uav(Handle h) { }
		virtual void place_rtv(Handle h) { assert(false); }
		virtual void place_dsv(Handle h) { assert(false); }
		virtual void place_cb(Handle h) { assert(false); }

	};
	class TextureView :public ResourceView
	{
		HandleTableLight hlsl;
		HandleTableLight rtv;
	public:


		HLSL::Texture2D<> texture2D;
		HLSL::RWTexture2D<> rwTexture2D;

		HLSL::Texture3D<> texture3D;
		HLSL::RWTexture3D<> rwTexture3D;

		HLSL::TextureCube<> texture—ube;
		HLSL::Texture2DArray<> texture2DArray;

		Handle renderTarget;
	public:
		template<class F>
		void init(F& frame)
		{
			hlsl = frame.get_cpu_heap(DescriptorHeapType::CBV_SRV_UAV).place(2);

			auto& desc = resource->get_desc();
			if (view_desc.type == ResourceType::TEXTURE2D) {
				texture2D = HLSL::Texture2D<>(hlsl[0]);
				rwTexture2D = HLSL::RWTexture2D<>(hlsl[1]);

			}
			if (view_desc.type == ResourceType::TEXTURE3D)
			{
				texture3D = HLSL::Texture3D<>(hlsl[0]);
				rwTexture3D = HLSL::RWTexture3D<>(hlsl[1]);
			}
			if (view_desc.type == ResourceType::CUBE)
				texture—ube = HLSL::TextureCube<>(hlsl[0]);
			if (view_desc.type == ResourceType::TEXTURE2D)
				texture2DArray = HLSL::Texture2DArray<>(hlsl[0]);
			if (!(desc.Flags & D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE)) {

				place_srv(hlsl[0]);

			}

			if (desc.Flags & D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS) {

				place_uav(hlsl[1]);
			}
			
			if (desc.Flags & D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET) {

				rtv = frame.get_cpu_heap(DescriptorHeapType::RTV).place(1);

				renderTarget = rtv[0];
				place_rtv(renderTarget);
			}
			
		}
		TextureView() = default;

		template<class F>
		TextureView(Resource::ptr resource, F& frame, bool cube = false) :ResourceView(resource)
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
			init(frame);

		}
		template<class F>
		TextureView(Resource::ptr resource, F& frame, ResourceViewDesc vdesc) :ResourceView(resource)
		{

			view_desc = vdesc;
			init_views(frame);
			init(frame);


		}

		virtual void place_srv(Handle h);
		virtual void place_uav(Handle h);
		virtual void place_rtv(Handle h);
		virtual void place_dsv(Handle h);
		virtual void place_cb(Handle h) { assert(false); }



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

		HLSL::RWByteAddressBuffer uav_handle;

		BufferView() = default;

		BufferView(Resource::ptr resource, FrameResources& frame);

		~BufferView()
		{

		}
		virtual void place_srv(Handle h) ;
		virtual void place_uav(Handle h) ;
		virtual void place_rtv(Handle h) { assert(false); }
		virtual void place_dsv(Handle h) { assert(false); }
		virtual void place_cb(Handle h);
		template<class T>
		void write(UINT64 offset, T* data, UINT64 count)
		{
			memcpy(resource->buffer_data + offset, data, sizeof(Underlying<T>) * count);
		}


	};



	template<class T>
	class StructuredBufferView :public ResourceView
	{
		HandleTableLight hlsl;
		HLSL::AppendStructuredBuffer<T> appendStructuredBuffer;

	public:
		UAVHandle uav_clear;
		HLSL::StructuredBuffer<T> structuredBuffer;
		HLSL::RWStructuredBuffer<T> rwStructuredBuffer;
	
	public:
		StructuredBufferView() = default;

		template<class F>
		StructuredBufferView(Resource::ptr resource, F& frame, UINT offset = 0, UINT64 size = 0) :ResourceView(resource)
		{
			init_desc();
			view_desc.Buffer.Offset = offset;
			if (size) view_desc.Buffer.Size = size;

			init_views(frame);

			hlsl = frame.get_cpu_heap(DescriptorHeapType::CBV_SRV_UAV).place(4);

			structuredBuffer = HLSL::StructuredBuffer<T>(hlsl[0]);
			rwStructuredBuffer = HLSL::RWStructuredBuffer<T>(hlsl[1]);
			appendStructuredBuffer = HLSL::AppendStructuredBuffer<T>(hlsl[2]);


			uav_clear = hlsl[3];

			auto& desc = resource->get_desc();

			if (!(desc.Flags & D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE)) {
				place_srv(structuredBuffer);
			}


			if (desc.Flags & D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS) {

				place_uav(rwStructuredBuffer);
				place_uav(appendStructuredBuffer);



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
		}
		Handle get_uav_clear() { return uav_clear; }
		~StructuredBufferView()
		{

		}
		virtual void place_srv(Handle h)  {
			if (!resource) return;

			D3D12_SHADER_RESOURCE_VIEW_DESC  desc = {};
			desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
			desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			desc.Format = DXGI_FORMAT_UNKNOWN;

			desc.Buffer.StructureByteStride = sizeof(Underlying<T>);
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

		virtual void place_uav(Handle h) {

			if (!resource) return;
			{
				D3D12_UNORDERED_ACCESS_VIEW_DESC  desc = {};
				desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;

				desc.Format = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;

				desc.Buffer.StructureByteStride = sizeof(Underlying<T>);
				desc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;


				desc.Buffer.NumElements = static_cast<UINT>(view_desc.Buffer.Size / sizeof(Underlying<T>));
				desc.Buffer.FirstElement = view_desc.Buffer.Offset / sizeof(Underlying<T>);
				desc.Buffer.CounterOffsetInBytes = 0;

				Device::get().create_uav(h, resource.get(), desc);
			}




		}
		virtual void place_rtv(Handle h) { assert(false); }
		virtual void place_dsv(Handle h) { assert(false); }
		virtual void place_cb(Handle h) {
			if (!resource) return;

			D3D12_CONSTANT_BUFFER_VIEW_DESC  desc = {};
			desc.BufferLocation = resource->get_gpu_address();
			desc.SizeInBytes = (UINT)view_desc.Buffer.Size;
			assert(desc.SizeInBytes < 65536);

			Device::get().create_cbv(h, resource.get(), desc);
		}
		void write(UINT64 offset, T* data, UINT64 count)
		{
			memcpy(resource->buffer_data + offset, data, sizeof(Underlying<T>) * count);
		}

	};


	template<class T, DXGI_FORMAT format>
	class FormattedBufferView :public ResourceView
	{

	public:
		FormattedBufferView() = default;
		HLSL::Buffer<T> srv_handle;
		HLSL::RWBuffer<T> uav_handle;

		template<class F>
		FormattedBufferView(Resource::ptr resource, F& frame, UINT offset = 0, UINT64 size = 0) :ResourceView(resource)
		{
			init_desc();


			view_desc.Buffer.Offset = offset;
			if (size) view_desc.Buffer.Size = size;

			init_views(frame);

			srv_handle = HLSL::Buffer<T>(frame.get_cpu_heap(DescriptorHeapType::CBV_SRV_UAV).place());
			place_srv(srv_handle);


			auto& desc = resource->get_desc();
			if (desc.Flags & D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS) {
				uav_handle = HLSL::RWBuffer<T>(frame.get_cpu_heap(DescriptorHeapType::CBV_SRV_UAV).place());
				place_uav(uav_handle);
			}
		}

		~FormattedBufferView()
		{

		}
		virtual void place_srv(Handle h){
			if (!resource) return;

			D3D12_SHADER_RESOURCE_VIEW_DESC  desc = {};
			desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
			desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
			desc.Format = format;

			desc.Buffer.StructureByteStride = 0;
			desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;


			desc.Buffer.NumElements = static_cast<UINT>(view_desc.Buffer.Size / sizeof(Underlying<T>));
			desc.Buffer.FirstElement = view_desc.Buffer.Offset / sizeof(Underlying<T>);

			Device::get().create_srv(h, resource.get(), desc);


		}

		virtual void place_uav(Handle h) {

			if (!resource) return;

			D3D12_UNORDERED_ACCESS_VIEW_DESC  desc = {};
			desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;

			desc.Format = format;

			desc.Buffer.StructureByteStride = 0;
			desc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;


			desc.Buffer.NumElements = static_cast<UINT>(view_desc.Buffer.Size / sizeof(Underlying<T>));
			desc.Buffer.FirstElement = view_desc.Buffer.Offset / sizeof(Underlying<T>);
			desc.Buffer.CounterOffsetInBytes = 0;

			Device::get().create_uav(h, resource.get(), desc);
		}

		virtual void place_rtv(Handle h) { assert(false); }
		virtual void place_dsv(Handle h) { assert(false); }
		virtual void place_cb(Handle h) {
			if (!resource) return;

			D3D12_CONSTANT_BUFFER_VIEW_DESC  desc = {};
			desc.BufferLocation = resource->get_gpu_address();
			desc.SizeInBytes = (UINT)view_desc.Buffer.Size;
			assert(desc.SizeInBytes < 65536);

			Device::get().create_cbv(h, resource.get(), desc);
		}
		void write(UINT64 offset, T* data, UINT64 count)
		{
			memcpy(resource->buffer_data + offset, data, sizeof(Underlying<T>) * count);
		}
	};

	struct IndexBufferView
	{
		D3D12_INDEX_BUFFER_VIEW view;
		Resource* resource = nullptr;
	};
}