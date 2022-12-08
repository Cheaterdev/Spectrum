export module HAL:Slots;

import :ResourceViews;
import :Concepts;
import :DescriptorHeap;
import :Enums;
import :Buffer;
import :SIG;

export {


	template<typename T> concept Compilable =
		requires (T t) {
		T::compile;
	};

	template<typename T> concept HandleType = std::is_base_of_v<HAL::Handle, T>;

	template<class Context>
	class Compiler
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
		Compiler() :s(std::stringstream::out | std::stringstream::binary)
		{

		}

		template<HandleType T>
		void compile(const T& handle)
		{
			uint offset = 0;
			if (handle.is_valid())
			{
				if(handle.get_storage()->can_free())
				descriptors.insert(handle.get_storage());
				resources.push_back(handle.get_resource_info());
				offset = handle.get_offset();
			}

			s.write(reinterpret_cast<const char*>(&offset), sizeof(offset));
		}

		template<HandleType T, uint N>
		void compile(const T(&handles)[N])
		{

			pad();

			
			for (uint i = 0; i < N; i++)
			{
				uint offset = 0;
				auto& handle = handles[i];
				if (handle.is_valid())
				{
					if(handle.get_storage()->can_free())
				descriptors.insert(handle.get_storage());
				resources.push_back(handle.get_resource_info());
					offset = handle.get_offset();
				}

				s.write(reinterpret_cast<const char*>(&offset), sizeof(offset));
				pad();
			}



		}

		template<Compilable T>
		void compile(const T& t) //equires (std::is_base_of_v<T, Handle>)
		{
			pad();
			t.compile(*this);
		}

		void compile(const DynamicData& t)
		{
			s.write(reinterpret_cast<const char*>(t.data()), t.size());
		}

		template<HandleType T>
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
					if(handle.get_storage()->can_free())
				descriptors.insert(handle.get_storage());
					offsets.emplace_back(handle.get_offset());
						resources.push_back(handle.get_resource_info());
					}
					else
					{
						offsets.emplace_back(0);
					}
				}


				auto info = context->place_raw(offsets);
				auto srv = info.resource->create_view<HAL::StructuredBufferView<UINT>>(*context, HAL::StructuredBufferViewDesc{ (uint)info.resource_offset, (uint)info.size,false }).structuredBuffer;
				if(srv.get_storage()->can_free())
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
		void compile(const T& t) //equires (std::is_base_of_v<T, Handle>)
		{
			auto start = s.str().length() % sizeof(uint4);
			auto end = start + sizeof(T);

			if (start != 0 && end > sizeof(uint4))
			{
				pad();
			}
			s.write(reinterpret_cast<const char*>(&t), sizeof(T));
			//	t.compile(*this);
		}
	};
	template<class _SlotTable, SlotID _ID, class _Table, class _Slot>
	struct CompiledData
	{
		std::vector<HAL::ResourceInfo*> resources;
			std::set<std::shared_ptr<HAL::DescriptorHeapStorage>> descriptors;

		static constexpr SlotID ID = _ID;
		using Table = _Table;
		using Slot = _Slot;
		using SlotTable = _SlotTable;

		HAL::ResourceAddress offsets_cb;
		HLSL::ConstBuffer<Table> const_buffer;

		const CompiledData<SlotTable, ID, Table, Slot>& set(HAL::SignatureDataSetter& graphics) const
		{
			graphics.set_slot(*this);
			return *this;
		}

		const void set_tables(HAL::SignatureDataSetter& graphics) const
		{
			for (auto resource_info : resources)
				graphics.get_base().transition(resource_info);

			for (auto d : descriptors)
				graphics.get_base().track_object(*d);

			graphics.set_cb(Slot::ID, const_buffer);
		}
		//operator HAL::ResourceAddress() const
		//{
		//	return offsets_cb;
		//}

		HLSL::ConstBuffer<Table> compiled()
		{
			assert(const_buffer.is_valid());
			return const_buffer;
		}
	};

	template<class _SlotTable, SlotID ID, class Table, class _Slot = Table::Slot>
	struct DataHolder : public Table
	{
		using Table::Table;
		using Slot = _Slot;
		using SlotTable = _SlotTable;

		using Compiled = CompiledData<SlotTable, ID, Table, Slot>;


		template<class Context>
		Compiled compile(Context& context) const
		{

			Compiler<Context> compiler;
			compiler.context = &context;
			Table::compile(compiler);
			

			Compiled compiled;

			auto str = compiler.s.str();

			auto ptr = reinterpret_cast<const std::byte*>(str.data());
			std::vector<std::byte> data;
			data.assign(ptr, ptr + str.size());

			auto info = context.place_data(Math::roundUp(data.size(),256),256);
			context.write(info, data);
	
			compiled.offsets_cb =  info;
			compiled.resources = compiler.resources;
			compiled.descriptors = compiler.descriptors;


				auto hlsl = context.alloc_descriptor(1, HAL::DescriptorHeapIndex{ HAL::DescriptorHeapType::CBV_SRV_UAV, HAL::DescriptorHeapFlags::ShaderVisible });

				compiled.const_buffer = HLSL::ConstBuffer<Table>(hlsl[0]);
			compiled.const_buffer.create(compiled.offsets_cb.resource, info.resource_offset,info.size);


				if(compiled.const_buffer.get_storage()->can_free())
						compiled.descriptors.insert(compiled.const_buffer.get_storage());


			return compiled;
		}


		void set(HAL::SignatureDataSetter& context) const
		{
			compile(context.get_base()).set(context);
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

}