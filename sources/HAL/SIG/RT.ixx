
export module HAL:RT;

import :Concepts;
import :DescriptorHeap;

import :CommandList;

import :Types;

template<class Context>
class RT_Compiler
{

public:
	Context* context;
	std::vector<HAL::ResourceInfo*> resources;
	std::set<std::shared_ptr<HAL::DescriptorHeapStorage>> descriptors;


	std::vector<const HAL::Handle*> rtvs;
	std::optional<const HAL::Handle*> dsv;

	RT_Compiler()
	{

	}


	template<HAL::HandleClass T>
	void compile(const T& handle)
	{
		assert(handle.is_valid());

		if (handle.get_storage()->can_free())
			descriptors.insert(handle.get_storage());
		resources.push_back(&handle.get_resource_info());


		if constexpr (T::TYPE == HAL::HandleType::RTV)
			rtvs.emplace_back(&handle);
		else if constexpr (T::TYPE == HAL::HandleType::DSV)
			dsv = (&handle);
		else
			assert(false);
	}

	template<HAL::HandleClass T, uint N>
	void compile(const T(&handles)[N])
	{
		for (uint i = 0; i < N; i++)
		{
			compile(handles[i]);
		}
	}

	template<Compilable T>
	void compile(const T& t) //equires (std::is_base_of_v<T, Handle>)
	{
		t.compile(*this);
	}


};
export
{




	template<class Table>
	class RTHolder :public Table
	{

	public:
		using Table::Table;
		using Compiled = HAL::CompiledRT;

		template<class Context>
		Compiled compile(Context& context) const
		{
			Compiled compiled;


			RT_Compiler<Context> compiler;
			compiler.context = &context;
			Table::compile(compiler);


			if (compiler.rtvs.size()) {
				compiled.table_rtv = context.alloc_descriptor<HAL::RTVHandle>(static_cast<uint>(compiler.rtvs.size()), HAL::DescriptorHeapIndex{ HAL::DescriptorHeapType::RTV, HAL::DescriptorHeapFlags::None });
				for (UINT i = 0; i < (UINT)compiled.table_rtv.get_count(); i++)
				{
					compiled.table_rtv[i].place(*compiler.rtvs[i]);
				}
			}


			if (compiler.dsv)
			{
				compiled.table_dsv = context.alloc_descriptor<HAL::DSVHandle>(1u, HAL::DescriptorHeapIndex{ HAL::DescriptorHeapType::DSV, HAL::DescriptorHeapFlags::None });
				compiled.table_dsv[0].place(**compiler.dsv);
			}

			return compiled;
		}


		Compiled set(HAL::GraphicsContext& context, HAL::RTOptions options = HAL::RTOptions::Default, float depth = 1, uint stencil = 0) const
		{
			return compile(context.get_base()).set(context, options, depth, stencil);
		}

	};


}