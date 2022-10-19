export module Graphics:ResourceViews;
import :Descriptors;
import :Types;

import HAL;
import :HLSLDescriptors;
import :Device;

export
{

	namespace Graphics
	{
		struct TextureViewDesc
		{

			//Format format;

		//	uint PlaneSlice;
			uint MipSlice = 0;
			uint MipLevels = 0;
			uint FirstArraySlice = 0;
			uint ArraySize = 0;

		};

		struct CubeViewDesc
		{
			uint MipSlice = 0;
			uint MipLevels = 0;
			uint FirstArraySlice = 0;
			uint ArraySize = 0;
		};

		struct Texture3DViewDesc
		{
			uint MipSlice = 0;
			uint MipLevels = 0;
		};

		class ResourceView
		{

		public:
			HAL::Resource* resource; //////////////
			ResourceView()
			{

			}

			auto get_desc() const
			{
				return resource->get_desc();
			}

			ResourceView(HAL::Resource* _resource) :resource(_resource)
			{

			}

			operator bool() const
			{
				return !!resource;
			}


		};
		struct FormattedBufferViewDesc
		{
			uint offset;
			uint size;
		};

		template<class T, Format::Formats _format>
		class FormattedBufferView :public ResourceView
		{

		public:

			using Desc = FormattedBufferViewDesc;

			Desc desc;
			FormattedBufferView() = default;
			HLSL::Buffer<T> buffer;
			HLSL::RWBuffer<T> rwBuffer;

			template<class F>
			void init(F& frame)
			{
				Format format = _format;
				buffer = HLSL::Buffer<T>(frame.get_gpu_heap(HAL::DescriptorHeapType::CBV_SRV_UAV).place());
				buffer.create(resource, format, static_cast<UINT>(desc.offset / sizeof(Underlying<T>)), static_cast<UINT>(desc.size / sizeof(Underlying<T>)));

				//
				if (check(resource->get_desc().Flags & HAL::ResFlags::UnorderedAccess)) {
					rwBuffer = HLSL::RWBuffer<T>(frame.get_gpu_heap(HAL::DescriptorHeapType::CBV_SRV_UAV).place());
					rwBuffer.create(resource, format, static_cast<UINT>(desc.offset / sizeof(Underlying<T>)), static_cast<UINT>(desc.size / sizeof(Underlying<T>)));
				}
			}


			template<class F>
			FormattedBufferView(HAL::Resource* resource, F& frame, FormattedBufferViewDesc _desc) :ResourceView(resource), desc(_desc)
			{
				init(frame);
			}

			template<class F>
			FormattedBufferView(HAL::Resource* resource, F& frame) : ResourceView(resource)
			{
				auto& res_desc = resource->get_desc().as_buffer();

				desc.offset = 0;
				desc.size = res_desc.SizeInBytes;

				init(frame);
			}

			~FormattedBufferView()
			{

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
			RTXSceneView(HAL::Resource* resource, FrameResources& frame);

		};
		class TextureView :public ResourceView
		{
			TextureViewDesc view_desc;
		public:
			HLSL::Texture2D<> texture2D;
			HLSL::RWTexture2D<> rwTexture2D;

			//	HLSL::Texture3D<> texture3D;
			//	HLSL::RWTexture3D<> rwTexture3D;


			HLSL::Texture2DArray<> texture2DArray;

			HLSL::RenderTarget<> renderTarget;
			HLSL::DepthStencil<> depthStencil;
		public:
			template<class F>
			void init(F& frame, TextureViewDesc _view_desc)
			{
				view_desc = _view_desc;

				auto& desc = resource->get_desc().as_texture();

				if (view_desc.MipLevels == 0)
				{
					view_desc.MipLevels = desc.MipLevels - view_desc.MipSlice;
				}
				HandleTableLight hlsl = frame.get_gpu_heap(HAL::DescriptorHeapType::CBV_SRV_UAV).place(3);
				if (desc.is2D()) {
					texture2D = HLSL::Texture2D<>(hlsl[0]);
					rwTexture2D = HLSL::RWTexture2D<>(hlsl[1]);

					texture2DArray = HLSL::Texture2DArray<>(hlsl[2]);


				}

				/*	if (desc.is3D()) {
						texture3D = HLSL::Texture3D<>(hlsl[0]);
						rwTexture3D = HLSL::RWTexture3D<>(hlsl[1]);
					}*/


				if (check(resource->get_desc().Flags & HAL::ResFlags::ShaderResource)) {

					if (desc.is2D() && view_desc.ArraySize == 1)
						texture2D.create(resource, view_desc.MipSlice, view_desc.MipLevels, view_desc.FirstArraySlice);
					else if (desc.is2D())
						texture2DArray.create(resource, view_desc.MipSlice, view_desc.MipLevels, view_desc.FirstArraySlice, view_desc.ArraySize);
					//	else if (desc.is3D())
					//		texture3D.create(resource, view_desc.MipSlice, view_desc.MipLevels);
					else
						assert(false);
				}

				if (check(resource->get_desc().Flags & HAL::ResFlags::UnorderedAccess)) {
					if (desc.is2D() && view_desc.ArraySize == 1)
						rwTexture2D.create(resource, view_desc.MipSlice, view_desc.FirstArraySlice);
					//	else if (desc.is3D())
					//		rwTexture3D.create(resource, view_desc.MipSlice);
					else
						assert(false);
				}

				if (check(resource->get_desc().Flags & HAL::ResFlags::RenderTarget)) {

					HandleTableLight rtv = frame.get_cpu_heap(HAL::DescriptorHeapType::RTV).place(1);

					renderTarget = HLSL::RenderTarget<>(rtv[0]);
					//	place_rtv(renderTarget);
					if (desc.is2D() && view_desc.ArraySize == 1 && view_desc.FirstArraySlice == 0)
					{
						renderTarget.createFrom2D(resource, view_desc.MipSlice);
					}
					else
					{
						renderTarget.createFrom2DArray(resource, view_desc.MipSlice, view_desc.FirstArraySlice);
					}
				}

				if (check(resource->get_desc().Flags & HAL::ResFlags::DepthStencil)) {
					HandleTableLight dsv = frame.get_cpu_heap(HAL::DescriptorHeapType::DSV).place(1);
					depthStencil = HLSL::DepthStencil<>(dsv[0]);

					if (desc.is2D() && view_desc.ArraySize == 1 && view_desc.FirstArraySlice == 0)
					{
						depthStencil.createFrom2D(resource, view_desc.MipSlice);
					}
					else
					{
						depthStencil.createFrom2DArray(resource, view_desc.MipSlice, view_desc.FirstArraySlice);
					}
				}


			}
			TextureView() = default;

			template<class F>
			TextureView(HAL::Resource* resource, F& frame) :ResourceView(resource)
			{
				auto& texture_desc = resource->get_desc().as_texture();
				uint array_size = texture_desc.ArraySize;

				init(frame, { 0, texture_desc.MipLevels, 0,array_size });
			}
			template<class F>
			TextureView(HAL::Resource* resource, F& frame, TextureViewDesc vdesc) :ResourceView(resource)
			{

				init(frame, vdesc);

			}


			Viewport get_viewport()
			{

				auto& texture_desc = resource->get_desc().as_texture();

				UINT scaler = 1 << view_desc.MipSlice;


				Viewport p;
				p.Width = std::max(1.0f, static_cast<float>(texture_desc.Dimensions.x / scaler));
				p.Height = std::max(1.0f, static_cast<float>(texture_desc.Dimensions.y / scaler));
				p.TopLeftX = 0;
				p.TopLeftY = 0;
				p.MinDepth = 0;
				p.MaxDepth = 1;

				return p;
			}


			sizer_long get_scissor()
			{
				UINT scaler = 1 << view_desc.MipSlice;
				auto& texture_desc = resource->get_desc().as_texture();

				return { 0,0, std::max(1u,texture_desc.Dimensions.x / scaler),std::max(1u,texture_desc.Dimensions.y / scaler) };
			}
			UINT get_mip_count()
			{
				return view_desc.MipLevels;
			}

			ivec2 get_size()
			{
				auto& texture_desc = resource->get_desc().as_texture();

				UINT scaler = 1 << view_desc.MipSlice;

				return uint2::max({ 1,1 }, uint2(texture_desc.Dimensions.xy) / scaler);
			}

			TextureView create_2d_slice(UINT slice, FrameResources& frame);

			TextureView create_mip(UINT mip, FrameResources& frame);


		};


		class Texture3DView :public ResourceView
		{
			Texture3DViewDesc view_desc;
		public:
			HLSL::Texture3D<> texture3D;

			struct Mip
			{
				HLSL::Texture3D<> texture3D;
				HLSL::RWTexture3D<> rwTexture3D;
			};

			std::vector<Mip> mips;

		public:
			template<class F>
			void init(F& frame, Texture3DViewDesc _view_desc)
			{
				view_desc = _view_desc;

				auto& desc = resource->get_desc().as_texture();

				if (view_desc.MipLevels == 0)
				{
					view_desc.MipLevels = desc.MipLevels - view_desc.MipSlice;
				}
				HandleTableLight hlsl = frame.get_gpu_heap(HAL::DescriptorHeapType::CBV_SRV_UAV).place(1 + view_desc.MipLevels * 2);
				assert(desc.is3D());


				uint offset = 0;
				mips.resize(view_desc.MipLevels);
				if (check(resource->get_desc().Flags & HAL::ResFlags::ShaderResource)) {
					texture3D = HLSL::Texture3D<>(hlsl[offset++]);
					texture3D.create(resource, view_desc.MipSlice, view_desc.MipLevels);

					for (uint i = 0; i < view_desc.MipLevels; i++)
					{
						mips[i].texture3D = HLSL::Texture3D<>(hlsl[offset++]);
						mips[i].texture3D.create(resource, view_desc.MipSlice + i, 1);
					}
				}

				if (check(resource->get_desc().Flags & HAL::ResFlags::UnorderedAccess)) {
					for (uint i = 0; i < view_desc.MipLevels; i++)
					{
						mips[i].rwTexture3D = HLSL::RWTexture3D<>(hlsl[offset++]);
						mips[i].rwTexture3D.create(resource, view_desc.MipSlice + i);
					}
				}
			}

			Texture3DView() = default;

			template<class F>
			Texture3DView(HAL::Resource* resource, F& frame) :ResourceView(resource)
			{
				auto& texture_desc = resource->get_desc().as_texture();
				uint array_size = texture_desc.ArraySize;

				init(frame, { 0, texture_desc.MipLevels });
			}
			template<class F>
			Texture3DView(HAL::Resource* resource, F& frame, Texture3DViewDesc vdesc) :ResourceView(resource)
			{

				init(frame, vdesc);

			}


			Viewport get_viewport()
			{

				auto& texture_desc = resource->get_desc().as_texture();

				UINT scaler = 1 << view_desc.MipSlice;


				Viewport p;
				p.Width = std::max(1.0f, static_cast<float>(texture_desc.Dimensions.x / scaler));
				p.Height = std::max(1.0f, static_cast<float>(texture_desc.Dimensions.y / scaler));
				p.TopLeftX = 0;
				p.TopLeftY = 0;
				p.MinDepth = 0;
				p.MaxDepth = 1;

				return p;
			}


			sizer_long get_scissor()
			{
				UINT scaler = 1 << view_desc.MipSlice;
				auto& texture_desc = resource->get_desc().as_texture();

				return { 0,0, std::max(1u,texture_desc.Dimensions.x / scaler),std::max(1u,texture_desc.Dimensions.y / scaler) };
			}
			UINT get_mip_count()
			{
				return view_desc.MipLevels;
			}

			ivec2 get_size()
			{
				auto& texture_desc = resource->get_desc().as_texture();

				UINT scaler = 1 << view_desc.MipSlice;

				return uint2::max({ 1,1 }, uint2(texture_desc.Dimensions.xy) / scaler);
			}
		};


		class CubeView :public ResourceView
		{
			CubeViewDesc view_desc;
		public:

			HLSL::TextureCube<> textureCube;


			std::array<TextureView, 6> faces;

		public:
			template<class F>
			void init(F& frame, CubeViewDesc _view_desc)
			{
				view_desc = _view_desc;

				auto& desc = resource->get_desc().as_texture();

				if (view_desc.MipLevels == 0)
				{
					view_desc.MipLevels = desc.MipLevels - view_desc.MipSlice;
				}
				HandleTableLight hlsl = frame.get_gpu_heap(HAL::DescriptorHeapType::CBV_SRV_UAV).place(1);
				assert(desc.is2D());
				textureCube = HLSL::TextureCube<>(hlsl[0]);


				if (check(resource->get_desc().Flags & HAL::ResFlags::ShaderResource)) {
					assert(desc.is2D());
					textureCube.create(resource, view_desc.MipSlice, view_desc.MipLevels, view_desc.FirstArraySlice / 6);
				}

				if (check(resource->get_desc().Flags & HAL::ResFlags::UnorderedAccess)) {
					TextureViewDesc desc;

					desc.MipSlice = view_desc.MipSlice;
					desc.MipLevels = view_desc.MipLevels;
					desc.ArraySize = 1;


					for (uint i = 0; i < 6; i++)
					{
						desc.FirstArraySlice = view_desc.FirstArraySlice + i;
						faces[i] = TextureView(resource, frame, desc);
					}

				}

			}
			CubeView() = default;

			template<class F>
			CubeView(HAL::Resource* resource, F& frame) :ResourceView(resource)
			{
				auto& texture_desc = resource->get_desc().as_texture();
				uint array_size = texture_desc.ArraySize / 6;

				init(frame, { 0, texture_desc.MipLevels, 0,array_size });
			}
			template<class F>
			CubeView(HAL::Resource* resource, F& frame, CubeViewDesc vdesc) :ResourceView(resource)
			{

				init(frame, vdesc);

			}


			Viewport get_viewport()
			{

				auto& texture_desc = resource->get_desc().as_texture();

				UINT scaler = 1 << view_desc.MipSlice;


				Viewport p;
				p.Width = std::max(1.0f, static_cast<float>(texture_desc.Dimensions.x / scaler));
				p.Height = std::max(1.0f, static_cast<float>(texture_desc.Dimensions.y / scaler));
				p.TopLeftX = 0;
				p.TopLeftY = 0;
				p.MinDepth = 0;
				p.MaxDepth = 1;

				return p;
			}


			sizer_long get_scissor()
			{
				UINT scaler = 1 << view_desc.MipSlice;
				auto& texture_desc = resource->get_desc().as_texture();

				return { 0,0, std::max(1u,texture_desc.Dimensions.x / scaler),std::max(1u,texture_desc.Dimensions.y / scaler) };
			}
			ivec2 get_size()
			{
				auto& texture_desc = resource->get_desc().as_texture();

				UINT scaler = 1 << view_desc.MipSlice;

				return uint2::max({ 1,1 }, uint2(texture_desc.Dimensions.xy) / scaler);
			}

			TextureView get_face(UINT face);

			CubeView create_mip(UINT mip, FrameResources& frame);
		};

		class BufferView : public ResourceView
		{

		public:
			HLSL::ByteAddressBuffer byteBuffer;
			HLSL::RWByteAddressBuffer rwbyteBuffer;

			BufferView() = default;
			template<class F>
			BufferView(HAL::Resource* resource, F& frame, UINT offset = 0, UINT64 size = 0) :ResourceView(resource)
			{
				HandleTableLight hlsl = frame.get_gpu_heap(HAL::DescriptorHeapType::CBV_SRV_UAV).place(2);

				byteBuffer = HLSL::ByteAddressBuffer(hlsl[0]);
				rwbyteBuffer = HLSL::RWByteAddressBuffer(hlsl[1]);

				auto& desc = resource->get_desc().as_buffer();

				if (check(resource->get_desc().Flags & HAL::ResFlags::ShaderResource)) {
					byteBuffer.create(resource, offset, size);
				}

				if (check(resource->get_desc().Flags & HAL::ResFlags::UnorderedAccess)) {
					rwbyteBuffer = HLSL::RWByteAddressBuffer(frame.get_cpu_heap(HAL::DescriptorHeapType::CBV_SRV_UAV).place());
					rwbyteBuffer.create(resource, offset, size);
				}
			}

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
			CounterView(HAL::Resource* resource, F& frame, UINT offset = 0) :ResourceView(resource)
			{
				HandleTableLight hlsl = frame.get_gpu_heap(HAL::DescriptorHeapType::CBV_SRV_UAV).place(3);

				structuredBuffer = HLSL::StructuredBuffer<UINT>(hlsl[0]);
				rwStructuredBuffer = HLSL::RWStructuredBuffer<UINT>(hlsl[1]);
				rwRAW = HLSL::RWBuffer<std::byte>(hlsl[2]);
				auto& desc = resource->get_desc().as_buffer();

				if (check(resource->get_desc().Flags & HAL::ResFlags::ShaderResource)) {
					structuredBuffer.create(resource, offset / sizeof(UINT), 1);
				}

				if (check(resource->get_desc().Flags & HAL::ResFlags::UnorderedAccess)) {
					rwStructuredBuffer.create(resource, offset / sizeof(UINT), 1);
					rwRAW.create(resource, Format::R8_UINT, offset, sizeof(UINT));
				}
			}
		};



		struct StructuredBufferViewDesc
		{
			uint offset;
			uint size;
			bool counted;
		};
		template<class T>
		class StructuredBufferView :public ResourceView
		{

		public:
			using Desc = StructuredBufferViewDesc;

			Desc desc;
			HLSL::RWBuffer<std::byte> rwRAW;
			HLSL::StructuredBuffer<T> structuredBuffer;
			HLSL::RWStructuredBuffer<T> rwStructuredBuffer;
			HLSL::AppendStructuredBuffer<T> appendStructuredBuffer;

			StructuredBufferView() = default;
			CounterView counter_view;
		public:


			template<class F>
			void init(F& frame)
			{
				uint local_offset = 0;
				UINT offset = desc.offset;
				UINT64 size = desc.size;

				if (desc.counted)
				{
					local_offset = Math::AlignUp(4, sizeof(Underlying<T>));
					if (size == 0) size = resource->get_desc().as_buffer().SizeInBytes;
					size -= local_offset;
					offset += local_offset;

					counter_view = resource->create_view<CounterView>(frame, 0);
				}

				HandleTableLight hlsl = frame.get_gpu_heap(HAL::DescriptorHeapType::CBV_SRV_UAV).place(4);

				structuredBuffer = HLSL::StructuredBuffer<T>(hlsl[0]);
				rwStructuredBuffer = HLSL::RWStructuredBuffer<T>(hlsl[1]);
				appendStructuredBuffer = HLSL::AppendStructuredBuffer<T>(hlsl[2]);


				rwRAW = HLSL::RWBuffer<std::byte>(hlsl[3]);


				if (check(resource->get_desc().Flags & HAL::ResFlags::ShaderResource)) {
					structuredBuffer.create(resource, static_cast<UINT>(offset / sizeof(Underlying<T>)), static_cast<UINT>(size / sizeof(Underlying<T>)));
				}


				if (check(resource->get_desc().Flags & HAL::ResFlags::UnorderedAccess)) {
					rwStructuredBuffer.create(resource, static_cast<UINT>(offset / sizeof(Underlying<T>)), static_cast<UINT>(size / sizeof(Underlying<T>)));
					rwRAW.create(resource, Format::R8_UINT, static_cast<UINT>(offset), static_cast<UINT>(size));

					if (desc.counted)
						appendStructuredBuffer.create(resource, 0, resource, static_cast<UINT>(offset / sizeof(Underlying<T>)), static_cast<UINT>(size / sizeof(Underlying<T>)));
				}
			}

			template<class F>
			StructuredBufferView(HAL::Resource* resource, F& frame, StructuredBufferViewDesc _desc) :ResourceView(resource), desc(_desc)
			{
				init(frame);
			}

			template<class F>
			StructuredBufferView(HAL::Resource* resource, F& frame) : ResourceView(resource)
			{
				auto& res_desc = resource->get_desc().as_buffer();

				desc.offset = 0;
				desc.size = res_desc.SizeInBytes;
				desc.counted = false;

				init(frame);
			}
			Handle get_uav_clear() { return rwRAW; }

			void write(UINT64 offset, T* data, UINT64 count)
			{
				memcpy(resource->buffer_data + offset, data, sizeof(Underlying<T>) * count);
			}

		};


	}
}