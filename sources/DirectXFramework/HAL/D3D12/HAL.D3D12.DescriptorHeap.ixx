export module HAL:API.DescriptorHeap;

import :API.Device;
import :Utils;
import :Types;
import :Descriptors;
import :API.Resource;

import Utils;
import Debug;

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
		DescriptorHeap& heap;
		const uint offset;

		Descriptor(DescriptorHeap& heap, uint offset) : heap(heap), offset(offset)
		{
		}

		friend class DescriptorHeap;
	public:
		void operator=(const Descriptor& r);

		void place(const Views::ShaderResource& view);
		void place(const Views::UnorderedAccess& view);
		void place(const Views::RenderTarget& view);
		void place(const Views::ConstantBuffer& view);
		void place(const Views::DepthStencil& view);

		D3D12_CPU_DESCRIPTOR_HANDLE  get_cpu();
		D3D12_GPU_DESCRIPTOR_HANDLE  get_gpu();

	};

	class DescriptorHeap
	{
	public:// TODO
		D3D::DescriptorHeap m_cpu_heap;
		D3D::DescriptorHeap m_gpu_heap;

		const DescriptorHeapDesc desc;
		const Device& device;

		CD3DX12_CPU_DESCRIPTOR_HANDLE  cpu_start;
		CD3DX12_CPU_DESCRIPTOR_HANDLE  gpu_cpu_start;


		CD3DX12_GPU_DESCRIPTOR_HANDLE  gpu_start;

		uint handle_size;
		friend class Descriptor;
	public:
		DescriptorHeap(Device& device, const DescriptorHeapDesc& desc) :device(device), desc(desc)
		{
			D3D12_DESCRIPTOR_HEAP_DESC native_desc = {};
			native_desc.NumDescriptors = desc.Count;
			native_desc.Type = to_native(desc.HeapType);
			native_desc.Flags = to_native(DescriptorHeapFlags::NONE);
			native_desc.NodeMask = 1;

			TEST(device, device.native_device->CreateDescriptorHeap(&native_desc, IID_PPV_ARGS(&m_cpu_heap)));
			cpu_start = m_cpu_heap->GetCPUDescriptorHandleForHeapStart();

			if (check(desc.Flags & DescriptorHeapFlags::SHADER_VISIBLE))
			{
				native_desc.Flags = to_native(DescriptorHeapFlags::SHADER_VISIBLE);
				TEST(device, device.native_device->CreateDescriptorHeap(&native_desc, IID_PPV_ARGS(&m_gpu_heap)));
				gpu_cpu_start = m_gpu_heap->GetCPUDescriptorHandleForHeapStart();

				gpu_start = m_gpu_heap->GetGPUDescriptorHandleForHeapStart();
			}

			handle_size = device.get_descriptor_size(desc.HeapType);
		}

		Descriptor operator[](uint i)
		{
			return Descriptor{ *this, i };
		}
	};

}