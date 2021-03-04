
template<class Slot>
struct CompiledData
{
	Render::ResourceAddress cb;
	Render::HandleTableLight table_srv;
	Render::HandleTableLight table_uav;
	Render::HandleTableLight table_smp;

	const CompiledData<Slot>& set(Render::SignatureDataSetter& graphics) const
	{
		if constexpr (TableHasSRV<Slot>)  if (table_srv.is_valid()) graphics.set_table<Render::HandleType::SRV>(Slot::SRV_ID, table_srv);
		if constexpr (TableHasSMP<Slot>)  if (table_smp.is_valid()) graphics.set_table<Render::HandleType::SMP>(Slot::SMP_ID, table_smp);
		if constexpr (TableHasUAV<Slot>)  if (table_uav.is_valid()) graphics.set_table<Render::HandleType::UAV>(Slot::UAV_ID, table_uav);

		if (cb)
			graphics.set_cb(Slot::CB_ID, cb);

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
		compiled.table_srv = context.get_gpu_heap(Render::DescriptorHeapType::CBV_SRV_UAV).place(sizeof(srv) / sizeof(Render::Handle));
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
		compiled.table_srv = context.get_gpu_heap(Render::DescriptorHeapType::CBV_SRV_UAV).place(sizeof(srv) / sizeof(Render::Handle));
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
	
		compiled.table_uav = context.get_gpu_heap(Render::DescriptorHeapType::CBV_SRV_UAV).place(sizeof(uav) / sizeof(Render::Handle));

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

		compiled.table_smp = context.get_gpu_heap(Render::DescriptorHeapType::SAMPLER).place(sizeof(smp) / sizeof(Render::Handle));
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

		CompiledData<Slot> compiled;


		if constexpr (HasSRV<Table> || HasBindless<Table>)
		{

			UINT srv_count = 0;
			if constexpr (HasSRV<Table>) srv_count += sizeof(Table::srv) / sizeof(Render::Handle);
			if constexpr (HasBindless<Table>) srv_count += (UINT)Table::bindless.size();

			if (srv_count > 0) {
				compiled.table_srv = context.get_gpu_heap(Render::DescriptorHeapType::CBV_SRV_UAV).place(srv_count);
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


			if constexpr (HasData<Table>)
				compiled.cb = context.place_raw(Table::data, Table::cb).get_resource_address();
			else
				compiled.cb = context.place_raw(Table::cb).get_resource_address();
		}


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
		desc.ConstantBufferView.RootParameterIndex = Slot::CB_ID;

		return desc;
	}


};
