export module HAL:ResourceViews;

import :Types;
import :HLSL;
import :Device;
import :FrameManager;
import :Resource.Buffer;
import :Resource.Texture;

import Core;
export
{
	namespace HAL
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
			virtual Resource* get_resource() const = 0;

		public:
			ResourceView() = default;

			auto get_desc() const
			{
				return get_resource()->get_desc();
			}

			operator bool() const;

			SERIALIZE()
			{
			}
		};

		class BufferView : public ResourceView
		{
		public:
			using ResourceType = Buffer;
			Buffer::ptr resource;

			Resource* get_resource() const override;

			BufferView() = default;

			BufferView(const Buffer::ptr& _resource);

			virtual member_item::ptr describe(void* data, uint64 size);
			SERIALIZE()
			{
				ar & NVP(resource);
			}
		};

		class StructuredBufferViewBase: public BufferView
		{

		public:

			using BufferView::BufferView;

		   virtual uint get_element_size() const =0;
		   		   virtual 	uint64 get_count() const=0;
	
		};
		class TextureView : public ResourceView
		{
		public:
			using ResourceType = TextureResource;
			TextureResource::ptr resource;

			TextureResource* get_resource() const override;

		public:
			TextureView() = default;

			TextureView(const TextureResource::ptr& _resource);


			virtual member_item::ptr describe();


			SERIALIZE()
			{
				ar & NVP(resource);
			}
		};


		struct FormattedBufferViewDesc
		{
			uint64 offset;
			uint64 size;
		};

		template <class T, Format::Formats _format>
		class FormattedBufferView : public BufferView
		{
		public:
			using Desc = FormattedBufferViewDesc;

			Desc desc;
			FormattedBufferView() = default;
			HLSL::Buffer<T> buffer;
			HLSL::RWBuffer<T> rwBuffer;

			void init(GPUEntityStorageInterface& frame)
			{
				Format format = _format;
				buffer = HLSL::Buffer<T>(frame.alloc_descriptor(1, DescriptorHeapIndex{
					                                                HAL::DescriptorHeapType::CBV_SRV_UAV,
					                                                HAL::DescriptorHeapFlags::ShaderVisible
				                                                }));
				buffer.create(resource, format, static_cast<UINT>(desc.offset / sizeof(Underlying<T>)),
				              static_cast<UINT>(desc.size / sizeof(Underlying<T>)));

				//
				if (check(get_desc().Flags & HAL::ResFlags::UnorderedAccess))
				{
					rwBuffer = HLSL::RWBuffer<T>(frame.alloc_descriptor(1, DescriptorHeapIndex{
						                                                    HAL::DescriptorHeapType::CBV_SRV_UAV,
						                                                    HAL::DescriptorHeapFlags::ShaderVisible
					                                                    }));
					rwBuffer.create(resource, format, static_cast<UINT>(desc.offset / sizeof(Underlying<T>)),
					                static_cast<UINT>(desc.size / sizeof(Underlying<T>)));
				}
			}


			FormattedBufferView(const Resource::ptr& resource, GPUEntityStorageInterface& frame,
			                    FormattedBufferViewDesc _desc) : BufferView(std::static_pointer_cast<Buffer>(resource)),
			                                                     desc(_desc)
			{
				init(frame);
			}


			FormattedBufferView(const Resource::ptr& resource, GPUEntityStorageInterface& frame) : BufferView(
				std::static_pointer_cast<Buffer>(resource))
			{
				auto& res_desc = get_desc().as_buffer();

				desc.offset = 0;
				desc.size = res_desc.SizeInBytes;

				init(frame);
			}

			~FormattedBufferView()
			{
			}

			void write(UINT64 offset, T* data, UINT64 count)
			{
				std::memcpy(resource->buffer_data + offset, data, sizeof(Underlying<T>) * count);
			}

			ResourceAddress get_resource_address(uint offset = 0) const
			{
				return resource->get_resource_address().offset(desc.offset + offset * sizeof(Underlying<T>));
			}
		};


		class RTXSceneView : public BufferView
		{
			Buffer::ptr resource_buffer;

		public:
			HLSL::RaytracingAccelerationStructure scene;

			RTXSceneView() = default;
			RTXSceneView(const Resource::ptr& resource, GPUEntityStorageInterface& frame);
		};

		class Texture2DView : public TextureView
		{
			TextureViewDesc view_desc;

		public:
			HLSL::Texture2D<> texture2D;
			HLSL::RWTexture2D<> rwTexture2D;

			HLSL::FeedbackTexture2DMip feedback;
			//	HLSL::Texture3D<> texture3D;
			//	HLSL::RWTexture3D<> rwTexture3D;


			HLSL::Texture2DArray<> texture2DArray;

			HLSL::RenderTarget<> renderTarget;
			HLSL::DepthStencil<> depthStencil;

		public:
			void init(GPUEntityStorageInterface& frame, TextureViewDesc _view_desc);

			Texture2DView() = default;


			Texture2DView(const TextureResource::ptr& resource, GPUEntityStorageInterface& frame);

			Texture2DView(const TextureResource::ptr& resource, GPUEntityStorageInterface& frame,
			              TextureViewDesc vdesc);


			Viewport get_viewport();

			sizer_long get_scissor();

			UINT get_mip_count();

			ivec2 get_size();

			Texture2DView create_2d_slice(UINT slice, GPUEntityStorageInterface& frame);

			Texture2DView create_mip(UINT mip, GPUEntityStorageInterface& frame);
		};


		class Texture3DView : public TextureView
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
			void init(GPUEntityStorageInterface& frame, Texture3DViewDesc _view_desc);

			Texture3DView() = default;


			Texture3DView(const TextureResource::ptr& resource, GPUEntityStorageInterface& frame);

			Texture3DView(const TextureResource::ptr& resource, GPUEntityStorageInterface& frame,
			              Texture3DViewDesc vdesc);


			Viewport get_viewport();

			sizer_long get_scissor();

			UINT get_mip_count();

			ivec2 get_size();
		};


		class CubeView : public TextureView
		{
			CubeViewDesc view_desc;

		public:
			HLSL::TextureCube<> textureCube;


			std::array<Texture2DView, 6> faces;

		public:
			void init(GPUEntityStorageInterface& frame, CubeViewDesc _view_desc);

			CubeView() = default;


			CubeView(const TextureResource::ptr& resource, GPUEntityStorageInterface& frame);

			CubeView(const TextureResource::ptr& resource, GPUEntityStorageInterface& frame,
			         CubeViewDesc vdesc);


			Viewport get_viewport();

			sizer_long get_scissor();

			ivec2 get_size();

			Texture2DView get_face(UINT face);

			CubeView create_mip(UINT mip, GPUEntityStorageInterface& frame);
		};

			struct ByteBufferViewDesc
			{
				uint64 offset;
		 		uint64 size;
		
			};
		class ByteBufferView : public BufferView
		{
		public:
			using Desc = ByteBufferViewDesc;
			HLSL::ByteAddressBuffer byteBuffer;
			HLSL::RWByteAddressBuffer rwbyteBuffer;

			ByteBufferView() = default;

			ByteBufferView(const Resource::ptr& resource, GPUEntityStorageInterface& frame, ByteBufferViewDesc viewdesc);

			template <class T>
			void write(UINT64 offset, T* data, UINT64 count)
			{
				std::memcpy(resource->buffer_data + offset, data, sizeof(Underlying<T>) * count);
			}
		};


		class CounterView : public BufferView
		{
		public:
			HLSL::StructuredBuffer<UINT> structuredBuffer;
			HLSL::RWStructuredBuffer<UINT> rwStructuredBuffer;
			HLSL::RWBuffer<std::byte> rwRAW;
			CounterView() = default;
			uint64 offset;

			CounterView(const Resource::ptr& resource, GPUEntityStorageInterface& frame, uint64 offset = 0);
		};


		struct StructuredBufferViewDesc
		{
			uint64 offset;
			uint64 size;
			counterType counted;

			SERIALIZE()
			{
				ar & NVP(offset);
				ar & NVP(size);
				ar & NVP(counted);
			}
		};

		template <class T>
		class StructuredBufferView : public StructuredBufferViewBase
		{
			uint offset;

		public:
			using Desc = StructuredBufferViewDesc;
			using UnderlyingType = Underlying<T>;

			Desc desc;
			HLSL::RWBuffer<std::byte> rwRAW;
			//	HLSL::ConstBuffer<T> constBuffer;
			HLSL::StructuredBuffer<T> structuredBuffer;
			HLSL::RWStructuredBuffer<T> rwStructuredBuffer;
			HLSL::AppendStructuredBuffer<T> appendStructuredBuffer;
		  	HLSL::ConsumeStructuredBuffer<T> consumeStructuredBuffer;

			StructuredBufferView() = default;
			CounterView counter_view;

		public:

			virtual uint get_element_size() const  override
			{
				return sizeof(UnderlyingType);
			}




				 member_item::ptr describe(void* data, uint64 size) override {


				return ObjectTreeSerializer::describe(std::span(static_cast<T*>(data), size/get_element_size()));
			}


		   


			uint get_data_offset() const
			{
				return offset / sizeof(UnderlyingType);
			}

			uint get_data_offset_in_bytes(uint offset = 0) const
			{
				return this->offset + offset * sizeof(UnderlyingType);
			}


			uint get_counter_offset() const
			{
				return 0;
			}

			Buffer::ptr get_counter_buffer()
			{
				if (desc.counted == counterType::HELP_BUFFER)
					return counter_view.resource;

				return resource;
			}

			uint64 get_count()  const	 override
			{
				return desc.size / sizeof(UnderlyingType);
			}

			void init(GPUEntityStorageInterface& frame)
			{
				offset = uint(desc.offset);
				uint64 size = desc.size;

				if (desc.counted == counterType::SELF)
				{
					uint local_offset = 0;
					local_offset = Math::roundUp(4, sizeof(Underlying<T>));
					if (size == 0) size = get_desc().as_buffer().SizeInBytes;
					size -= local_offset;
					offset += local_offset;

					counter_view = resource->create_view<CounterView>(frame, desc.offset);
				}

				if (desc.counted == counterType::HELP_BUFFER)
				{
					counter_view = counter_view.resource->create_view<CounterView>(frame, 0);
				}

				auto hlsl = frame.alloc_descriptor(6, DescriptorHeapIndex{
					                                   HAL::DescriptorHeapType::CBV_SRV_UAV,
					                                   HAL::DescriptorHeapFlags::ShaderVisible
				                                   });

				structuredBuffer = HLSL::StructuredBuffer<T>(hlsl[0]);
				rwStructuredBuffer = HLSL::RWStructuredBuffer<T>(hlsl[1]);
				appendStructuredBuffer = HLSL::AppendStructuredBuffer<T>(hlsl[2]);
				consumeStructuredBuffer		  = HLSL::ConsumeStructuredBuffer<T>(hlsl[3]);

				rwRAW = HLSL::RWBuffer<std::byte>(hlsl[4]);

				//		constBuffer = HLSL::ConstBuffer<T>(hlsl[4]);

				//		constBuffer.create(resource, offset,size);

				if (check(get_desc().Flags & HAL::ResFlags::ShaderResource) && !check(
					get_desc().Flags & HAL::ResFlags::Raytracing))
				{
					structuredBuffer.create(resource, static_cast<UINT>(offset / sizeof(Underlying<T>)),
					                        static_cast<UINT>(size / sizeof(Underlying<T>)));
				}


				if (check(get_desc().Flags & HAL::ResFlags::UnorderedAccess) && !check(
					get_desc().Flags & HAL::ResFlags::Raytracing))
				{
					rwStructuredBuffer.create(resource, static_cast<UINT>(offset / sizeof(Underlying<T>)),
					                          static_cast<UINT>(size / sizeof(Underlying<T>)));
					rwRAW.create(resource, Format::R8_UINT, static_cast<UINT>(offset), static_cast<UINT>(size));

					if (desc.counted != counterType::NONE)
					{
										appendStructuredBuffer.create(get_counter_buffer(), get_counter_offset(), resource,
						                              static_cast<UINT>(offset / sizeof(Underlying<T>)),
						                              static_cast<UINT>(size / sizeof(Underlying<T>)));

						consumeStructuredBuffer.create(get_counter_buffer(), get_counter_offset(), resource,
						                              static_cast<UINT>(offset / sizeof(Underlying<T>)),
						                              static_cast<UINT>(size / sizeof(Underlying<T>)));
					}
		
				}
			}


			StructuredBufferView(const Resource::ptr& resource, GPUEntityStorageInterface& frame,
			                     StructuredBufferViewDesc _desc) :
				StructuredBufferViewBase(std::static_pointer_cast<Buffer>(resource)), desc(_desc)
			{
				ASSERT(std::dynamic_pointer_cast<Buffer>(resource));
				init(frame);
			}


			StructuredBufferView(const Resource::ptr& resource, GPUEntityStorageInterface& frame) : StructuredBufferViewBase(
				std::static_pointer_cast<Buffer>(resource))
			{
				ASSERT(std::dynamic_pointer_cast<Buffer>(resource));
				auto& res_desc = get_desc().as_buffer();

				desc.offset = 0;
				desc.size = res_desc.SizeInBytes;
				desc.counted = counterType::NONE;

				init(frame);
			}


			StructuredBufferView(Device& device, uint64 count, counterType counted = counterType::NONE,
			                     HAL::ResFlags flags = HAL::ResFlags::ShaderResource,
			                     HAL::HeapType heap_type = HAL::HeapType::DEFAULT) :
				StructuredBufferViewBase(std::make_shared<HAL::Buffer>(device,
					HAL::ResourceDesc::Buffer(
						count * sizeof(T) + (counted == counterType::SELF) * Math::roundUp(4, sizeof(Underlying<T>)),
						flags), heap_type))

			{
				resource->set_name("StructuredBufferView");
				auto& res_desc = get_desc().as_buffer();

				desc.offset = 0;
				desc.size = res_desc.SizeInBytes;
				desc.counted = counted;

				if (desc.counted == counterType::HELP_BUFFER)
				{
					counter_view.resource = std::make_shared<HAL::Buffer>(device,
						HAL::ResourceDesc::Buffer(4, flags), HeapType::DEFAULT);
				}
				init(device.get_static_gpu_data());
			}

			Handle get_uav_clear() { return rwRAW; }

			void write(UINT64 offset, const T* data, UINT64 count)
			{
				std::memcpy(resource->buffer_data + offset, data, sizeof(Underlying<T>) * count);
			}


			ResourceAddress get_resource_address(uint offset = 0) const
			{
				return resource->get_resource_address().offset(desc.offset + offset * sizeof(Underlying<T>));
			}


			HAL::Views::IndexBuffer get_index_buffer_view() //requires (std::is_same_v(T, uint))
			{
				HAL::Views::IndexBuffer view;
				view.OffsetInBytes = desc.offset;
				view.Format = HAL::Format::R32_UINT; // is_32 ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;
				view.SizeInBytes = desc.size;
				view.Resource = resource;
				return view;
			}

			SERIALIZE()
			{
				ar & NVP(resource);
				ar & NVP(desc);

				IF_LOAD()
				{
					if (resource)
						init(resource->get_device().get_static_gpu_data());
				}
			}
		};


		using IndexBuffer = StructuredBufferView<unsigned int>;
	}
}
