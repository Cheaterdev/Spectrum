module HAL:ResourceViews;

import :Types;

namespace HAL
{

	RTXSceneView::RTXSceneView(const Resource::ptr& resource, GPUEntityStorageInterface& frame) :BufferView(std::static_pointer_cast<Buffer>(resource))
	{
		/*	init_desc();*/
		scene = HLSL::RaytracingAccelerationStructure(frame.alloc_descriptor(1, DescriptorHeapIndex{ HAL::DescriptorHeapType::CBV_SRV_UAV, HAL::DescriptorHeapFlags::ShaderVisible }));
		
		scene.create(resource);
	}



	Texture2DView Texture2DView::create_2d_slice(UINT slice, GPUEntityStorageInterface& frame)
	{
		TextureViewDesc desc = view_desc;

		desc.ArraySize = 1;
		desc.FirstArraySlice = slice;

		return Texture2DView(resource, frame, desc);
	}

	Texture2DView Texture2DView::create_mip(UINT mip, GPUEntityStorageInterface& frame)
	{
		TextureViewDesc desc = view_desc;

		desc.MipSlice += mip;
		desc.MipLevels = 1;

		return Texture2DView(resource, frame, desc);
	}
	Texture2DView CubeView::get_face(UINT face)
	{
		return faces[face];
	}

	CubeView CubeView::create_mip(UINT mip, GPUEntityStorageInterface& frame)
	{
		CubeViewDesc desc = view_desc;

		desc.MipSlice += mip;
		desc.MipLevels = 1;

		return CubeView(resource, frame, desc);
	}


	ResourceView::operator bool() const
	{
		return !!get_resource();
	}


	Resource* BufferView::get_resource() const
	{
		return resource.get();
	}

	BufferView::BufferView(const Buffer::ptr& _resource) : resource(_resource)
	{
	}

	member_item::ptr BufferView::describe(void* data, uint64 size)
	{
		return nullptr;
	}


	TextureResource* TextureView::get_resource() const
	{
		return resource.get();
	}

	TextureView::TextureView(const TextureResource::ptr& _resource) : resource(_resource)
	{
	}

	member_item::ptr TextureView::describe()
	{
		return nullptr;
	}


