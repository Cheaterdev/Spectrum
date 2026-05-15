export module HAL:Slots;
import <HAL.h>;
import :ResourceViews;
import :Concepts;
import :DescriptorHeap;
import :Enums;
//import :Buffer;
import :SIG;

struct placement_info
{
	uint offset;
	uint size;
	SERIALIZE()
	{

		ar& NVP(offset);
		 	ar& NVP(size);

	}
};
std::optional<SlotID> get_slot(std::string_view slot_name);
template<class Context>
class Slot_Compiler
{
	void pad()
	{
		auto start = s.str().length() % sizeof(uint4);
		if (start != 0)
		{
			size_t add = sizeof(uint4) - start;

			for (size_t i = 0; i < add; i++)
			{
				char zero = 0;
				s.write(reinterpret_cast<const char*>(&zero), sizeof(zero));
			}
		}
	}
public:
	Context* context;
	std::vector<HAL::ResourceInfo*> resources;
	std::set<std::shared_ptr<HAL::DescriptorHeapStorage>> descriptors;

	std::stringstream s;
	Slot_Compiler() :s(std::stringstream::out | std::stringstream::binary)
	{

	}

	template<HAL::HandleClass T>
	void compile(const T& handle)
	{
		uint offset = 0;
		if (handle.is_valid())
		{
			if (handle.get_storage()->can_free())
				descriptors.insert(handle.get_storage());
			resources.push_back(&handle.get_resource_info());
			offset = handle.get_offset();
		}

		s.write(reinterpret_cast<const char*>(&offset), sizeof(offset));
	}

	template<HAL::HandleClass T, uint N>
	void compile(const T(&handles)[N])
	{
		pad();
		for (uint i = 0; i < N; i++)
		{
			compile(handles[i]);
			pad();
		}

	}

	template<SIG_TYPES::Table T>
	void compile(const T& t) //equires (std::is_base_of_v<T, Handle>)
	{
		pad();
		t.compile(*this);
	}

	void compile(const DynamicData& t)
	{
		s.write(reinterpret_cast<const char*>(t.data()), t.size());
	}

	template<HAL::HandleClass T>
	void compile(const std::vector<T>& t)
	{
		uint offset = 0;


		if (!t.empty()) {

			std::vector<uint> offsets;
			for (uint i = 0; i < t.size(); i++)
			{
				const HAL::Handle& handle = t[i];

				if (handle.is_valid())
				{
					if (handle.get_storage()->can_free())
						descriptors.insert(handle.get_storage());
					offsets.emplace_back(handle.get_offset());
					resources.push_back(&handle.get_resource_info());
				}
				else
				{
					offsets.emplace_back(0);
				}
			}


			auto info = context->place_raw(offsets);
			auto srv = info.resource->create_view<HAL::StructuredBufferView<UINT>>(*context, HAL::StructuredBufferViewDesc{ (uint)info.resource_offset, (uint)info.size,HAL::counterType::NONE }).structuredBuffer;
			if (srv.get_storage()->can_free())
				descriptors.insert(srv.get_storage());
			offset = srv.get_offset();

		}
		s.write(reinterpret_cast<const char*>(&offset), sizeof(offset));

	}



	template<class T, uint N>
	void compile(const T(&t)[N]) //equires (std::is_base_of_v<T, Handle>)
	{
		pad();

		for (int i = 0; i < N; i++)
		{
			s.write(reinterpret_cast<const char*>(&t[i]), sizeof(T));
			pad();
		}
	}




	template<class T>
	placement_info compile(const T& t) 
	{
		auto st = s.str().length();
		auto start = s.str().length() % sizeof(uint4);
		auto end = start + sizeof(T);

		if (start != 0 && end > sizeof(uint4))
		{
			pad();
		}
		s.write(reinterpret_cast<const char*>(&t), sizeof(T));

		return {uint(st), sizeof(T)};
		//	t.compile(*this);
	}

