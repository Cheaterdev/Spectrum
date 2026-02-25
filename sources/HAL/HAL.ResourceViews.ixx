export module HAL:ResourceViews;
import <HAL.h>;
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

			operator bool() const
			{
				return !!get_resource();
			}

			SERIALIZE()
			{
			}
		};

		class BufferView : public ResourceView
		{
		public:
			using ResourceType = Buffer;
			Buffer::ptr resource;

			Resource* get_resource() const override
			{
				return resource.get();
			}

			BufferView() = default;

			BufferView(const Buffer::ptr& _resource) : resource(_resource)
			{
			}

			virtual member_item::ptr describe(void* data, uint64 size) {return nullptr;}
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

			TextureResource* get_resource() const override
			{
				return resource.get();
			}

		public:
			TextureView() = default;

			TextureView(const TextureResource::ptr& _resource) : resource(_resource)
			{
			}


			virtual member_item::ptr describe() {
				return nullptr;
			}


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
				memcpy(resource->buffer_data + offset, data, sizeof(Underlying<T>) * count);
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
			void init(GPUEntityStorageInterface& frame, TextureViewDesc _view_desc)
			{
				view_desc = _view_desc;

				auto& desc = get_desc().as_texture();

				if (view_desc.MipLevels == 0)
				{
					view_desc.MipLevels = desc.MipLevels - view_desc.MipSlice;
				}

				auto hlsl = frame.alloc_descriptor(4, DescriptorHeapIndex{
					                                   HAL::DescriptorHeapType::CBV_SRV_UAV,
					                                   HAL::DescriptorHeapFlags::ShaderVisible
				                                   });

				PROFILE(L"create_views");

				if (desc.is2D())
				{
					texture2D = HLSL::Texture2D<>(hlsl[0]);
					rwTexture2D = HLSL::RWTexture2D<>(hlsl[1]);

					texture2DArray = HLSL::Texture2DArray<>(hlsl[2]);
				}


				if (check(get_desc().Flags & HAL::ResFlags::Virtual))
				{
					feedback = HLSL::FeedbackTexture2DMip(hlsl[3]);
				}


				if (check(get_desc().Flags & HAL::ResFlags::ShaderResource))
				{
					if (desc.is2D() && view_desc.ArraySize == 1)
						texture2D.create(resource, view_desc.MipSlice, view_desc.MipLevels, view_desc.FirstArraySlice);
					else if (desc.is2D())
						texture2DArray.create(resource, view_desc.MipSlice, view_desc.MipLevels,
						                      view_desc.FirstArraySlice, view_desc.ArraySize);
						//	else if (desc.is3D())
						//		texture3D.create(resource, view_desc.MipSlice, view_desc.MipLevels);
					else
						assert(false);
				}

				if (check(get_desc().Flags & HAL::ResFlags::UnorderedAccess))
				{
					if (desc.is2D() && view_desc.ArraySize == 1)
						rwTexture2D.create(resource, view_desc.MipSlice, view_desc.FirstArraySlice);
						//	else if (desc.is3D())
						//		rwTexture3D.create(resource, view_desc.MipSlice);
					else
						assert(false);
				}

				if (check(get_desc().Flags & HAL::ResFlags::RenderTarget))
				{
					auto rtv = frame.alloc_descriptor(1, DescriptorHeapIndex{
						                                  HAL::DescriptorHeapType::RTV, HAL::DescriptorHeapFlags::None
					                                  });

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

				if (check(get_desc().Flags & HAL::ResFlags::DepthStencil))
				{
					auto dsv = frame.alloc_descriptor(1, DescriptorHeapIndex{
						                                  HAL::DescriptorHeapType::DSV, HAL::DescriptorHeapFlags::None
					                                  });
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

			Texture2DView() = default;


			Texture2DView(const TextureResource::ptr& resource, GPUEntityStorageInterface& frame) : TextureView(
				resource)
			{
				auto& texture_desc = get_desc().as_texture();
				uint array_size = texture_desc.ArraySize;

				init(frame, {0, texture_desc.MipLevels, 0, array_size});
			}

			Texture2DView(const TextureResource::ptr& resource, GPUEntityStorageInterface& frame,
			              TextureViewDesc vdesc) : TextureView(resource)
			{
				init(frame, vdesc);
			}


			Viewport get_viewport()
			{
				auto& texture_desc = get_desc().as_texture();

				UINT scaler = 1 << view_desc.MipSlice;


				Viewport p;
				p.size = float2::max(float2{1.0f, 1.0f}, float2(texture_desc.Dimensions.xy) / scaler);
				p.pos = {0, 0};
				p.depths = {0, 1};

				return p;
			}


			sizer_long get_scissor()
			{
				UINT scaler = 1 << view_desc.MipSlice;
				auto& texture_desc = get_desc().as_texture();

				return {
					0, 0, std::max(1u, texture_desc.Dimensions.x / scaler),
					std::max(1u, texture_desc.Dimensions.y / scaler)
				};
			}

			UINT get_mip_count()
			{
				return view_desc.MipLevels;
			}

			ivec2 get_size()
			{
				auto& texture_desc = get_desc().as_texture();

				UINT scaler = 1 << view_desc.MipSlice;

				return uint2::max(uint2{1, 1}, uint2(texture_desc.Dimensions.xy) / scaler);
			}

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
			void init(GPUEntityStorageInterface& frame, Texture3DViewDesc _view_desc)
			{
				view_desc = _view_desc;

				auto& desc = get_desc().as_texture();

				if (view_desc.MipLevels == 0)
				{
					view_desc.MipLevels = desc.MipLevels - view_desc.MipSlice;
				}
				auto hlsl = frame.alloc_descriptor(1 + view_desc.MipLevels * 2, DescriptorHeapIndex{
					                                   HAL::DescriptorHeapType::CBV_SRV_UAV,
					                                   HAL::DescriptorHeapFlags::ShaderVisible
				                                   });

				assert(desc.is3D());


				uint offset = 0;
				mips.resize(view_desc.MipLevels);
				if (check(get_desc().Flags & HAL::ResFlags::ShaderResource))
				{
					texture3D = HLSL::Texture3D<>(hlsl[offset++]);
					texture3D.create(resource, view_desc.MipSlice, view_desc.MipLevels);

					for (uint i = 0; i < view_desc.MipLevels; i++)
					{
						mips[i].texture3D = HLSL::Texture3D<>(hlsl[offset++]);
						mips[i].texture3D.create(resource, view_desc.MipSlice + i, 1);
					}
				}

				if (check(get_desc().Flags & HAL::ResFlags::UnorderedAccess))
				{
					for (uint i = 0; i < view_desc.MipLevels; i++)
					{
						mips[i].rwTexture3D = HLSL::RWTexture3D<>(hlsl[offset++]);
						mips[i].rwTexture3D.create(resource, view_desc.MipSlice + i);
					}
				}
			}

			Texture3DView() = default;


			Texture3DView(const TextureResource::ptr& resource, GPUEntityStorageInterface& frame) : TextureView(
				resource)
			{
				auto& texture_desc = get_desc().as_texture();
				uint array_size = texture_desc.ArraySize;

				init(frame, {0, texture_desc.MipLevels});
			}

			Texture3DView(const TextureResource::ptr& resource, GPUEntityStorageInterface& frame,
			              Texture3DViewDesc vdesc) : TextureView(resource)
			{
				init(frame, vdesc);
			}


			Viewport get_viewport()
			{
				auto& texture_desc = get_desc().as_texture();

				UINT scaler = 1 << view_desc.MipSlice;


				Viewport p;
				p.size = float2::max(float2{1.0f, 1.0f}, float2(texture_desc.Dimensions.xy) / scaler);
				p.pos = {0, 0};
				p.depths = {0, 1};
				return p;
			}


			sizer_long get_scissor()
			{
				UINT scaler = 1 << view_desc.MipSlice;
				auto& texture_desc = get_desc().as_texture();

				return {
					0, 0, std::max(1u, texture_desc.Dimensions.x / scaler),
					std::max(1u, texture_desc.Dimensions.y / scaler)
				};
			}

			UINT get_mip_count()
			{
				return view_desc.MipLevels;
			}

			ivec2 get_size()
			{
				auto& texture_desc = get_desc().as_texture();

				UINT scaler = 1 << view_desc.MipSlice;

				return uint2::max(uint2{1, 1}, uint2(texture_desc.Dimensions.xy) / scaler);
			}
		};


		class CubeView : public TextureView
		{
			CubeViewDesc view_desc;

		public:
			HLSL::TextureCube<> textureCube;


			std::array<Texture2DView, 6> faces;

		public:
			void init(GPUEntityStorageInterface& frame, CubeViewDesc _view_desc)
			{
				view_desc = _view_desc;

				auto& desc = get_desc().as_texture();

				if (view_desc.MipLevels == 0)
				{
					view_desc.MipLevels = desc.MipLevels - view_desc.MipSlice;
				}
				auto hlsl = frame.alloc_descriptor(1, DescriptorHeapIndex{
					                                   HAL::DescriptorHeapType::CBV_SRV_UAV,
					                                   HAL::DescriptorHeapFlags::ShaderVisible
				                                   });
				assert(desc.is2D());
				textureCube = HLSL::TextureCube<>(hlsl[0]);


				if (check(get_desc().Flags & HAL::ResFlags::ShaderResource))
				{
					assert(desc.is2D());
					textureCube.create(resource, view_desc.MipSlice, view_desc.MipLevels,
					                   view_desc.FirstArraySlice / 6);
				}

				if (check(get_desc().Flags & HAL::ResFlags::UnorderedAccess))
				{
					TextureViewDesc desc;

					desc.MipSlice = view_desc.MipSlice;
					desc.MipLevels = view_desc.MipLevels;
					desc.ArraySize = 1;


					for (uint i = 0; i < 6; i++)
					{
						desc.FirstArraySlice = view_desc.FirstArraySlice + i;
						faces[i] = Texture2DView(resource, frame, desc);
					}
				}
			}

			CubeView() = default;


			CubeView(const TextureResource::ptr& resource, GPUEntityStorageInterface& frame) : TextureView(resource)
			{
				auto& texture_desc = get_desc().as_texture();
				uint array_size = texture_desc.ArraySize / 6;

				init(frame, {0, texture_desc.MipLevels, 0, array_size});
			}

			CubeView(const TextureResource::ptr& resource, GPUEntityStorageInterface& frame,
			         CubeViewDesc vdesc) : TextureView(resource)
			{
				init(frame, vdesc);
			}


			Viewport get_viewport()
			{
				auto& texture_desc = get_desc().as_texture();

				UINT scaler = 1 << view_desc.MipSlice;


				Viewport p;
				p.size = float2::max(float2{1.0f, 1.0f}, float2(texture_desc.Dimensions.xy) / scaler);
				p.pos = {0, 0};
				p.depths = {0, 1};

				return p;
			}


			sizer_long get_scissor()
			{
				UINT scaler = 1 << view_desc.MipSlice;
				auto& texture_desc = get_desc().as_texture();

				return {
					0, 0, std::max(1u, texture_desc.Dimensions.x / scaler),
					std::max(1u, texture_desc.Dimensions.y / scaler)
				};
			}

			ivec2 get_size()
			{
				auto& texture_desc = get_desc().as_texture();

				UINT scaler = 1 << view_desc.MipSlice;

				return uint2::max(uint2{1, 1}, uint2(texture_desc.Dimensions.xy) / scaler);
			}

			Texture2DView get_face(UINT face);

			CubeView create_mip(UINT mip, GPUEntityStorageInterface& frame);
		};

		class ByteBufferView : public BufferView
		{
		public:
			HLSL::ByteAddressBuffer byteBuffer;
			HLSL::RWByteAddressBuffer rwbyteBuffer;

			ByteBufferView() = default;

			ByteBufferView(const Resource::ptr& resource, GPUEntityStorageInterface& frame, UINT offset = 0,
			               UINT64 size = 0) : BufferView(std::static_pointer_cast<Buffer>(resource))
			{
				auto hlsl = frame.alloc_descriptor(2, DescriptorHeapIndex{
					                                   HAL::DescriptorHeapType::CBV_SRV_UAV,
					                                   HAL::DescriptorHeapFlags::ShaderVisible
				                                   });

				byteBuffer = HLSL::ByteAddressBuffer(hlsl[0]);
				rwbyteBuffer = HLSL::RWByteAddressBuffer(hlsl[1]);

				auto& desc = get_desc().as_buffer();

				if (check(get_desc().Flags & HAL::ResFlags::ShaderResource))
				{
					byteBuffer.create(resource, offset, size);
				}

				if (check(get_desc().Flags & HAL::ResFlags::UnorderedAccess))
				{
					rwbyteBuffer.create(resource, offset, size);
				}
			}

			template <class T>
			void write(UINT64 offset, T* data, UINT64 count)
			{
				memcpy(resource->buffer_data + offset, data, sizeof(Underlying<T>) * count);
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

			CounterView(const Resource::ptr& resource, GPUEntityStorageInterface& frame, uint64 offset = 0) :
				BufferView(std::static_pointer_cast<Buffer>(resource)), offset(offset)
			{
				auto hlsl = frame.alloc_descriptor(3, DescriptorHeapIndex{
					                                   HAL::DescriptorHeapType::CBV_SRV_UAV,
					                                   HAL::DescriptorHeapFlags::ShaderVisible
				                                   });

				structuredBuffer = HLSL::StructuredBuffer<UINT>(hlsl[0]);
				rwStructuredBuffer = HLSL::RWStructuredBuffer<UINT>(hlsl[1]);
				rwRAW = HLSL::RWBuffer<std::byte>(hlsl[2]);
				auto& desc = get_desc().as_buffer();

				if (check(get_desc().Flags & HAL::ResFlags::ShaderResource))
				{
					structuredBuffer.create(resource, offset / sizeof(UINT), 1);
				}

				if (check(get_desc().Flags & HAL::ResFlags::UnorderedAccess))
				{
					rwStructuredBuffer.create(resource, offset / sizeof(UINT), 1);
					rwRAW.create(resource, Format::R8_UINT, offset, sizeof(UINT));
				}
			}
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
				return offset + offset * sizeof(UnderlyingType);
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

				auto hlsl = frame.alloc_descriptor(5, DescriptorHeapIndex{
					                                   HAL::DescriptorHeapType::CBV_SRV_UAV,
					                                   HAL::DescriptorHeapFlags::ShaderVisible
				                                   });

				structuredBuffer = HLSL::StructuredBuffer<T>(hlsl[0]);
				rwStructuredBuffer = HLSL::RWStructuredBuffer<T>(hlsl[1]);
				appendStructuredBuffer = HLSL::AppendStructuredBuffer<T>(hlsl[2]);


				rwRAW = HLSL::RWBuffer<std::byte>(hlsl[3]);

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
						appendStructuredBuffer.create(get_counter_buffer(), get_counter_offset(), resource,
						                              static_cast<UINT>(offset / sizeof(Underlying<T>)),
						                              static_cast<UINT>(size / sizeof(Underlying<T>)));
				}
			}


			StructuredBufferView(const Resource::ptr& resource, GPUEntityStorageInterface& frame,
			                     StructuredBufferViewDesc _desc) :
				StructuredBufferViewBase(std::static_pointer_cast<Buffer>(resource)), desc(_desc)
			{
				assert(std::dynamic_pointer_cast<Buffer>(resource));
				init(frame);
			}


			StructuredBufferView(const Resource::ptr& resource, GPUEntityStorageInterface& frame) : StructuredBufferViewBase(
				std::static_pointer_cast<Buffer>(resource))
			{
				assert(std::dynamic_pointer_cast<Buffer>(resource));
				auto& res_desc = get_desc().as_buffer();

				desc.offset = 0;
				desc.size = res_desc.SizeInBytes;
				desc.counted = counterType::NONE;

				init(frame);
			}


			StructuredBufferView(uint64 count, counterType counted = counterType::NONE,
			                     HAL::ResFlags flags = HAL::ResFlags::ShaderResource,
			                     HAL::HeapType heap_type = HAL::HeapType::DEFAULT,
			                     GPUEntityStorageInterface& frame = Device::get().get_static_gpu_data()) :
				StructuredBufferViewBase(std::make_shared<HAL::Buffer>(
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
					counter_view.resource = std::make_shared<HAL::Buffer>(
						HAL::ResourceDesc::Buffer(4, flags), HeapType::DEFAULT);
				}
				init(frame);
			}

			Handle get_uav_clear() { return rwRAW; }

			void write(UINT64 offset, const T* data, UINT64 count)
			{
				memcpy(resource->buffer_data + offset, data, sizeof(Underlying<T>) * count);
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
					init(HAL::Device::get().get_static_gpu_data());
				}
			}
		};


		using IndexBuffer = StructuredBufferView<unsigned int>;
	}
}
