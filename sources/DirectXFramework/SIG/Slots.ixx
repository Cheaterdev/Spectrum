module;
#include "pch_dx.h"
export module Slots;

import Resource;
import CommandList;
import Concepts;
import Descriptors;
import Enums;
import Buffer;
import SIG;
import d3d12_types;

export {


	template<typename T> concept Compilable =
		requires (T t) {
		T::compile;
	};

	template<typename T> concept HandleType = std::is_base_of_v<Render::Handle, T>;

	template<class Context>
	class Compiler
	{
		
	public:
		Context* context;
		std::vector<DX12::ResourceInfo*> resources;
		std::stringstream s;
		Compiler():s(std::stringstream::out | std::stringstream::binary)
		{

		}
		template<HandleType T>
		void compile(const T& handle)
		{
			auto table = context->get_gpu_heap(DX12::DescriptorHeapType::CBV_SRV_UAV).place(1);

			if (handle.is_valid())
			{
				table[0].place(handle);

				if (handle.resource_info)
				{
					resources.push_back(handle.resource_info);
				}
			}

			uint offset = table.offset;
			s.write(reinterpret_cast<const char*>(&offset), sizeof(offset));
		}

		template<Compilable T>
		void compile(const T& t) //equires (std::is_base_of_v<T, Handle>)
		{
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
				auto table = context->get_gpu_heap(DX12::DescriptorHeapType::CBV_SRV_UAV).place(t.size());
				for (uint i = 0; i < t.size(); i++)
				{
					const Render::Handle& handle = t[i];
					if (handle.is_valid())
					{
						table[i].place(handle);

						if (handle.resource_info)
						{
							resources.push_back(handle.resource_info);
						}
					}
				}

				offset = table.offset;
			}
		//	std::vector<uint> offsets;

		//	for (auto& e : t)
		//	{
		//		offsets.emplace_back(e.offset);

		//		if (e.resource_info)
		//		{
		//			resources.push_back(e.resource_info);
		//		}
		//	}
		//	auto info = uploader->place_raw(offsets);

	
			s.write(reinterpret_cast<const char*>(&offset), sizeof(offset));
	
		}


	


		template<class T>
		void compile(const T& t) //equires (std::is_base_of_v<T, Handle>)
		{
			auto start = s.str().length() % sizeof(uint4);
			auto end = start + sizeof(T);

			if (start != 0 && end > sizeof(uint4))
			{
				uint add = sizeof(uint4) - start;

				for (int i = 0; i < add; i++)
				{
					char zero = 0;
					s.write(reinterpret_cast<const char*>(&zero), sizeof(zero));
				}
			}
			s.write(reinterpret_cast<const char*>(&t), sizeof(T));
		//	t.compile(*this);
		}
	};
	template<class _SlotTable, SlotID _ID, class _Table, class _Slot>
	struct CompiledData
	{
		std::vector<DX12::ResourceInfo*> resources;

		static constexpr SlotID ID = _ID;
		using Table = _Table;
		using Slot = _Slot;
		using SlotTable = _SlotTable;

		Render::ResourceAddress offsets_cb;
		UINT offset_cb;

		const CompiledData<SlotTable, ID, Table, Slot>& set(Render::SignatureDataSetter& graphics) const
		{
			graphics.set_slot(*this);
			return *this;
		}

		const void set_tables(Render::SignatureDataSetter& graphics) const
		{
			for(auto resource_info :resources)
			graphics.get_base().transition(resource_info);
			graphics.set_cb(Slot::ID, offsets_cb);
		}
		operator Render::ResourceAddress() const
		{
			return offsets_cb;
		}

		Render::ResourceAddress compiled()
		{
			return offsets_cb;
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
			data.assign(ptr,ptr+ str.size());
			compiled.offsets_cb = context.place_raw(data).get_resource_address();
			compiled.resources = compiler.resources;
			return compiled;
		}


		void set(Render::SignatureDataSetter& context) const
		{
			compile(context.get_base()).set(context);
		}


		static D3D12_INDIRECT_ARGUMENT_DESC create_indirect()
		{
			static_assert(HasCB<Table>);
			static_assert(!HasSRV<Table>);
			static_assert(!HasUAV<Table>);
			static_assert(!HasSMP<Table>);

			D3D12_INDIRECT_ARGUMENT_DESC desc;
			desc.Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT_BUFFER_VIEW;
			desc.ConstantBufferView.RootParameterIndex = Slot::ID;

			return desc;
		}


	};

}