	void Texture2DView::init(GPUEntityStorageInterface& frame, TextureViewDesc _view_desc)
	{
		view_desc = _view_desc;

		auto& desc = get_desc().as_texture();

		if (view_desc.MipLevels == 0)
		{
			view_desc.MipLevels = desc.MipLevels - view_desc.MipSlice;
		}

		auto hlsl = frame.alloc_descriptor(5, DescriptorHeapIndex{
			                                   HAL::DescriptorHeapType::CBV_SRV_UAV,
			                                   HAL::DescriptorHeapFlags::ShaderVisible
		                                   });

		PROFILE(L"create_views");

		if (desc.is2D())
		{
			texture2D = HLSL::Texture2D<>(hlsl[0]);
			rwTexture2D = HLSL::RWTexture2D<>(hlsl[1]);

			texture2DArray = HLSL::Texture2DArray<>(hlsl[2]);
			rwTexture2DArray = HLSL::RWTexture2DArray<>(hlsl[4]);
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
				ASSERT(false);
		}

		if (check(get_desc().Flags & HAL::ResFlags::UnorderedAccess))
		{
			if (desc.is2D() && view_desc.ArraySize == 1)
				rwTexture2D.create(resource, view_desc.MipSlice, view_desc.FirstArraySlice);
			else if (desc.is2D())
				rwTexture2DArray.create(resource, view_desc.MipSlice, 1, view_desc.FirstArraySlice, view_desc.ArraySize);
				//	else if (desc.is3D())
				//		rwTexture3D.create(resource, view_desc.MipSlice);
			else
				ASSERT(false);
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

	Texture2DView::Texture2DView(const TextureResource::ptr& resource, GPUEntityStorageInterface& frame) : TextureView(
		resource)
	{
		auto& texture_desc = get_desc().as_texture();
		uint array_size = texture_desc.ArraySize;

		init(frame, {0, texture_desc.MipLevels, 0, array_size});
	}

	Texture2DView::Texture2DView(const TextureResource::ptr& resource, GPUEntityStorageInterface& frame,
	                             TextureViewDesc vdesc) : TextureView(resource)
	{
		init(frame, vdesc);
	}

	Viewport Texture2DView::get_viewport()
	{
		auto& texture_desc = get_desc().as_texture();

		UINT scaler = 1 << view_desc.MipSlice;


		Viewport p;
		p.size = float2::max(float2{1.0f, 1.0f}, float2(texture_desc.Dimensions.xy) / scaler);
		p.pos = {0, 0};
		p.depths = {0, 1};

		return p;
	}

	sizer_long Texture2DView::get_scissor()
	{
		UINT scaler = 1 << view_desc.MipSlice;
		auto& texture_desc = get_desc().as_texture();

		return {
			0, 0, std::max(1u, texture_desc.Dimensions.x / scaler),
			std::max(1u, texture_desc.Dimensions.y / scaler)
		};
	}

	UINT Texture2DView::get_mip_count()
	{
		return view_desc.MipLevels;
	}

	ivec2 Texture2DView::get_size()
	{
		auto& texture_desc = get_desc().as_texture();

		UINT scaler = 1 << view_desc.MipSlice;

		return uint2::max(uint2{1, 1}, uint2(texture_desc.Dimensions.xy) / scaler);
	}


	void Texture3DView::init(GPUEntityStorageInterface& frame, Texture3DViewDesc _view_desc)
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

		ASSERT(desc.is3D());


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

	Texture3DView::Texture3DView(const TextureResource::ptr& resource, GPUEntityStorageInterface& frame) : TextureView(
		resource)
	{
		auto& texture_desc = get_desc().as_texture();
		uint array_size = texture_desc.ArraySize;

		init(frame, {0, texture_desc.MipLevels});
	}

	Texture3DView::Texture3DView(const TextureResource::ptr& resource, GPUEntityStorageInterface& frame,
	                             Texture3DViewDesc vdesc) : TextureView(resource)
	{
		init(frame, vdesc);
	}

	Viewport Texture3DView::get_viewport()
	{
		auto& texture_desc = get_desc().as_texture();

		UINT scaler = 1 << view_desc.MipSlice;


		Viewport p;
		p.size = float2::max(float2{1.0f, 1.0f}, float2(texture_desc.Dimensions.xy) / scaler);
		p.pos = {0, 0};
		p.depths = {0, 1};
		return p;
	}

	sizer_long Texture3DView::get_scissor()
	{
		UINT scaler = 1 << view_desc.MipSlice;
		auto& texture_desc = get_desc().as_texture();

		return {
			0, 0, std::max(1u, texture_desc.Dimensions.x / scaler),
			std::max(1u, texture_desc.Dimensions.y / scaler)
		};
	}

	UINT Texture3DView::get_mip_count()
	{
		return view_desc.MipLevels;
	}

	ivec2 Texture3DView::get_size()
	{
		auto& texture_desc = get_desc().as_texture();

		UINT scaler = 1 << view_desc.MipSlice;

		return uint2::max(uint2{1, 1}, uint2(texture_desc.Dimensions.xy) / scaler);
	}


	void CubeView::init(GPUEntityStorageInterface& frame, CubeViewDesc _view_desc)
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
		ASSERT(desc.is2D());
		textureCube = HLSL::TextureCube<>(hlsl[0]);


		if (check(get_desc().Flags & HAL::ResFlags::ShaderResource))
		{
			ASSERT(desc.is2D());
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

	CubeView::CubeView(const TextureResource::ptr& resource, GPUEntityStorageInterface& frame) : TextureView(resource)
	{
		auto& texture_desc = get_desc().as_texture();
		uint array_size = texture_desc.ArraySize / 6;

		init(frame, {0, texture_desc.MipLevels, 0, array_size});
	}

	CubeView::CubeView(const TextureResource::ptr& resource, GPUEntityStorageInterface& frame,
	                   CubeViewDesc vdesc) : TextureView(resource)
	{
		init(frame, vdesc);
	}

	Viewport CubeView::get_viewport()
	{
		auto& texture_desc = get_desc().as_texture();

		UINT scaler = 1 << view_desc.MipSlice;


		Viewport p;
		p.size = float2::max(float2{1.0f, 1.0f}, float2(texture_desc.Dimensions.xy) / scaler);
		p.pos = {0, 0};
		p.depths = {0, 1};

		return p;
	}

	sizer_long CubeView::get_scissor()
	{
		UINT scaler = 1 << view_desc.MipSlice;
		auto& texture_desc = get_desc().as_texture();

		return {
			0, 0, std::max(1u, texture_desc.Dimensions.x / scaler),
			std::max(1u, texture_desc.Dimensions.y / scaler)
		};
	}

	ivec2 CubeView::get_size()
	{
		auto& texture_desc = get_desc().as_texture();

		UINT scaler = 1 << view_desc.MipSlice;

		return uint2::max(uint2{1, 1}, uint2(texture_desc.Dimensions.xy) / scaler);
	}


	ByteBufferView::ByteBufferView(const Resource::ptr& resource, GPUEntityStorageInterface& frame, ByteBufferViewDesc viewdesc) : BufferView(std::static_pointer_cast<Buffer>(resource))
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
			byteBuffer.create(resource, viewdesc.offset, viewdesc.size);
		}

		if (check(get_desc().Flags & HAL::ResFlags::UnorderedAccess))
		{
			rwbyteBuffer.create(resource, viewdesc.offset, viewdesc.size);
		}
	}


	CounterView::CounterView(const Resource::ptr& resource, GPUEntityStorageInterface& frame, uint64 offset) :
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
}
