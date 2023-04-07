export module HAL:FrameGraph;

export import "defines.h";
import Core;
import :Types;
import :CommandList;
import :ResourceViews;
import :Texture;

using namespace HAL;
 export namespace FrameGraph
{




	enum class ResourceType :int {
		UNSPECIFIED,
		Buffer,
		Texture,
		Heap
	};

	enum class ResourceFlags :int {
		None = 0,
		PixelRead = (1 << 1),
		ComputeRead = (1 << 2),
		DSRead = (1 << 3),

		UnorderedAccess = (1 << 4),
		RenderTarget = (1 << 5),
		DepthStencil = (1 << 6),
		CopyDest = (1 << 7),


		GenCPU = (1 << 8),
		ReadCPU = (1 << 9),

		Temporal = 0,
		Static = (1 << 10),
		Required = (1 << 11),

		Changed = (1 << 13)
	};

	 const constexpr ResourceFlags WRITEABLE_FLAGS =ResourceFlags::CopyDest |  ResourceFlags::UnorderedAccess | ResourceFlags::RenderTarget | ResourceFlags::DepthStencil | ResourceFlags::GenCPU;

	//struct BufferDesc
	//{
	//	size_t size;
	//	bool counted;

	//	auto   operator<=>(const  BufferDesc& r)  const = default;
	//};

	struct TextureDesc
	{
		ivec3 size;
		HAL::Format format;
		uint array_count;
		uint mip_count;

		auto   operator<=>(const  TextureDesc& r)  const = default;
	};


	struct ResourceAllocInfo;

	struct ResourceHandler
	{
		bool is_new();
		void changed();

		bool is_changed();


	public:
		friend struct TaskBuilder;
		friend struct FrameContext;
		ResourceAllocInfo* info = nullptr;

		virtual void init(ResourceAllocInfo& info) = 0;

		virtual void init_view(ResourceAllocInfo& info, HAL::FrameResources& frame) = 0;
	};





	struct Pass;

	struct UsedResources
	{
		std::set<ResourceAllocInfo*> resources;
		std::map<ResourceAllocInfo*, ResourceFlags> resource_flags;
		std::set<ResourceAllocInfo*> resource_creations;
	};



	struct ResourceRWState
	{
		bool write = false;
		std::list<Pass*> passes;

		std::list<Pass*> compute;
		std::list<Pass*> graphics;

	};


	class SyncState
	{


	public:
		enum_array<CommandListType, const Pass*> values;
		SyncState(){reset();}
		void reset();
		void set_synced(const Pass* pass);
	
		bool is_in_sync(const Pass* pass,bool equal = false);
		bool is_in_sync(const SyncState&state);

		bool is_sync_to(const SyncState&state);


			void min(const Pass*pass);
		void max(const Pass*pass);

		void min(const SyncState&state);
		void max(const SyncState&state);

	};

	struct ResourceAllocInfo
	{
		std::string name;
		// desc
		//ResourceType type;
		//MyVariant desc;
		ResourceFlags flags;


		HAL::ResourceHandle alloc_ptr;
		ResourceAllocInfo* orig = nullptr;

		HAL::ResourceDesc d3ddesc;
		HAL::HeapType heap_type;
		// setup
		SyncState used_begin;
		SyncState used_end;


		SubResourcesGPU first_state;
		bool enabled = true;

		bool is_new = false;
		bool resource_just_created = true;
		std::vector<ResourceRWState> states;
		int last_writer;
		//compile
		std::map<HAL::ResourceHandle, HAL::Resource::ptr> resource_places;
		HAL::Resource::ptr resource;

		std::shared_ptr<ResourceHandler> handler;
		std::shared_ptr<HAL::ResourceView> view;

		bool passed = false;
		size_t frame_id;

		std::set<Pass*> related;
		std::set<Pass*> related_read;

		void add_pass(Pass* pass, ResourceFlags flags);
		void reset();

		void remove_inactive();

		template<class T = ResourceHandler, class ...Args>
		T& create_handler(Args...args)
		{
			//	if (!handler)
			handler = std::make_shared<T>(args...);
			//	else
			//		*handler = T(args...);

			handler->info = this;
			return get_handler<T>();
		}

