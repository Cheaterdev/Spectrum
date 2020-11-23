
template<class Slot>
struct CompiledData
{
	//Render::HandleTableLight table_cbv;

	Render::ResourceAddress cb;
	Render::HandleTableLight table_srv;
	Render::HandleTableLight table_uav;
	Render::HandleTableLight table_smp;

	const CompiledData<Slot>& set(Render::SignatureDataSetter& graphics, bool use_transitions = true) const
	{
		if (use_transitions) {
			//	auto timer = Profiler::get().start(L"transitions");

			for (UINT i = 0; i < (UINT)table_srv.get_count(); ++i)
			{
				auto h = table_srv[i];
				if (h.resource_info && h.resource_info->resource_ptr)
					if (h.resource_info->resource_ptr->get_heap_type() == Render::HeapType::DEFAULT)
					{
						graphics.get_base().transition_srv(h.resource_info);
						//	graphics.get_base().transition(h.resource_info->resource_ptr, Render::ResourceState::PIXEL_SHADER_RESOURCE | Render::ResourceState::NON_PIXEL_SHADER_RESOURCE);
					}
					else
					{
						graphics.get_base().use_resource(h.resource_info->resource_ptr);
					}

			}

			for (UINT i = 0; i < (UINT)table_uav.get_count(); ++i)
			{
				auto h = table_uav[i];
				if (h.resource_info && h.resource_info->resource_ptr)
					if (h.resource_info->resource_ptr->get_heap_type() == Render::HeapType::DEFAULT)
					{
						//graphics.get_base().transition(h.resource_info->resource_ptr, Render::ResourceState::UNORDERED_ACCESS);
						graphics.get_base().transition_uav(h.resource_info);
					}
					else
					{
						graphics.get_base().use_resource(h.resource_info->resource_ptr);
					}
			}


			/*for (int i = 0; i < table_cbv.get_count(); ++i)
			{
				auto h = table_cbv[i];
				if (h.resource_ptr && *h.resource_ptr)
					if ((*h.resource_ptr)->get_heap_type() == Render::HeapType::DEFAULT)
						graphics.get_base().transition(*h.resource_ptr, Render::ResourceState::VERTEX_AND_CONSTANT_BUFFER);
			}*/


			if (cb)
			{
				if ((cb.resource)->get_heap_type() == Render::HeapType::DEFAULT)
				{
					graphics.get_base().transition(cb.resource, Render::ResourceState::VERTEX_AND_CONSTANT_BUFFER);
				}
				else
				{
					graphics.get_base().use_resource(cb.resource);
				}

			}
		}


		//auto timer = Profiler::get().start(L"set");
		if constexpr (TableHasSRV<Slot>)  if (table_srv.is_valid()) graphics.set(Slot::SRV_ID, table_srv);
		if constexpr (TableHasSMP<Slot>)  if (table_smp.is_valid()) graphics.set(Slot::SMP_ID, table_smp);
		if constexpr (TableHasUAV<Slot>)  if (table_uav.is_valid()) graphics.set(Slot::UAV_ID, table_uav);
		//	if (table_cbv.get_count()> 0 ) graphics.set(Slot::CB_ID, table_cbv);



		if (cb)
			graphics.set_const_buffer(Slot::CB_ID, cb.address);

		return *this;
	}

};

template<class Table, class _Slot = Table::Slot>
struct DataHolder : public Table
{
	using Table::Table;
	using Slot = _Slot;
	using Compiled = CompiledData<Slot>;

	template<class Context, class SRV>
	void place_srv(CompiledData<Slot>& compiled, Context& context, SRV& srv) const
	{
		compiled.table_srv = context.srv.place(sizeof(srv) / sizeof(Render::Handle));
		auto ptr = reinterpret_cast<Render::Handle*>(&srv);
		for (int i = 0; i < compiled.table_srv.get_count(); i++)
		{
			Render::Handle* handle = ptr + i;
			compiled.table_srv[i].place(*handle);
		}
	}

	template<class Context, class SRV>
	void place_srv(CompiledData<Slot>& compiled, Context& context, SRV& srv, Render::Bindless& bindless) const
	{
		compiled.table_srv = context.srv.place(sizeof(srv) / sizeof(Render::Handle));
		auto ptr = reinterpret_cast<Render::Handle*>(&srv);
		for (int i = 0; i < compiled.table_srv.get_count(); i++)
		{
			Render::Handle* handle = ptr + i;
			//assert(!(*handle[0].resource_ptr)->debug);
			compiled.table_srv[i].place(*handle);
		}
	}

