export module HAL:SIG;
import <HAL.h>;

import :Concepts;

import :DescriptorHeap;
import Core;


export
{


	namespace Graphics
	{
		using Bindless = std::vector<HAL::Handle>;


	}


	using GPUAddress = D3D12_GPU_VIRTUAL_ADDRESS;

	class DrawIndexedArguments : public D3D12_DRAW_INDEXED_ARGUMENTS
	{
	public:
		static D3D12_INDIRECT_ARGUMENT_DESC create_indirect()
		{
			D3D12_INDIRECT_ARGUMENT_DESC desc;
			desc.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;
			return desc;
		}
		using Compiled = D3D12_DRAW_INDEXED_ARGUMENTS;
	private:
		SERIALIZE()
		{
			ar& NVP(IndexCountPerInstance);
			ar& NVP(InstanceCount);
			ar& NVP(StartIndexLocation);
			ar& NVP(BaseVertexLocation);
			ar& NVP(StartInstanceLocation);
		}

	};

	class DispatchMeshArguments : public D3D12_DISPATCH_MESH_ARGUMENTS
	{
	public:
		static D3D12_INDIRECT_ARGUMENT_DESC create_indirect()
		{
			D3D12_INDIRECT_ARGUMENT_DESC desc;
			desc.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH_MESH;
			return desc;
		}
		using Compiled = D3D12_DISPATCH_MESH_ARGUMENTS;
	private:
		SERIALIZE()
		{
			ar& NVP(ThreadGroupCountX);
			ar& NVP(ThreadGroupCountY);
			ar& NVP(ThreadGroupCountZ);
		}

	};


	class DispatchArguments : public D3D12_DISPATCH_ARGUMENTS
	{
	public:
		static D3D12_INDIRECT_ARGUMENT_DESC create_indirect()
		{
			D3D12_INDIRECT_ARGUMENT_DESC desc;
			desc.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH;
			return desc;
		}

		using Compiled = D3D12_DISPATCH_ARGUMENTS;
	};

	using DefaultCB = HAL::Resource*;// std::vector<std::byte>;
	using DynamicData = std::vector<std::byte>;// std::vector<std::byte>;
}