		template<class T = ResourceHandler>
		T& clone_handler(std::shared_ptr<ResourceHandler>& h)
		{
			//	if (!handler)
			handler = std::make_shared<T>(static_cast<T*>(h.get())->desc);
			//	else
			//		*handler = T(args...);

			handler->info = this;
			return get_handler<T>();
		}


		template<class T = ResourceHandler>
		T& get_handler()
		{
			return *static_cast<T*>(handler.get());
		}


		template<class T, class ...Args>
		void init_view(Args&&...args)
		{
			view = std::make_shared<T>(resource->create_view<T>(std::forward<Args>(args)...));
		}
	};


	struct Handlers
	{
		template<class T>
		class UniversalHandler : public ResourceHandler
		{
		public:
			using Desc = T;
			using View = typename T::View;
			Desc desc;

			std::string name;
			auto& operator*()
			{
				return *static_cast<View*>(info->view.get());
			}

			auto operator->()
			{
				return static_cast<View*>(info->view.get());
			}

			operator bool() const
			{
				return !!info;
			}

			UniversalHandler() = default;

			UniversalHandler(const Desc& desc) :desc(desc)
			{

				//info->buffer = HAL::BufferView();
			}
			UniversalHandler(std::string_view name) :name(name)
			{

			}

			virtual void init(ResourceAllocInfo& info) override
			{
				info.d3ddesc = desc.create_resource_desc(info.flags);
			}

			virtual void init_view(ResourceAllocInfo& info, HAL::FrameResources& frame) override
			{
				info.init_view<View>(frame, desc.as_view(info.flags));
			}
		};

		template<class T, HAL::Format::Formats format>
		struct FormattedDesc
		{

			using View = HAL::FormattedBufferView<T, format>;
			uint count;

			HAL::ResourceDesc create_resource_desc(ResourceFlags resflags)
			{
				HAL::ResFlags flags = HAL::ResFlags::ShaderResource;

				if (check(resflags & ResourceFlags::UnorderedAccess))
				{
					flags |= HAL::ResFlags::UnorderedAccess;
				}
				return HAL::ResourceDesc::Buffer(count * sizeof(Underlying<T>), flags);
			}


			HAL::FormattedBufferViewDesc as_view(ResourceFlags resflags)
			{
				return { 0, count * sizeof(Underlying<T>) };
			}
		};

		template<class T>
		struct StructuredDesc
		{

			using View = HAL::StructuredBufferView<T>;
			uint count;
			bool counted;
			HAL::ResourceDesc create_resource_desc(ResourceFlags resflags)
			{
				HAL::ResFlags flags = HAL::ResFlags::ShaderResource;

				if (check(resflags & ResourceFlags::UnorderedAccess))
				{
					flags |= HAL::ResFlags::UnorderedAccess;
				}

				uint size = count * sizeof(Underlying<T>);

				if (counted)
				{
					uint local_offset = Math::roundUp(4, sizeof(Underlying<T>));
					size += local_offset;
				}

				return HAL::ResourceDesc::Buffer(size, flags);
			}


			HAL::StructuredBufferViewDesc as_view(ResourceFlags resflags)
			{
				return { 0, count * sizeof(Underlying<T>), counted };
			}
		};


		struct TextureDesc
		{

			using View = HAL::TextureView;
			ivec3 size;
			HAL::Format format;
			UINT array_count;
			UINT mip_count;

			HAL::ResourceDesc create_resource_desc(ResourceFlags resflags)
			{

				HAL::ResFlags flags = HAL::ResFlags::None;

				if (check(resflags & ResourceFlags::RenderTarget))
				{
					flags |= HAL::ResFlags::RenderTarget;
				}

				if (check(resflags & ResourceFlags::DepthStencil))
				{
					flags |= HAL::ResFlags::DepthStencil;
				}

				if (check(resflags & ResourceFlags::UnorderedAccess))
				{
					flags |= HAL::ResFlags::UnorderedAccess;
				}

				if (format.is_shader_visible())
					flags |= HAL::ResFlags::ShaderResource;

				if (mip_count == 0) {
					mip_count = 1;
					auto tsize = size;

					while (tsize.x != 1 && tsize.y != 1)
					{
						tsize /= 2;
						mip_count++;
					}

				}

				return HAL::ResourceDesc::Tex2D(format, size.xy, array_count, mip_count, flags);
			}


