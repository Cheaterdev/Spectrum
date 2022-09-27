#pragma once

import Graphics;
import Scheduler;

import Utils;
import serialization;
class camera;
class main_renderer;
class Scene;


import Math;


namespace FrameGraph
{


	enum class ResourceType :int {
		UNSPECIFIED,
		Buffer,
		Texture,
		Heap

		, GENERATE_OPS
	};

	enum class ResourceFlags :int {
		None = 0,
		PixelRead = (1 << 1),
		ComputeRead = (1 << 2),
		DSRead = (1 << 3),

		UnorderedAccess = (1 << 4),
		RenderTarget = (1 << 5),
		DepthStencil = (1 << 6),


		GenCPU = (1 << 7),
		ReadCPU = (1 << 8),

		Temporal = 0,
		Static = (1 << 9),
		Required = (1 << 10),

		Cube = (1 << 11),
		Counted = (1 << 12),
		Changed = (1 << 13)

		, GENERATE_OPS
	};

	static const ResourceFlags WRITEABLE_FLAGS = ResourceFlags::UnorderedAccess | ResourceFlags::RenderTarget | ResourceFlags::DepthStencil | ResourceFlags::GenCPU;

	struct BufferDesc
	{
		size_t size;
		bool counted;

		auto   operator<=>(const  BufferDesc& r)  const = default;
	};

	struct TextureDesc
	{
		ivec3 size;
		Graphics::Format format;
		UINT array_count;
		UINT mip_count;

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

		virtual void init_view(ResourceAllocInfo& info, Graphics::FrameResources& frame) = 0;
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
	struct ResourceAllocInfo
	{
		std::string name;
		// desc
		ResourceType type;
		MyVariant desc;
		ResourceFlags flags;
		bool placed;

		Graphics::ResourceHandle alloc_ptr;
		ResourceAllocInfo* orig = nullptr;

		HAL::ResourceDesc d3ddesc;
		Graphics::HeapType heap_type;
		// setup
		Pass* valid_from = nullptr;
		Pass* valid_to = nullptr;
		Pass* valid_to_start = nullptr;


		bool enabled = true;

		bool is_new = false;

		std::vector<ResourceRWState> states;
		int last_writer;
		//compile
		std::map<Graphics::ResourceHandle, Graphics::Resource::ptr> resource_places;
		Graphics::Resource::ptr resource;

		std::shared_ptr<ResourceHandler> handler;
		std::shared_ptr<Graphics::ResourceView> view;

		bool need_recreate = false;
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
			handler = std::make_shared<T>(static_cast<T*>(h.get())->m_desc);
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
#define H(x) x = #x

	struct Handlers
	{
		template<class T>
		class StructuredBuffer : public ResourceHandler
		{

		public:
			std::string name;
			struct Desc
			{
				UINT count;
				bool counted;
			}m_desc;

			auto& operator*()
			{
				return *static_cast<Graphics::StructuredBufferView<T>*>(info->view.get());
			}

			auto operator->()
			{
				return static_cast<Graphics::StructuredBufferView<T>*>(info->view.get());
			}
			operator bool() const
			{
				return !!info;
			}
			StructuredBuffer() = default;

			StructuredBuffer(const Desc& desc) :m_desc(desc)
			{

				//info->buffer = Graphics::BufferView();
			}
			StructuredBuffer(std::string_view name) :name(name)
			{

			}
			virtual void init(ResourceAllocInfo& info) override
			{
				auto s = m_desc.count * sizeof(Underlying<T>);
				if (m_desc.counted)
				{
					s += std::max(4ull, sizeof(Underlying<T>));
				}
				auto desc = BufferDesc{ s , m_desc.counted };
				info.need_recreate = info.desc != desc;
				info.type = ResourceType::Buffer;
				info.desc = desc;

			}

			virtual void init_view(ResourceAllocInfo& info, Graphics::FrameResources& frame) override
			{
				info.init_view<Graphics::StructuredBufferView<T>>(frame, m_desc.counted ? Graphics::BufferType::COUNTED : Graphics::BufferType::NONE);
			}
		};

		class Texture : public ResourceHandler
		{
		public:
			struct Desc
			{
				ivec3 size;
				Graphics::Format format;
				UINT array_count;
				UINT mip_count;
			}m_desc;

			std::string name;
			auto& operator*()
			{
				return *static_cast<Graphics::TextureView*>(info->view.get());
			}
			auto operator->()
			{
				return static_cast<Graphics::TextureView*>(info->view.get());
			}
			operator bool() const
			{
				return !!info;
			}
			Texture() = default;

			Texture(std::string_view name) :name(name)
			{

			}

			Texture(const Desc& desc) :m_desc(desc)
			{

			}

			virtual void init(ResourceAllocInfo& info)override
			{
				auto desc = TextureDesc{ m_desc.size, m_desc.format, m_desc.array_count,m_desc.mip_count };
				info.need_recreate = info.desc != desc;
				info.type = ResourceType::Texture;
				info.desc = desc;
			}

