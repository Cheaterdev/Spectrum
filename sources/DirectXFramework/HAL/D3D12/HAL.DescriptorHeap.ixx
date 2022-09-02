export module HAL:DescriptorHeap;

import :Device;
import :Utils;
import :Types;
import :Descriptors;
import :Resource;

export namespace HAL
{

	struct DescriptorHeapDesc
	{
		uint Count;
		DescriptorHeapType HeapType;
		DescriptorHeapFlags Flags;
	};

	class DescriptorHeap;
	class Descriptor
	{
		const DescriptorHeap* heap;
		const uint offset;

		Descriptor(DescriptorHeap* heap, uint offset) : heap(heap), offset(offset)
		{
		}
		//void operator=(const Handle&r);
		friend class DescriptorHeap;
	public:
		void place(const Views::ShaderResource& view);
		void place(const Views::UnorderedAccess& view);
		void place(const Views::RenderTarget& view);
		void place(const Views::ConstantBuffer& view);
		void place(const Views::DepthStencil& view);

	};

	class DescriptorHeap
	{
		D3D::DescriptorHeap m_native_heap;
		const DescriptorHeapDesc desc;
		const Device& device;

		CD3DX12_CPU_DESCRIPTOR_HANDLE cpu_start;
		friend class Descriptor;
	public:
		DescriptorHeap(Device& device, const DescriptorHeapDesc& desc) :device(device), desc(desc)
		{
			D3D12_DESCRIPTOR_HEAP_DESC native_desc = {};
			native_desc.NumDescriptors = desc.Count;
			native_desc.Type = to_native(desc.HeapType);
			native_desc.Flags = to_native(desc.Flags);
			native_desc.NodeMask = 1;

			TEST(device.native_device->CreateDescriptorHeap(&native_desc, IID_PPV_ARGS(&m_native_heap)));

			cpu_start = m_native_heap->GetCPUDescriptorHandleForHeapStart();
		}

		Descriptor operator[](uint i)
		{
			return Descriptor{ this, i };
		}
	};

	template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };

	void Descriptor::place(const Views::ShaderResource& view)
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC desc;

		desc.Format = to_native(view.Format);
		desc.Shader4ComponentMapping = view.Shader4ComponentMapping;

		std::visit(overloaded{
			[&](const Views::ShaderResource::Buffer& Buffer) {
				desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
				desc.Buffer.FirstElement = Buffer.FirstElement;
				desc.Buffer.NumElements = Buffer.NumElements;
				desc.Buffer.Flags = Buffer.Raw ? D3D12_BUFFER_SRV_FLAG_RAW : D3D12_BUFFER_SRV_FLAG_NONE;
			},
			[&](const Views::ShaderResource::Texture1D& Texture1D) {
				desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
				desc.Texture1D.MostDetailedMip = Texture1D.MostDetailedMip;
				desc.Texture1D.MipLevels = Texture1D.MipLevels;
				desc.Texture1D.ResourceMinLODClamp = Texture1D.ResourceMinLODClamp;
			},
			[&](const Views::ShaderResource::Texture1DArray& Texture1DArray) {
				desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
				desc.Texture1DArray.MostDetailedMip = Texture1DArray.MostDetailedMip;
				desc.Texture1DArray.MipLevels = Texture1DArray.MipLevels;
				desc.Texture1DArray.ResourceMinLODClamp = Texture1DArray.ResourceMinLODClamp;
				desc.Texture1DArray.FirstArraySlice = Texture1DArray.FirstArraySlice;
				desc.Texture1DArray.ArraySize = Texture1DArray.ArraySize;
			},
			[&](const Views::ShaderResource::Texture2D& Texture2D) {
				desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
				desc.Texture2D.MostDetailedMip = Texture2D.MostDetailedMip;
				desc.Texture2D.MipLevels = Texture2D.MipLevels;
				desc.Texture2D.ResourceMinLODClamp = Texture2D.ResourceMinLODClamp;
				desc.Texture2D.PlaneSlice = Texture2D.PlaneSlice;

				assert(desc.Texture2D.MipLevels > 0);
				assert(desc.Texture2D.MostDetailedMip + desc.Texture2D.MipLevels <= view.Resource->get_desc().MipLevels);
			},
			[&](const Views::ShaderResource::Texture2DArray& Texture2DArray) {
				desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
				desc.Texture2DArray.MostDetailedMip = Texture2DArray.MostDetailedMip;
				desc.Texture2DArray.MipLevels = Texture2DArray.MipLevels;
				desc.Texture2DArray.ResourceMinLODClamp = Texture2DArray.ResourceMinLODClamp;
				desc.Texture2DArray.PlaneSlice = Texture2DArray.PlaneSlice;
				desc.Texture2DArray.FirstArraySlice = Texture2DArray.FirstArraySlice;
				desc.Texture2DArray.ArraySize = Texture2DArray.ArraySize;

				assert(desc.Texture2DArray.MipLevels > 0);
				assert(desc.Texture2DArray.MostDetailedMip + desc.Texture2DArray.MipLevels <= view.Resource->get_desc().MipLevels);
			},
			[&](const Views::ShaderResource::Texture3D& Texture3D) {
				desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
				desc.Texture3D.MostDetailedMip = Texture3D.MostDetailedMip;
				desc.Texture3D.MipLevels = Texture3D.MipLevels;
				desc.Texture3D.ResourceMinLODClamp = Texture3D.ResourceMinLODClamp;
			},
			[&](const Views::ShaderResource::Texture2DMS& Texture2DMS) {
				desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
			},
			[&](const Views::ShaderResource::Texture2DMSArray& Texture2DMSArray) {
				desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY;
				desc.Texture2DMSArray.FirstArraySlice = Texture2DMSArray.FirstArraySlice;
				desc.Texture2DMSArray.ArraySize = Texture2DMSArray.ArraySize;
			},
			[&](const Views::ShaderResource::Cube& Cube) {
				desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
				desc.TextureCube.MostDetailedMip = Cube.MostDetailedMip;
				desc.TextureCube.MipLevels = Cube.MipLevels;
				desc.TextureCube.ResourceMinLODClamp = Cube.ResourceMinLODClamp;

				assert(desc.TextureCube.MipLevels > 0);
				assert(desc.TextureCube.MostDetailedMip + desc.TextureCube.MipLevels <= view.Resource->get_desc().MipLevels);
			},
			[&](const Views::ShaderResource::CubeArray& CubeArray) {
				desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
				desc.TextureCubeArray.MostDetailedMip = CubeArray.MostDetailedMip;
				desc.TextureCubeArray.MipLevels = CubeArray.MipLevels;
				desc.TextureCubeArray.ResourceMinLODClamp = CubeArray.ResourceMinLODClamp;
				desc.TextureCubeArray.NumCubes = CubeArray.NumCubes;
				desc.TextureCubeArray.First2DArrayFace = CubeArray.First2DArrayFace;
			},
			[&](const Views::ShaderResource::Raytracing& Raytracing) {
				desc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
				desc.RaytracingAccelerationStructure.Location = Raytracing.Location;
			},
			[&](auto other) {
				assert(false);
			}
		}, view.View);

		auto h = heap->cpu_start;

		auto size = heap->device.get_descriptor_size(heap->desc.HeapType);
		h.Offset(offset, size);
		heap->device.native_device->CreateShaderResourceView(view.Resource ? view.Resource->native_resource.Get() : nullptr, &desc, h);
		}

	}