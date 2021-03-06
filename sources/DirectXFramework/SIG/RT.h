

struct CompiledRT
{
	Render::HandleTableLight table_rtv;
	Render::HandleTableLight table_dsv;



	const CompiledRT& set(Render::GraphicsContext& graphics, bool use_transitions = true) const
	{
		graphics.set_rtv(table_rtv, table_dsv[0]);

		return *this;

	}
};


template<class Table>
class RTHolder :public Table
{



	template<class Context, class RTV>
	void place_rtv(CompiledRT& compiled, Context& context, RTV& rtv) const
	{
		compiled.table_rtv = context.get_cpu_heap(Render::DescriptorHeapType::RTV).place(sizeof(rtv) / sizeof(Render::Handle));
		auto ptr = reinterpret_cast<Render::Handle*>(&rtv);
		for (UINT i = 0; i < (UINT)compiled.table_rtv.get_count(); i++)
		{
			Render::Handle* handle = ptr + i;
			compiled.table_rtv[i].place(*handle);

			//Render::Device::get().create_rtv(compiled.table_rtv[i], handle->resource_info->resource_ptr, handle->resource_info->rtv);

		}
	}


	template<class Context, class DSV>
	void place_dsv(CompiledRT& compiled, Context& context, DSV& dsv) const
	{
		compiled.table_dsv = context.get_cpu_heap(Render::DescriptorHeapType::DSV).place(sizeof(dsv) / sizeof(Render::Handle));
		auto ptr = reinterpret_cast<Render::Handle*>(&dsv);
		for (UINT i = 0; i < (UINT)compiled.table_dsv.get_count(); i++)
		{
			Render::Handle* handle = ptr + i;
			compiled.table_dsv[i].place(*handle);
		}
	}



public:
	using Table::Table;
	using Compiled = CompiledRT;

	template<class Context>
	CompiledRT compile(Context& context) const
	{

		// PROFILE(L"compile");
		CompiledRT compiled;



		if constexpr (HasRTV<Table>)
			place_rtv(compiled, context, Table::rtv);

		if constexpr (HasDSV<Table>)
			place_dsv(compiled, context, Table::dsv);


		return compiled;
	}


	void set(Render::GraphicsContext& context, bool use_transitions = true) const
	{
		compile(context.get_base()).set(context, use_transitions);
	}

};

