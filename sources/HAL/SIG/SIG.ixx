export module HAL:SIG;


import :Concepts;

import :DescriptorHeap;
import Core;
import :HLSL;

import :Enums;

export
{


	namespace Graphics
	{
		using Bindless = std::vector<HAL::Handle>;


	}


	using GPUAddress = uint64_t;

	template<class T>
	class Pointer
	{
		uint ptr;

	public:

		void operator=(const HLSL::ConstantBuffer<T>& t)
		{
			ptr = t.get_offset();
		}

			SERIALIZE()
		{
			 ar& NVP(ptr);
		}
	};
	class DrawIndexedArguments
	{
	public:

		UINT IndexCountPerInstance;
		UINT InstanceCount;
		UINT StartIndexLocation;
		INT  BaseVertexLocation;
		UINT StartInstanceLocation;

		using Compiled = DrawIndexedArguments;
		static const IndirectCommands CommandID = IndirectCommands::DrawIndexedArguments;

		template<class Processor> static void for_each(Processor& processor) {
			processor.template process<DrawIndexedArguments>();
		}
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

	class DispatchMeshArguments
	{
	public:
		UINT ThreadGroupCountX;
		UINT ThreadGroupCountY;
		UINT ThreadGroupCountZ;

		using Compiled = DispatchMeshArguments;
		static const IndirectCommands CommandID = IndirectCommands::DispatchMeshArguments;


		template<class Processor> static void for_each(Processor& processor) {
			processor.template process<DispatchMeshArguments>();
		}


	private:
		SERIALIZE()
		{
			ar& NVP(ThreadGroupCountX);
			ar& NVP(ThreadGroupCountY);
			ar& NVP(ThreadGroupCountZ);
		}

	};


	class DispatchArguments
	{
	public:
		UINT ThreadGroupCountX;
		UINT ThreadGroupCountY;
		UINT ThreadGroupCountZ;

		using Compiled = DispatchArguments;
		static const IndirectCommands CommandID = IndirectCommands::DispatchArguments;

		template<class Processor> static void for_each(Processor& processor) {
			processor.template process<DispatchArguments>();
		}

	private:
		SERIALIZE()
		{
			ar& NVP(ThreadGroupCountX);
			ar& NVP(ThreadGroupCountY);
			ar& NVP(ThreadGroupCountZ);
		}
	};

	using DefaultCB = HAL::Buffer*;// std::vector<std::byte>;
	using DynamicData = std::vector<std::byte>;// std::vector<std::byte>;




	struct EntryPointsCompiled
	{

		struct Record
		{
			HAL::GPUAddressPtr address;
			uint size;

		};
		//std::vector<Record>

	};
	
}