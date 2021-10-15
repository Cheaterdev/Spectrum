#include "pch_dx.h"
import Resource;
#include "DX12/CommandList.h"
import Concepts;
import Descriptors;
template<class _SlotTable, SlotID _ID, class _Table, class _Slot>
struct CompiledData
{

	static constexpr SlotID ID = _ID;
	using Table = _Table;
	using Slot = _Slot;
	using SlotTable = _SlotTable;
	Render::ResourceAddress cb;
	Render::HandleTableLight table_srv;
	Render::HandleTableLight table_uav;
	Render::HandleTableLight table_smp;


	std::vector<UINT> offsets_srv;
	std::vector<UINT> offsets_uav;
	std::vector<UINT> offsets_smp;

	Render::ResourceAddress offsets_cb;
	UINT offset_cb;

	const CompiledData<SlotTable, ID, Table, Slot>& set(Render::SignatureDataSetter& graphics) const
	{
		graphics.set_slot(*this);
		return *this;
	}

	const void set_tables(Render::SignatureDataSetter& graphics) const
	{
		if constexpr (HasSRV<Table>) graphics.set_table<Render::HandleType::SRV>(Slot::SRV_ID, table_srv);
		if constexpr (HasSMP<Table>) graphics.set_table<Render::HandleType::SMP>(Slot::SMP_ID, table_smp);
		if constexpr (HasUAV<Table>) graphics.set_table<Render::HandleType::UAV>(Slot::UAV_ID, table_uav);
		if constexpr (HasCB<Table>) graphics.set_cb(Slot::CB_ID, cb);

		if constexpr (HasSRV<Table> || HasSMP<Table> || HasUAV<Table>)
		graphics.set_cb(Slot::CB_ID+1, offsets_cb);
	}

};

template<class _SlotTable, SlotID ID, class Table, class _Slot = Table::Slot>
struct DataHolder : public Table
{
	using Table::Table;
	using Slot = _Slot;
	using SlotTable = _SlotTable;

	using Compiled = CompiledData<SlotTable, ID, Table, Slot>;

	/*
	template<class Context, class SRV>
	void place_srv(Compiled& compiled, Context& context, SRV& srv) const
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
	void place_srv(Compiled& compiled, Context& context, SRV& srv, Render::Bindless& bindless) const
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
	*/
	
	template<class Context, class UAV>
	void place_uav(Compiled& compiled, Context& context, UAV& uav) const
	{
		auto count = static_cast<UINT>(sizeof(uav) / sizeof(Render::Handle));

		compiled.table_uav = context.get_gpu_heap(Render::DescriptorHeapType::CBV_SRV_UAV).place(count);
		compiled.offsets_uav.resize(count);


		auto ptr = reinterpret_cast<Render::Handle*>(&uav);
		for (int i = 0; i < sizeof(uav) / sizeof(Render::Handle); i++)
		{
			Render::Handle* handle = ptr + i;
			if (ptr[i].cpu.ptr != 0)
			{
				compiled.table_uav[i].place(*handle);
				compiled.offsets_uav[i] = compiled.table_uav.offset + i;// handle->offset;
			}
			else
			{
				compiled.table_uav[i].resource_info->resource_ptr = nullptr;
				compiled.offsets_uav[i] = UINT_MAX;
			}


		}
	}

	template<class Context, class SMP>
	void place_smp(Compiled& compiled, Context& context, SMP& smp) const
	{
		auto count = sizeof(smp) / sizeof(Render::Handle);

		compiled.table_smp = context.get_gpu_heap(Render::DescriptorHeapType::SAMPLER).place(count);
		compiled.offsets_smp.resize(count);

		auto ptr = reinterpret_cast<Render::Handle*>(&smp);
		for (int i = 0; i < compiled.table_smp.get_count(); i++)
		{
			Render::Handle* handle = ptr + i;
			compiled.table_smp[i].place(*handle);
		}
	}


	template<class Context>
	Compiled compile(Context& context) const
	{

		Compiled compiled;


		if constexpr (HasSRV<Table> || HasBindless<Table>)
		{

			UINT srv_count = 0;
			if constexpr (HasSRV<Table>) srv_count += sizeof(Table::srv) / sizeof(Render::Handle);
			if constexpr (HasBindless<Table>) srv_count += (UINT)Table::bindless.size();

			if (srv_count > 0) {
				compiled.table_srv = context.get_gpu_heap(Render::DescriptorHeapType::CBV_SRV_UAV).place(srv_count);
				compiled.offsets_srv.resize(srv_count);


				int _offset = 0;
				if constexpr (HasSRV<Table>) {

					Render::Handle* ptr = reinterpret_cast<Render::Handle*>(std::addressof(Table::srv));
					for (int i = 0; i < sizeof(Table::srv) / sizeof(Render::Handle); i++)
					{
						if (ptr[i].cpu.ptr != 0)

						{
							compiled.offsets_srv[_offset] = compiled.table_srv.offset + _offset;// handle->offset;
							compiled.table_srv[_offset++].place(ptr[i]);

						}
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
						{
							compiled.offsets_srv[_offset] = compiled.table_srv.offset + _offset;// handle->offset;
							compiled.table_srv[_offset++].place(Table::bindless[j]);
						}
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

		if constexpr (HasSRV<Table> || HasSMP<Table> || HasUAV<Table>)
		compiled.offsets_cb = context.place_raw(compiled.offsets_srv, compiled.offsets_uav, compiled.offsets_smp).get_resource_address();

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