			virtual void init_view(ResourceAllocInfo& info, Graphics::FrameResources& frame) override
			{
				info.init_view<Graphics::TextureView>(frame, check(info.flags & ResourceFlags::Cube));
			}
		};
	};


	struct Runtime
	{
		using Texture = Graphics::TextureView;

		template<class T>
		using StructuredBuffer = Graphics::StructuredBufferView<T>;
	};



	struct TaskBuilder
	{

	private:

	public:
		std::map<std::string, std::string> resources_names;

		std::map<std::string, ResourceAllocInfo> alloc_resources;

		std::set<ResourceAllocInfo*> passed_resources;

		Graphics::ResourceHeapAllocator<Thread::Free> allocator;
		Graphics::ResourceHeapAllocator<Thread::Free> static_allocator;


		Graphics::FrameResourceManager frames;
		Graphics::FrameResources::ptr current_frame;

		std::map<int, Graphics::ResourceHeapAllocator<Thread::Free>> frame_allocs;

		Graphics::ResourceHeapAllocator<Thread::Free>* current_alloc;
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
			handler.init(info);
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
			handler.init(info);
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
			handler.init(info);
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
		void pass_texture(std::string name, Graphics::Texture::ptr tex, ResourceFlags flags = ResourceFlags::None);


		void create_resources();
		void reset();

		TaskBuilder();



		ResourceAllocInfo* get(std::string name)
		{
			if (resources_names.count(name) == 0) return nullptr;
			name = resources_names[name];
			ResourceAllocInfo& info = alloc_resources[name];
			return &info;
		}
	};



	struct FrameContext
	{
		Pass* pass;
		Graphics::FrameResources::ptr frame;

		std::list<Graphics::ResourceView> textureViews;

		Graphics::CommandList::ptr list;

		Graphics::CommandList::ptr& get_list();
		void begin(Pass* pass, Graphics::FrameResources::ptr& frame);
		void end();


		void execute();

		void register_subview(const Graphics::ResourceView& view)
		{
			textureViews.push_back(view);
		}
	};


	enum class PassFlags
	{
		General = 0,
		Required = 1,

		Graphics = 0,
		Compute = 2,

		GENERATE_OPS
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
		std::set<Pass*> related;
		std::future<void> render_task;
		Graphics::FenceWaiter fence_end;

		int graphic_count = 0;
		int compute_count = 0;
		Pass* wait_pass = nullptr;
		Pass* prev_pass = nullptr;

		virtual bool setup(TaskBuilder& builder) = 0;

		Graphics::CommandListType get_type()
		{
			Graphics::CommandListType type = Graphics::CommandListType::DIRECT;

			if (check(flags & PassFlags::Compute))
				type = Graphics::CommandListType::COMPUTE;

			return type;
		}
		void compile(TaskBuilder& builder);

		virtual void render(Graphics::FrameResources::ptr& frame) = 0;
		void wait();
		void execute();

		bool active()
		{
			return enabled && renderable;
		}
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

		virtual void render(Graphics::FrameResources::ptr& frame) override
		{
			if (!enabled || !renderable)  return;



			render_task = scheduler::get().enqueue([this, &frame]() {

				context.begin(this, frame);
				render_func(data, context);
				context.end();
				}, std::chrono::steady_clock::now());

			//	render_task.wait();
		}
	};



	struct CreationContext
	{
		ivec2 frame_size;
		ivec2 upscale_size;
		camera* cam;
		main_renderer* renderer;
		Scene* scene;

		float time;
		float totalTime = 0;
		float3 sunDir;
	};

	//ENABLE_ENUM(wtf);

	class SlotContext
	{
		std::map<SlotID, std::function<void(Graphics::SignatureDataSetter&)>> slot_setters;

	public:


		template<class Compiled>
		void register_slot_setter(Compiled compiled)
		{
			SlotID id = Compiled::ID;
			slot_setters[id] = [compiled](Graphics::SignatureDataSetter& setter) {
				compiled.set(setter);
			};
		}

		void set_slot(SlotID id, Graphics::SignatureDataSetter& setter)
		{
			assert(slot_setters.contains(id));
			slot_setters[id](setter);
		}

	};
	class Graph : public CreationContext, VariableContext, public SlotContext
	{
	public:
		std::list<std::shared_ptr<Pass>> passes;

		std::list<std::shared_ptr<Pass>> required_passes;
		std::list<Pass*> enabled_passes;
		Variable<bool> optimize = { true, "optimize", this };

		std::list<std::function<void(Graph& g)>> pre_run;
		template<class Pass>
		void internal_pass(std::string name, typename Pass::setup_func_type s, typename Pass::render_func_type r, PassFlags flags = PassFlags::General)
		{
			passes.push_back(std::make_shared<Pass>((UINT)passes.size(), name, s, r));
			passes.back()->flags = flags;

			if (check(flags & PassFlags::Required))
			{
				required_passes.push_back(passes.back());
			}
		}

	public:

		Graph() :VariableContext(L"Graph") {}
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