			HAL::TextureViewDesc as_view(ResourceFlags resflags)
			{
				return { 0, mip_count, 0, array_count };
			}
		};

		struct Texture3DDesc
		{

			using View = HAL::Texture3DView;
			ivec3 size;
			HAL::Format format;
			UINT mip_count;

			HAL::ResourceDesc create_resource_desc(ResourceFlags resflags)
			{

				HAL::ResFlags flags = HAL::ResFlags::None;

				if (check(resflags & ResourceFlags::RenderTarget))
				{
					flags |= HAL::ResFlags::RenderTarget;
				}

				if (check(resflags & ResourceFlags::DepthStencil))
				{
					flags |= HAL::ResFlags::DepthStencil;
				}

				if (check(resflags & ResourceFlags::UnorderedAccess))
				{
					flags |= HAL::ResFlags::UnorderedAccess;
				}

				if (format.is_shader_visible())
					flags |= HAL::ResFlags::ShaderResource;

				if (mip_count == 0) {
					mip_count = 1;
					auto tsize = size;

					while (tsize.x != 1 && tsize.y != 1 && tsize.z != 1)
					{
						tsize /= 2;
						mip_count++;
					}

				}

				return HAL::ResourceDesc::Tex3D(format, size, mip_count, flags);
			}


			HAL::Texture3DViewDesc as_view(ResourceFlags resflags)
			{
				return { 0, mip_count };
			}
		};


		struct CubeDesc
		{

			using View = HAL::CubeView;
			ivec3 size;
			HAL::Format format;
			UINT array_count;
			UINT mip_count;

			HAL::ResourceDesc create_resource_desc(ResourceFlags resflags)
			{

				HAL::ResFlags flags = HAL::ResFlags::None;

				if (check(resflags & ResourceFlags::RenderTarget))
				{
					flags |= HAL::ResFlags::RenderTarget;
				}

				if (check(resflags & ResourceFlags::DepthStencil))
				{
					flags |= HAL::ResFlags::DepthStencil;
				}

				if (check(resflags & ResourceFlags::UnorderedAccess))
				{
					flags |= HAL::ResFlags::UnorderedAccess;
				}

				if (format.is_shader_visible())
					flags |= HAL::ResFlags::ShaderResource;

				if (mip_count == 0) {
					mip_count = 1;
					auto tsize = size;

					while (tsize.x != 1 && tsize.y != 1)
					{
						tsize /= 2;
						mip_count++;
					}

				}

				return HAL::ResourceDesc::Tex2D(format, size.xy, array_count * 6, mip_count, flags);
			}


			HAL::CubeViewDesc as_view(ResourceFlags resflags)
			{
				return { 0, mip_count, 0, array_count * 6 };
			}
		};


		template<class T, HAL::Format::Formats format>
		using FormattedBuffer = UniversalHandler<FormattedDesc<T, format>>;


		template<class T>
		using StructuredBuffer = UniversalHandler<StructuredDesc<T>>;

		using Texture = UniversalHandler<TextureDesc>;


		using Cube = UniversalHandler<CubeDesc>;
		using Texture3D = UniversalHandler<Texture3DDesc>;

	};

	struct TaskBuilderResourceAllocationContext
	{
		using AllocatorType = CommonAllocator;
		using LockPolicy = Thread::Free;
	};

	struct TaskBuilder
	{
		using MemoryAllocatorType = Allocators::HeapPageManager<ResourceContext, TaskBuilderResourceAllocationContext>;
	
	private:

	public:
		std::map<std::string, std::string> resources_names;

		std::map<std::string, ResourceAllocInfo> alloc_resources;

		std::set<ResourceAllocInfo*> passed_resources;
			std::list<std::shared_ptr<Pass>> required_passes;
		std::list<Pass*> enabled_passes;
		MemoryAllocatorType allocator;
		HAL::FrameResourceManager frames;
		HAL::FrameResources::ptr current_frame;

	
		Pass* current_pass = nullptr;
		void begin(Pass* pass);

		void end(Pass* pass);