	template<class Context, class UAV>
	void place_uav(CompiledData<Slot>& compiled, Context& context, UAV& uav) const
	{
		//auto timer = Profiler::get().start(L"UAV");

		compiled.table_uav = context.srv.place(sizeof(uav) / sizeof(Render::Handle));

		auto ptr = reinterpret_cast<Render::Handle*>(&uav);
		for (int i = 0; i < sizeof(uav) / sizeof(Render::Handle); i++)
		{
			Render::Handle* handle = ptr + i;
			if (ptr[i].cpu.ptr != 0)
				compiled.table_uav[i].place(*handle);
			else
				compiled.table_uav[i].resource_info->resource_ptr = nullptr;

		}
	}

	template<class Context, class SMP>
	void place_smp(CompiledData<Slot>& compiled, Context& context, SMP& smp) const
	{
		//auto timer = Profiler::get().start(L"SMP");

		compiled.table_smp = context.smp.place(sizeof(smp) / sizeof(Render::Handle));
		auto ptr = reinterpret_cast<Render::Handle*>(&smp);
		for (int i = 0; i < compiled.table_smp.get_count(); i++)
		{
			Render::Handle* handle = ptr + i;
			compiled.table_smp[i].place(*handle);
		}
	}


	template<class Context>
	CompiledData<Slot> compile(Context& context) const
	{

		// auto timer = Profiler::get().start(L"compile");
		CompiledData<Slot> compiled;


		if constexpr (HasSRV<Table> || HasBindless<Table>)
		{
			//		auto timer = Profiler::get().start(L"SRV");

			UINT srv_count = 0;
			if constexpr (HasSRV<Table>) srv_count += sizeof(Table::srv) / sizeof(Render::Handle);
			if constexpr (HasBindless<Table>) srv_count += (UINT)Table::bindless.size();

			if (srv_count > 0) {
				compiled.table_srv = context.srv.place(srv_count);
				int _offset = 0;
				if constexpr (HasSRV<Table>) {

					Render::Handle* ptr = reinterpret_cast<Render::Handle*>(std::addressof(Table::srv));
					for (int i = 0; i < sizeof(Table::srv) / sizeof(Render::Handle); i++)
					{
						if (ptr[i].cpu.ptr != 0)
							compiled.table_srv[_offset++].place(ptr[i]);
						else
						{
							compiled.table_srv[_offset++].resource_info->resource_ptr = nullptr;
						}

					}
				}


				if constexpr (HasBindless<Table>) {
					//	auto timer = Profiler::get().start(L"Bindless");

					for (int j = 0; j < Table::bindless.size(); j++)
					{
						if (Table::bindless[j].cpu.ptr != 0)
							compiled.table_srv[_offset++].place(Table::bindless[j]);
						else
						{
							compiled.table_srv[_offset++].resource_info->resource_ptr = nullptr;
						}
					}
				}
			}
		}

		if constexpr (HasUAV<Table>)
			place_uav(compiled, context, Table::uav);

		if constexpr (HasSMP<Table>)
			place_smp(compiled, context, Table::smp);

		if constexpr (HasCB<Table>)
		{
			//compiled.table_cbv = context.get_base().srv_descriptors.place(1);

		 //   auto timer = Profiler::get().start(L"CB");

			if constexpr (HasData<Table>)
				compiled.cb = context.place_raw(Table::data, Table::cb).get_resource_address();
			else
				compiled.cb = context.place_raw(Table::cb).get_resource_address();
		}


		return compiled;
	}


	void set(Render::SignatureDataSetter& context, bool use_transitions = true) const
	{
		compile(context.get_base()).set(context, use_transitions);
	}


	static D3D12_INDIRECT_ARGUMENT_DESC create_indirect()
	{
		static_assert(HasCB<Table>);
		static_assert(!HasSRV<Table>);
		static_assert(!HasUAV<Table>);
		static_assert(!HasSMP<Table>);

		D3D12_INDIRECT_ARGUMENT_DESC desc;
		desc.Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT_BUFFER_VIEW;
		desc.ConstantBufferView.RootParameterIndex = Slot::CB_ID;

		return desc;
	}


};
