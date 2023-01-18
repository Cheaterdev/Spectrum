
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
	enum class RTOptions :uint
	{
		SetHandles = 1,
		ClearDepth = 1<< 1,
		ClearStencil = 1 << 2,
		SetViewport = 1 << 3,
		SetScissors = 1 << 4,

		ClearColor = 1<< 5,
		ClearDepthStencil = ClearDepth | ClearStencil,
		ClearAll = ClearDepthStencil | ClearColor,
		Default = SetHandles | SetScissors | SetViewport,
	};
	struct CompiledRT
	{
		HAL::Handle table_rtv;
		HAL::Handle table_dsv;


		

	std::vector<HAL::Format> get_formats()
	{
		std::vector<HAL::Format> result;

		if (table_rtv)
		{

			for (uint i = 0; i < table_rtv.get_count(); i++)
			{
				auto& view = std::get<HAL::Views::RenderTarget>(table_rtv[i].get_resource_info().view);
				result.emplace_back(view.Format);
			}

		}
		return result;
	}
	HAL::Format get_depth_format()
	{
		if (!table_dsv) return HAL::Format::UNKNOWN;
		auto& view = std::get<HAL::Views::DepthStencil>((table_dsv).get_resource_info().view);

		return view.Format;
	}



		const CompiledRT& set(HAL::GraphicsContext& context, RTOptions options = RTOptions::Default, float depth = 1, uint stencil = 0) const
		{
			if ( check(options & RTOptions::SetHandles))
			context.set_rtv(table_rtv, table_dsv);


			if (check(options & RTOptions::ClearColor))
			{
				for (uint i = 0; i < table_rtv.get_count(); i++)
				{
					context.get_base().clear_rtv(table_rtv[i],float4(1,0,0,1));
				}
			}

			if (table_dsv && check(options & RTOptions::ClearDepth))
				context.get_base().clear_depth(table_dsv[0], depth);

			if (table_dsv && check(options & RTOptions::ClearStencil))
				context.get_base().clear_stencil(table_dsv[0], stencil);


			uint2 size;


			if (table_rtv)
			{

				auto& view = std::get<HAL::Views::RenderTarget>(table_rtv.get_resource_info().view);

				std::visit(overloaded{
			[&](const HAL::Views::RenderTarget::Buffer& Buffer) {
			assert(false);
			},
			[&](const HAL::Views::RenderTarget::Texture1D& Texture1D) {
			size = view.Resource->get_desc().as_texture().get_size(Texture1D.MipSlice).xy;

			},
			[&](const HAL::Views::RenderTarget::Texture1DArray& Texture1DArray) {
				size = view.Resource->get_desc().as_texture().get_size(Texture1DArray.MipSlice).xy;
			},
			[&](const HAL::Views::RenderTarget::Texture2D& Texture2D) {

					size = view.Resource->get_desc().as_texture().get_size(Texture2D.MipSlice).xy;
			},
			[&](const HAL::Views::RenderTarget::Texture2DArray& Texture2DArray) {
		size = view.Resource->get_desc().as_texture().get_size(Texture2DArray.MipSlice).xy;
			},
			[&](const HAL::Views::RenderTarget::Texture3D& Texture3D) {
		size = view.Resource->get_desc().as_texture().get_size(Texture3D.MipSlice).xy;
			},
			[&](const HAL::Views::RenderTarget::Texture2DMS& Texture2DMS) {
assert(false);
			},
			[&](const HAL::Views::RenderTarget::Texture2DMSArray& Texture2DMSArray) {
assert(false);
			},
			[&](auto other) {
				assert(false);
			}
					}, view.View);





			}
			else
				if (table_dsv)
				{

					auto& view = std::get<HAL::Views::DepthStencil>(table_dsv.get_resource_info().view);


					std::visit(overloaded{
		[&](const HAL::Views::DepthStencil::Texture1D& Texture1D) {
size = view.Resource->get_desc().as_texture().get_size(Texture1D.MipSlice).xy;

		},
		[&](const HAL::Views::DepthStencil::Texture1DArray& Texture1DArray) {
		size = view.Resource->get_desc().as_texture().get_size(Texture1DArray.MipSlice).xy;

		},
		[&](const HAL::Views::DepthStencil::Texture2D& Texture2D) {
size = view.Resource->get_desc().as_texture().get_size(Texture2D.MipSlice).xy;

		},
		[&](const HAL::Views::DepthStencil::Texture2DArray& Texture2DArray) {
			size = view.Resource->get_desc().as_texture().get_size(Texture2DArray.MipSlice).xy;

		},
		[&](const HAL::Views::DepthStencil::Texture2DMS& Texture2DMS) {
assert(false);
		},
		[&](const HAL::Views::DepthStencil::Texture2DMSArray& Texture2DMSArray) {
assert(false);
		},
		[&](auto other) {
			assert(false);
		}
						}, view.View);


				}
			HAL::Handle active_handle = table_rtv ? table_rtv : table_dsv;


			if (size.x)
			{

				if (check(options & RTOptions::SetViewport))
				{
					std::vector<HAL::Viewport> vps(1);
					vps[0].size = size;
					vps[0].pos = { 0,0 };
					vps[0].depths = { 0,1 };

					context.set_viewports(vps);
				}

				if (check(options & RTOptions::SetScissors))
				{
					sizer_long scissors = { 0, 0, size.x, size.y };
								context.set_scissors(scissors);
				}
			}

			return *this;
		}
	};




	template<class Table>
	class RTHolder :public Table
	{

	public:
		using Table::Table;
		using Compiled = CompiledRT;

		template<class Context>
		CompiledRT compile(Context& context) const
		{
			CompiledRT compiled;


			RT_Compiler<Context> compiler;
			compiler.context = &context;
			Table::compile(compiler);


			if (compiler.rtvs.size()) {
				compiled.table_rtv = context.alloc_descriptor(static_cast<uint>(compiler.rtvs.size()), HAL::DescriptorHeapIndex{ HAL::DescriptorHeapType::RTV, HAL::DescriptorHeapFlags::None });
				for (UINT i = 0; i < (UINT)compiled.table_rtv.get_count(); i++)
				{
					compiled.table_rtv[i].place(*compiler.rtvs[i]);
				}
			}


			if (compiler.dsv)
			{
				compiled.table_dsv = context.alloc_descriptor(1u, HAL::DescriptorHeapIndex{ HAL::DescriptorHeapType::DSV, HAL::DescriptorHeapFlags::None });
				compiled.table_dsv[0].place(**compiler.dsv);
			}

			return compiled;
		}


		void set(HAL::GraphicsContext& context, RTOptions options = RTOptions::Default, float depth = 1, uint stencil = 0) const
		{
			compile(context.get_base()).set(context, options, depth, stencil);
		}

	};


}