		void init(ResourceAllocInfo& info, std::string name, ResourceFlags flags);
		void init_pass(ResourceAllocInfo& info, ResourceFlags flags);


		template<class T>
		void create(T& result, const typename T::Desc& desc, ResourceFlags flags = ResourceFlags::None)
		{
			std::string& name = result.name;
			resources_names[name] = name;
			ResourceAllocInfo& info = alloc_resources[name];
			T& handler = info.create_handler<T>(desc);
			init(info, name, flags);
			//handler.init(info);
			result = handler;

		}
		template<class T>
		void recreate(T& result, ResourceFlags flags = ResourceFlags::None)
		{
			std::string name = result.name;
			ResourceAllocInfo& old_info = alloc_resources[name];
			std::string new_name = resources_names[name] + "recreated";
			resources_names[name] = new_name;
			name = new_name;


			ResourceAllocInfo& info = alloc_resources[name];
			T& handler = info.clone_handler<T>(old_info.handler);
			init(info, name, flags);
			//handler.init(info);
			info.orig = &old_info;


			result = handler;
		}


		template<class T>
		void recreate(T& result, const typename T::Desc& desc, ResourceFlags flags = ResourceFlags::None)
		{
			std::string name = result.name;
			ResourceAllocInfo& old_info = alloc_resources[name];
			std::string new_name = resources_names[name] + "recreated";
			resources_names[name] = new_name;
			name = new_name;


			ResourceAllocInfo& info = alloc_resources[name];
			T& handler = info.create_handler<T>(desc);
			init(info, name, flags);
			//handler.init(info);
			info.orig = &old_info;


			result = handler;
		}

		template<class T>
		bool exists(T& result)
		{
			std::string& name = resources_names[result.name];
			return !name.empty();
		}

		template<class T>
		void need(T& result, ResourceFlags flags = ResourceFlags::None)
		{
			std::string& name = resources_names[result.name];
			ResourceAllocInfo& info = alloc_resources[name];
			T& handler = info.get_handler<T>();

			init_pass(info, flags);

			result = handler;
		}

		//void free_texture(ResourceHandler* handler);
		void pass_texture(std::string name, HAL::Texture::ptr tex, ResourceFlags flags = ResourceFlags::None);


		void create_resources();
		void reset();

		TaskBuilder();



		ResourceAllocInfo* get(std::string name)
		{
			if (resources_names.count(name) == 0) return nullptr;
			name = resources_names[name];
			ResourceAllocInfo& info = alloc_resources[name];
			if (!info.enabled)return nullptr;;
			return &info;
		}
	};



	struct FrameContext
	{
		Pass* pass;
		HAL::FrameResources::ptr frame;

		std::list<HAL::ResourceView> textureViews;

		HAL::CommandList::ptr list;

		HAL::CommandList::ptr& get_list();
		void begin(Pass* pass, HAL::FrameResources::ptr& frame);
		void end();


		void execute();

		void register_subview(const HAL::ResourceView& view)
		{
			textureViews.push_back(view);
		}
	};


	enum class PassFlags
	{
		General = 0,
		Required = 1,

		Graphics = 0,
		Compute = 2
	};


	struct Pass
	{
		UINT id = 0;
		UINT call_id;
		int dependency_level;
		bool enabled = true;
		bool renderable = true;
		PassFlags flags;
		std::string name;
		UsedResources used;
		FrameContext context;

		SyncState sync_state;
		std::set<Pass*> prev_passes;

		std::set<Pass*> next_passes;
		std::future<void> render_task;
		HAL::FenceWaiter fence_end;

		int graphic_count = 0;
		int compute_count = 0;
		Pass* wait_pass = nullptr;


		
		Pass* prev_pass = nullptr;

		virtual bool setup(TaskBuilder& builder) = 0;

		HAL::CommandListType get_type() const
		{
			HAL::CommandListType type = HAL::CommandListType::DIRECT;

			if (check(flags & PassFlags::Compute))
				type = HAL::CommandListType::COMPUTE;

			return type;
		}
		void compile(TaskBuilder& builder);

		virtual void render(HAL::FrameResources::ptr& frame) = 0;
		void wait();
		void execute();

		bool active()
		{
			return enabled && renderable;
		}

		// optimization
		bool inserted = false;
	};