	template<>
	placement_info compile(const bool& t)
	{
		return compile(uint(t));
	}
};
export {


	template<class Table, class _Slot>
	struct CompiledData
	{
		static constexpr SIG_TYPE_COMPILED TYPE = SIG_TYPE_COMPILED::Slot;

		static constexpr SlotID ID = Table::ID;
		using Slot = _Slot;

		HLSL::ConstantBuffer<Table> const_buffer;
		std::vector<HAL::ResourceInfo*> resources;
		std::set<std::shared_ptr<HAL::DescriptorHeapStorage>> descriptors;

		const HLSL::ConstantBuffer<Table>& compiled() const
		{
			ASSERT(const_buffer.is_valid());
			return const_buffer;
		}
	};

	template<class _SlotTable, SlotID _ID, class Table, class _Slot = Table::Slot>
	struct DataHolder : public Table
	{
	

		using Table::Table;
		using Slot = _Slot;
		using SlotTable = _SlotTable;
		static constexpr SlotID ID = _ID;

		using Compiled = CompiledData<Table, Slot>;

	template<class Compiler>
		void compile_table(Compiler& compiler) const
	{
				Table::compile(compiler);


	}
		template<class Context>
		Compiled compile(Context& context) const  requires (std::is_base_of_v<GPUEntityStorageInterface,Context>)
		{

			Slot_Compiler<Context> Slot_Compiler;
			Slot_Compiler.context = &context;
			compile_table(Slot_Compiler);


			Compiled compiled;

			auto str = Slot_Compiler.s.str();

			auto ptr = reinterpret_cast<const std::byte*>(str.data());
			std::vector<std::byte> data;
			data.assign(ptr, ptr + str.size());

			auto info = context.place_data(Math::roundUp(data.size(), 256), 256);
			context.write(info, data);

			compiled.resources = std::move(Slot_Compiler.resources);
			compiled.descriptors = std::move(Slot_Compiler.descriptors);


			auto hlsl = context.alloc_descriptor(1, HAL::DescriptorHeapIndex{ HAL::DescriptorHeapType::CBV_SRV_UAV, HAL::DescriptorHeapFlags::ShaderVisible });

			compiled.const_buffer = HLSL::ConstantBuffer<Table>(hlsl[0]);
			compiled.const_buffer.create(info.resource->get_ptr(), info.resource_offset, info.size);


			if (compiled.const_buffer.get_storage()->can_free())
				compiled.descriptors.insert(compiled.const_buffer.get_storage());


			return compiled;
		}


		static D3D12_INDIRECT_ARGUMENT_DESC create_indirect()
		{
			D3D12_INDIRECT_ARGUMENT_DESC desc;
			desc.Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT;
			desc.Constant.RootParameterIndex = Slot::ID;
			desc.Constant.DestOffsetIn32BitValues = 0;
			desc.Constant.Num32BitValuesToSet = 1;

			return desc;
		}
	};

	class UsedSlots
	{

	public:
		std::set<SlotID> slots_usage;
		void merge(resource_file_depender& depender)
		{
			for (auto& d : depender.get_files())
			{
				auto parent = d.file_name.parent_path().filename();

				if (to_lower(parent.wstring()) == (L"autogen"))
				{

					auto name = d.file_name.stem();

					auto slot_id = get_slot(name.string());

					ASSERT(slot_id);
					slots_usage.insert(slot_id.value());

				}
			}
		}
		void merge(SlotID id)
		{
			slots_usage.insert(id);

		}


		void merge(UsedSlots& other)
		{
			slots_usage.insert(other.slots_usage.begin(), other.slots_usage.end());

		}
		void clear()
		{
			slots_usage.clear();
		}
		bool uses(SlotID id)const
		{
			return slots_usage.contains(id);
		}
		bool empty() const
		{
			return slots_usage.empty();
		}
	private:
		SERIALIZE()
		{
			ar& NVP(slots_usage);
		}
	};


	 template<class Context>
	struct EntryPoints
	{

		Context* context;
		
		

			 EntryPoints(Context*context):context(context)
			 {
				 
			 }
		struct Record
		{
			Slot_Compiler<Context> Slot_Compiler;
			uint stride = 0;
			uint count = 0;

		};

		uint num_records = 0;
		std::map<uint, Record>	  records;

					template<class Table>
		void add(uint id, Table& elem)
		{
			auto& rec = records[id];

						uint beg = uint(rec.Slot_Compiler.s.str().size());
			rec.Slot_Compiler.context = context;
			elem.compile_table(rec.Slot_Compiler);

						uint end = uint(rec.Slot_Compiler.s.str().size());
			++rec.count;
			rec.stride = sizeof(Table);
			ASSERT(end - beg == sizeof(Table));

		}


		HAL::ResourceAddress compile() const
		{
								
			if(records.empty())
				return 	HAL::ResourceAddress{};

			auto id_rec_data = context->place_data(sizeof(D3D12_NODE_GPU_INPUT) * records.size(), 256);

			auto rec_cpu = reinterpret_cast<D3D12_NODE_GPU_INPUT*>(id_rec_data.get_cpu_data());

			uint i = 0;
			for (auto& [id, id_records] : records)
			{


				D3D12_NODE_GPU_INPUT& input = rec_cpu[i];
				input.EntrypointIndex = id;
				input.NumRecords = id_records.count;

				auto binary = id_records.Slot_Compiler.s.str();
				auto gpu_binary = context->place_data(binary.size(), 256);

				std::memcpy(gpu_binary.get_cpu_data(), binary.data(), binary.size());
				input.Records.StartAddress = to_native(gpu_binary);
				input.Records.StrideInBytes = id_records.stride;


				++i;
			}


			auto res = context->place_data(sizeof(D3D12_MULTI_NODE_GPU_INPUT), 256);

			D3D12_MULTI_NODE_GPU_INPUT& t = *reinterpret_cast<D3D12_MULTI_NODE_GPU_INPUT*>(res.get_cpu_data());
			t.NumNodeInputs = uint(records.size());

			t.NodeInputs.StartAddress = to_native(id_rec_data);

			t.NodeInputs.StrideInBytes = sizeof(D3D12_NODE_GPU_INPUT);
			//	std::memcpy(info2.get_cpu_data(), &t, sizeof(t));



			//{
			//	D3D12_NODE_GPU_INPUT t;


			//	t.EntrypointIndex = 0;
			//	t.NumRecords = 1;
			//	t.Records.StartAddress = compiled.const_buffer.address;
			//	t.Records.StrideInBytes = sizeof(input);
			//	std::memcpy(info2.get_cpu_data(), &t, sizeof(t));
			//}


			//auto info3 = context.place_data(Math::roundUp(sizeof(D3D12_MULTI_NODE_GPU_INPUT), 256), 256);


			//{
			//	D3D12_MULTI_NODE_GPU_INPUT t;


			//	t.NumNodeInputs = 1;
			//	t.NodeInputs.StartAddress = to_native(info2);

			//	t.NodeInputs.StrideInBytes = sizeof(D3D12_NODE_GPU_INPUT);

			//	std::memcpy(info3.get_cpu_data(), &t, sizeof(t));
			//}
			return res;
		}
	};
								 template<class T>
		   EntryPoints<T> create_entry(T& context)
								 {
			   return   EntryPoints<T>(&context);
								 }


}