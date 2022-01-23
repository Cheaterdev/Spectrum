module;
#include "Math/Math.h"
#include "dx12_types.h"
export module ResourceViews;

import Resource;

import Descriptors;
import Graphics.Types;

import HLSLDescriptors;

export
{

	namespace Graphics
	{
		//class CommandList;

		class ResourceView
		{

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

			operator bool() const
			{
				return !!resource;
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
				srv_handle = HLSL::Buffer<T>(frame.get_gpu_heap(HAL::DescriptorHeapType::CBV_SRV_UAV).place());

				srv_handle.create(resource.get(), format, static_cast<UINT>(view_desc.Buffer.Offset / sizeof(Underlying<T>)), static_cast<UINT>(view_desc.Buffer.Size / sizeof(Underlying<T>)));

				auto& desc = resource->get_desc();
				if (desc.Flags & D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS) {
					uav_handle = HLSL::RWBuffer<T>(frame.get_gpu_heap(HAL::DescriptorHeapType::CBV_SRV_UAV).place());
					uav_handle.create(resource.get(), format, static_cast<UINT>(view_desc.Buffer.Offset / sizeof(Underlying<T>)), static_cast<UINT>(view_desc.Buffer.Size / sizeof(Underlying<T>)));
				}
			}

			~FormattedBufferView()
			{

			}

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


		class RTXSceneView :public ResourceView
		{

		public:
			HLSL::RaytracingAccelerationStructure scene;

			RTXSceneView() = default;
			RTXSceneView(Resource::ptr resource, FrameResources& frame);

		};
		class TextureView :public ResourceView
		{


		public:


			HLSL::Texture2D<> texture2D;
			HLSL::RWTexture2D<> rwTexture2D;

			HLSL::Texture3D<> texture3D;
			HLSL::RWTexture3D<> rwTexture3D;

			HLSL::TextureCube<> texture—ube;
			HLSL::Texture2DArray<> texture2DArray;

			HLSL::RenderTarget<> renderTarget;
			HLSL::DepthStencil<> depthStencil;
		public:
			template<class F>
			void init(F& frame)
			{
				HandleTableLight hlsl = frame.get_gpu_heap(HAL::DescriptorHeapType::CBV_SRV_UAV).place(2);

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
				{
					texture—ube = HLSL::TextureCube<>(hlsl[0]);
					rwTexture2D = HLSL::RWTexture2D<>(hlsl[1]);
				}

				if (view_desc.type == ResourceType::TEXTURE2D)
					texture2DArray = HLSL::Texture2DArray<>(hlsl[0]);


				if (!(desc.Flags & D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE)) {

					if (view_desc.type == ResourceType::TEXTURE2D && view_desc.Texture2D.ArraySize == 1)
						texture2D.create(resource.get(), view_desc.Texture2D.MipSlice, view_desc.Texture2D.MipLevels, view_desc.Texture2D.FirstArraySlice);
					else if (view_desc.type == ResourceType::TEXTURE2D)
						texture2DArray.create(resource.get(), view_desc.Texture2D.MipSlice, view_desc.Texture2D.MipLevels, view_desc.Texture2D.FirstArraySlice, view_desc.Texture2D.ArraySize);
					else if (view_desc.type == ResourceType::CUBE)
						texture—ube.create(resource.get(), view_desc.Texture2D.MipSlice, view_desc.Texture2D.MipLevels, view_desc.Texture2D.FirstArraySlice / 6);
					else if (view_desc.type == ResourceType::TEXTURE3D)
						texture3D.create(resource.get(), view_desc.Texture2D.MipSlice, view_desc.Texture2D.MipLevels);
					else
						assert(false);
				}

				if (desc.Flags & D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS) {
					if ((view_desc.type == ResourceType::TEXTURE2D || view_desc.type == ResourceType::CUBE) && view_desc.Texture2D.ArraySize == 1)
						rwTexture2D.create(resource.get(), view_desc.Texture2D.MipSlice, view_desc.Texture2D.FirstArraySlice);
					else if (view_desc.type == ResourceType::TEXTURE3D)
						rwTexture3D.create(resource.get(), view_desc.Texture2D.MipSlice, 0);
					else
						assert(false);
				}

				if (desc.Flags & D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET) {

					HandleTableLight rtv = frame.get_cpu_heap(HAL::DescriptorHeapType::RTV).place(1);

					renderTarget = HLSL::RenderTarget<>(rtv[0]);
					//	place_rtv(renderTarget);
					if (view_desc.type == ResourceType::TEXTURE2D && view_desc.Texture2D.ArraySize == 1 && view_desc.Texture2D.FirstArraySlice == 0)
					{
						renderTarget.createFrom2D(resource.get(), view_desc.Texture2D.MipSlice);
					}
					else
					{
						renderTarget.createFrom2DArray(resource.get(), view_desc.Texture2D.MipSlice, view_desc.Texture2D.FirstArraySlice);
					}
				}

				if (desc.Flags & D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL) {
					HandleTableLight dsv = frame.get_cpu_heap(HAL::DescriptorHeapType::DSV).place(1);
					depthStencil = HLSL::DepthStencil<>(dsv[0]);

					if (view_desc.type == ResourceType::TEXTURE2D && view_desc.Texture2D.ArraySize == 1 && view_desc.Texture2D.FirstArraySlice == 0)
					{
						depthStencil.createFrom2D(resource.get(), view_desc.Texture2D.MipSlice);
					}
					else
					{
						depthStencil.createFrom2DArray(resource.get(), view_desc.Texture2D.MipSlice, view_desc.Texture2D.FirstArraySlice);
					}
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

				init(frame);

			}
			template<class F>
			TextureView(Resource::ptr resource, F& frame, ResourceViewDesc vdesc) :ResourceView(resource)
			{

				view_desc = vdesc;
				init(frame);


			}


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

			TextureView create_2d_slice(UINT slice, FrameResources& frame);

			TextureView create_mip(UINT mip, FrameResources& frame);


		};
		/*

		class TextureCube : public ResourceView
		{
		public:
			HLSL::TextureCube<> texture—ube;

			std::array<TextureView, 6> faces;
		public:
			template<class F>
			void init(F& frame)
			{
				HandleTableLight hlsl = frame.get_cpu_heap(DescriptorHeapType::CBV_SRV_UAV).place(2);

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
				{
					texture—ube = HLSL::TextureCube<>(hlsl[0]);
					rwTexture2D = HLSL::RWTexture2D<>(hlsl[1]);
				}

				if (view_desc.type == ResourceType::TEXTURE2D)
					texture2DArray = HLSL::Texture2DArray<>(hlsl[0]);


				if (!(desc.Flags & D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE)) {

					if (view_desc.type == ResourceType::TEXTURE2D && view_desc.Texture2D.ArraySize == 1)
						texture2D.create(resource.get(), view_desc.Texture2D.MipSlice, view_desc.Texture2D.MipLevels, view_desc.Texture2D.FirstArraySlice);
					else if (view_desc.type == ResourceType::TEXTURE2D)
						texture2DArray.create(resource.get(), view_desc.Texture2D.MipSlice, view_desc.Texture2D.MipLevels, view_desc.Texture2D.FirstArraySlice, view_desc.Texture2D.ArraySize);
					else if (view_desc.type == ResourceType::CUBE)
						texture—ube.create(resource.get(), view_desc.Texture2D.MipSlice, view_desc.Texture2D.MipLevels, view_desc.Texture2D.FirstArraySlice / 6);
					else if (view_desc.type == ResourceType::TEXTURE3D)
						texture3D.create(resource.get(), view_desc.Texture2D.MipSlice, view_desc.Texture2D.MipLevels);
					else
						assert(false);
				}

				if (desc.Flags & D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS) {
					if ((view_desc.type == ResourceType::TEXTURE2D || view_desc.type == ResourceType::CUBE) && view_desc.Texture2D.ArraySize == 1)
						rwTexture2D.create(resource.get(), view_desc.Texture2D.MipSlice, view_desc.Texture2D.FirstArraySlice);
					else if (view_desc.type == ResourceType::TEXTURE3D)
						rwTexture3D.create(resource.get(), view_desc.Texture2D.MipSlice, 0);
					else
						assert(false);
				}

				if (desc.Flags & D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET) {

					HandleTableLight rtv = frame.get_cpu_heap(DescriptorHeapType::RTV).place(1);

					renderTarget = rtv[0];
					place_rtv(renderTarget);
				}

			}
			TextureCube() = default;

			template<class F>
			TextureCube(Resource::ptr resource, F & frame) :ResourceView(resource)
			{
				init_desc();

				auto& desc = resource->get_desc();

				view_desc.type = ResourceType::CUBE;
				view_desc.Texture2D.ArraySize = desc.ArraySize() / 6;
				view_desc.Texture2D.MipSlice = 0;
				view_desc.Texture2D.FirstArraySlice = 0;
				view_desc.Texture2D.PlaneSlice = 0;
				view_desc.Texture2D.MipLevels = desc.MipLevels;

				init_views(frame);
				init(frame);
			}

			template<class F>
			TextureCube(Resource::ptr resource, F & frame, ResourceViewDesc vdesc) :ResourceView(resource)
			{

				view_desc = vdesc;
				init_views(frame);
				init(frame);


			}

			virtual void place_rtv(Handle h);
			virtual void place_dsv(Handle h);

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

			TextureView create_2d_slice(UINT slice, FrameResources & frame)
			{
				ResourceViewDesc desc = view_desc;
				desc.type = ResourceType::TEXTURE2D;
				desc.Texture2D.ArraySize = 1;
				desc.Texture2D.FirstArraySlice = slice;

				return TextureView(resource, frame, desc);
			}

			TextureCube create_mip(UINT mip, FrameResources & frame)
			{
				ResourceViewDesc desc = view_desc;

				desc.Texture2D.MipSlice += mip;
				desc.Texture2D.MipLevels = 1;

				return TextureCube(resource, frame, desc);
			}


		};
		*/

		class BufferView : public ResourceView
		{

		public:
			HLSL::ByteAddressBuffer byteBuffer;
			HLSL::RWByteAddressBuffer rwbyteBuffer;

			BufferView() = default;
			template<class F>
			BufferView(Resource::ptr resource, F& frame, UINT offset = 0, UINT64 size = 0) :ResourceView(resource)
			{
				init_desc();

				HandleTableLight hlsl = frame.get_gpu_heap(HAL::DescriptorHeapType::CBV_SRV_UAV).place(2);

				byteBuffer = HLSL::ByteAddressBuffer(hlsl[0]);
				rwbyteBuffer = HLSL::RWByteAddressBuffer(hlsl[1]);

				auto& desc = resource->get_desc();

				if (!(desc.Flags & D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE)) {
					byteBuffer.create(resource.get(), offset, size);
				}

				if (desc.Flags & D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS) {
					rwbyteBuffer = HLSL::RWByteAddressBuffer(frame.get_cpu_heap(HAL::DescriptorHeapType::CBV_SRV_UAV).place());
					rwbyteBuffer.create(resource.get(), offset, size);
				}
			}

			~BufferView()
			{

			}

			virtual void place_cb(Handle h);
			template<class T>
			void write(UINT64 offset, T* data, UINT64 count)
			{
				memcpy(resource->buffer_data + offset, data, sizeof(Underlying<T>) * count);
			}

		};


		class CounterView :public ResourceView
		{
		public:
			HLSL::StructuredBuffer<UINT> structuredBuffer;
			HLSL::RWStructuredBuffer<UINT> rwStructuredBuffer;
			HLSL::RWBuffer<std::byte> rwRAW;
			CounterView() = default;

			template<class F>
			CounterView(Resource::ptr resource, F& frame, UINT offset = 0) :ResourceView(resource)
			{
				init_desc();

				HandleTableLight hlsl = frame.get_gpu_heap(HAL::DescriptorHeapType::CBV_SRV_UAV).place(3);

				structuredBuffer = HLSL::StructuredBuffer<UINT>(hlsl[0]);
				rwStructuredBuffer = HLSL::RWStructuredBuffer<UINT>(hlsl[1]);
				rwRAW = HLSL::RWBuffer<std::byte>(hlsl[2]);
				auto& desc = resource->get_desc();

				if (!(desc.Flags & D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE)) {
					structuredBuffer.create(resource.get(), offset / sizeof(UINT), 1);
				}

				if (desc.Flags & D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS) {
					rwStructuredBuffer.create(resource.get(), offset / sizeof(UINT), 1);
					rwRAW.create(resource.get(), DXGI_FORMAT::DXGI_FORMAT_R8_UINT, offset, sizeof(UINT));
				}
			}
		};

		enum class BufferType
		{
			NONE,
			COUNTED
		};
		template<class T>
		class StructuredBufferView :public ResourceView
		{

			int local_offset = 0;
		public:
			HLSL::RWBuffer<std::byte> rwRAW;
			HLSL::StructuredBuffer<T> structuredBuffer;
			HLSL::RWStructuredBuffer<T> rwStructuredBuffer;
			HLSL::AppendStructuredBuffer<T> appendStructuredBuffer;

			StructuredBufferView() = default;
			CounterView counter_view;
		public:


			template<class F>
			StructuredBufferView(Resource::ptr resource, F& frame, BufferType counted = BufferType::NONE, UINT offset = 0, UINT64 size = 0) :ResourceView(resource)
			{
				init_desc();

				local_offset = 0;
				if (counted == BufferType::COUNTED)
				{
					local_offset = Math::AlignUp(4, sizeof(Underlying<T>));
					if (size == 0) size = view_desc.Buffer.Size;
					size -= local_offset;
					offset += local_offset;


					counter_view = resource->create_view<CounterView>(frame, 0);
				}

				view_desc.Buffer.Offset = offset;
				if (size) view_desc.Buffer.Size = size;

				view_desc.Buffer.counted = counted == BufferType::COUNTED;
				HandleTableLight hlsl = frame.get_gpu_heap(HAL::DescriptorHeapType::CBV_SRV_UAV).place(4);

				structuredBuffer = HLSL::StructuredBuffer<T>(hlsl[0]);
				rwStructuredBuffer = HLSL::RWStructuredBuffer<T>(hlsl[1]);
				appendStructuredBuffer = HLSL::AppendStructuredBuffer<T>(hlsl[2]);


				rwRAW = HLSL::RWBuffer<std::byte>(hlsl[3]);

				auto& desc = resource->get_desc();

				if (!(desc.Flags & D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE)) {
					structuredBuffer.create(resource.get(), static_cast<UINT>(view_desc.Buffer.Offset / sizeof(Underlying<T>)), static_cast<UINT>(view_desc.Buffer.Size / sizeof(Underlying<T>)));
				}


				if (desc.Flags & D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS) {

					rwStructuredBuffer.create(resource.get(), static_cast<UINT>(view_desc.Buffer.Offset / sizeof(Underlying<T>)), static_cast<UINT>(view_desc.Buffer.Size / sizeof(Underlying<T>)));
					rwRAW.create(resource.get(), DXGI_FORMAT::DXGI_FORMAT_R8_UINT, static_cast<UINT>(view_desc.Buffer.Offset), static_cast<UINT>(view_desc.Buffer.Size));

					if (counted == BufferType::COUNTED)
						appendStructuredBuffer.create(resource.get(), 0, resource.get(), static_cast<UINT>(view_desc.Buffer.Offset / sizeof(Underlying<T>)), static_cast<UINT>(view_desc.Buffer.Size / sizeof(Underlying<T>)));
				}
			}
			Handle get_uav_clear() { return rwRAW; }
			~StructuredBufferView()
			{

			}

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

	
	}
}