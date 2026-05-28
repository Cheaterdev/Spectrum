export module HAL:API.DescriptorHeap;

import :API.Device;
import :Utils;
import :Types;
import :Descriptors;
import :API.Resource;

import Core;

export namespace HAL
{
	namespace API
	{
		class DescriptorHeap;
	}
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

		Descriptor(DescriptorHeap& heap, uint offset);

		friend class API::DescriptorHeap;
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
	namespace API
	{
		
	
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
		DescriptorHeap(Device& device, const DescriptorHeapDesc& desc);

		Descriptor operator[](uint i);

		auto get_dx() const
		{
			if (m_gpu_heap)
				return m_gpu_heap.Get();
			return m_cpu_heap.Get();
		}
	};
	}
}