	template <class Handler>
	struct TypedPass : public Pass
	{

		using render_func_type = std::function<void(Handler&, FrameContext&)>;
		using setup_func_type = std::function<bool(Handler&, TaskBuilder&)>;
		using setup_func_type_void = std::function<void(Handler&, TaskBuilder&)>;


		Handler data;

		setup_func_type setup_func;
		render_func_type render_func;

		TypedPass(int id, std::string name, setup_func_type s, render_func_type r)
		{
			this->id = id;
			this->name = name;

			setup_func = s;
			render_func = r;
		}

		virtual bool setup(TaskBuilder& builder) override
		{
			builder.begin(this);
			bool res = setup_func(data, builder);
			builder.end(this);

			return res;
		}

		virtual void render(HAL::FrameResources::ptr& frame) override
		{
			if (!enabled || !renderable)  return;



			render_task = thread_pool::get().enqueue([this, &frame]() {
				context.begin(this, frame);
				render_func(data, context);
				context.end();
				});

		//		render_task.wait();
		}
	};



	//struct CreationContext
	//{
	//	ivec2 frame_size;
	//	ivec2 upscale_size;
	//	camera* cam;
	//	main_renderer* renderer;
	//	Scene* scene;

	//	float time;
	//	float totalTime = 0;
	//	float3 sunDir;
	//};

	//ENABLE_ENUM(wtf);

	class SlotContext
	{
	protected:
		std::map<SlotID, std::function<void(HAL::SignatureDataSetter&)>> slot_setters;

	public:


		template<class Compiled>
		void register_slot_setter(Compiled compiled)
		{
			SlotID id = Compiled::ID;
			slot_setters[id] = [compiled](HAL::SignatureDataSetter& setter) {
				compiled.set(setter);
			};
		}

		void set_slot(SlotID id, HAL::SignatureDataSetter& setter)
		{
		//	assert(slot_setters.contains(id));
			slot_setters[id](setter);
		}

	};

	
	class Graph: public UniversalContext, public VariableContext, public SlotContext
	{	
	public:

		std::list<std::shared_ptr<Pass>> passes;

	
		Variable<bool> optimize = { true, "optimize", this };

		std::list<std::function<void(Graph& g)>> pre_run;
		template<class Pass>
		void internal_pass(std::string name, typename Pass::setup_func_type s, typename Pass::render_func_type r, PassFlags flags = PassFlags::General)
		{
			passes.push_back(std::make_shared<Pass>((UINT)passes.size(), name, s, r));
			passes.back()->flags = flags;

			if (check(flags & PassFlags::Required))
			{
				builder.required_passes.push_back(passes.back());
			}
		}

	public:

		Graph() :VariableContext(L"Graph")
		{
		}
		TaskBuilder builder;

		template<class T>
		void pass(std::string name, typename TypedPass<T>::setup_func_type s, typename TypedPass<T>::render_func_type r, PassFlags flags = PassFlags::General)
		{
			internal_pass<TypedPass<T>>(name, s, r, flags);
		}

		template<class T>
		void add_pass(std::string name, typename TypedPass<T>::setup_func_type_void s, typename TypedPass<T>::render_func_type r, PassFlags flags = PassFlags::General)
		{
			typename TypedPass<T>::setup_func_type f = [s](auto& t, auto& b) {s(t, b); return true; };

			internal_pass<TypedPass<T>>(name, f, r, flags);
		}


		template<class T>
		void add_pass2(std::string name, typename TypedPass<T>::setup_func_type_void s, typename TypedPass<T>::render_func_type r, PassFlags flags = PassFlags::General)
		{
			typename TypedPass<T>::setup_func_type f = [s](auto& t, auto& b) {s(t, b); return true; };

			internal_pass<TypedPass<T>>(name, f, r, flags);
		}


		void add_slot_generator(std::function<void(Graph&)> f)
		{
			pre_run.push_back(f);
		}
		void start_new_frame();

		void setup();
		void compile(int frame);
		void render();


		void commit_command_lists();
		void reset();
	};

	class GraphGenerator
	{
	public:
		virtual void generate(Graph& graph) = 0;
	};



	class GraphUsage
	{
	public:
		virtual void use(TaskBuilder& builder) = 0;
	};

}
