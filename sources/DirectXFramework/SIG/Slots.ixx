module;

export module Graphics:Slots;

import :Resource;
import :CommandList;
import :Concepts;
import :Descriptors;
import :Enums;
import :Buffer;
import :SIG;
import :ResourceViews;
export {


	template<typename T> concept Compilable =
		requires (T t) {
		T::compile;
	};

	template<typename T> concept HandleType = std::is_base_of_v<Graphics::Handle, T>;

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
		std::vector<Graphics::ResourceInfo*> resources;
		std::stringstream s;
		Compiler() :s(std::stringstream::out | std::stringstream::binary)
		{

		}

		template<HandleType T>
		void compile(const T& handle)
		{
			if (handle.is_valid())
			{
				if (handle.get_resource_info())
				{
					resources.push_back(handle.get_resource_info());
				}
			}

			uint offset = handle.offset;
			s.write(reinterpret_cast<const char*>(&offset), sizeof(offset));
		}

		template<HandleType T, uint N>
		void compile(const T(&handles)[N])
		{

			pad();


			for (uint i = 0; i < N; i++)
			{
				auto& handle = handles[i];
				if (handle.is_valid())
				{
					if (handle.get_resource_info())
					{
						resources.push_back(handle.get_resource_info());
					}
				}

				uint offset = handle.offset;
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
					const Graphics::Handle& handle = t[i];

					if (handle.is_valid())
					{
						offsets.emplace_back(handle.offset);

						if (handle.get_resource_info())
						{
							resources.push_back(handle.get_resource_info());
						}
					}
					else
					{
						offsets.emplace_back(0);
					}
				}


				auto info = context->place_raw(offsets);
				auto srv = info.resource->create_view<StructuredBufferView<UINT>>(*context, Graphics::BufferType::NONE, (UINT)info.offset, (UINT)info.size).structuredBuffer;

				offset = srv.offset;

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
		std::vector<Graphics::ResourceInfo*> resources;

		static constexpr SlotID ID = _ID;
		using Table = _Table;
		using Slot = _Slot;
		using SlotTable = _SlotTable;

		Graphics::ResourceAddress offsets_cb;
		UINT offset_cb;

		const CompiledData<SlotTable, ID, Table, Slot>& set(Graphics::SignatureDataSetter& graphics) const
		{
			graphics.set_slot(*this);
			return *this;
		}

		const void set_tables(Graphics::SignatureDataSetter& graphics) const
		{
			for (auto resource_info : resources)
				graphics.get_base().transition(resource_info);
			graphics.set_cb(Slot::ID, offsets_cb);
		}
		operator Graphics::ResourceAddress() const
		{
			return offsets_cb;
		}

		Graphics::ResourceAddress compiled()
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
			data.assign(ptr, ptr + str.size());
			compiled.offsets_cb = context.place_raw(data).get_resource_address();
			compiled.resources = compiler.resources;
			return compiled;
		}


		void set(Graphics::SignatureDataSetter& context) const
		{
			compile(context.get_base()).set(context);
		}

		static D3D12_INDIRECT_ARGUMENT_DESC create_indirect()
		{
			D3D12_INDIRECT_ARGUMENT_DESC desc;
			desc.Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT_BUFFER_VIEW;
			desc.ConstantBufferView.RootParameterIndex = Slot::ID;

			return desc;
		}
	};

}