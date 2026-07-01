#include "autogen/resource_ids.h"
#include "autogen/pass_ids.h"

export module FrameGraph:Base;

import Core;
import HAL;


using namespace HAL;


namespace Pipelines
{

class PipelineBase
{
public:
	virtual ~PipelineBase() = default;
	virtual std::span<const wchar_t* const> GetUsedPassNamesList() const = 0;
	virtual std::span<const wchar_t* const> GetUsedResourcesList() const = 0;
};

}



 export namespace FrameGraph
{
	class Graph;


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
		CopySource = (1 << 8),


	//	GenCPU = (1 << 9),
	//	ReadCPU = (1 << 10),

		Temporal = 0,
		Static = (1 << 11),
		Required = (1 << 12),

		Changed = (1 << 13)
	};

	 constexpr ResourceFlags WRITEABLE_FLAGS =ResourceFlags::CopyDest |  ResourceFlags::UnorderedAccess | ResourceFlags::RenderTarget | ResourceFlags::DepthStencil;// | ResourceFlags::GenCPU;

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

		virtual void init_view(ResourceAllocInfo& info, GPUEntityStorageInterface& frame) = 0;
	};





	struct Pass;

	struct UsedResources
	{
		std::list<HAL::FenceWaiter> fences;
		std::set<ResourceAllocInfo*> resources;
		std::map<ResourceAllocInfo*, ResourceFlags> resource_flags;
		std::set<ResourceAllocInfo*> resource_creations;


		std::set<ResourceAllocInfo*> resource_deletions_before;
		std::set<ResourceAllocInfo*> resource_deletions_after;


	};



	class SyncState
	{


	public:
		enum_array<CommandListType, const Pass*> values;
		SyncState();
		void reset();
		void set_synced(const Pass* pass);
	
		bool is_in_sync(const Pass* pass,bool equal = false);
		bool is_in_sync(const SyncState&state, bool equal = false);

		//bool is_sync_to(const SyncState&state);
			bool is_sync_to(const Pass* pass,bool equal = false);


			void min(const Pass*pass);
		void max(const Pass*pass);

		void min(const SyncState&state);
		void max(const SyncState&state);

	};
	

	struct ResourceRWState
	{
		bool write = false;
		std::vector<Pass*> passes;

		std::vector<Pass*> compute;
		std::vector<Pass*> graphics;


		HAL::SubResourcesGPU merged_read_state;

		SyncState from;
		SyncState to;
	};

	struct ResourceAllocInfo
	{
		ResourceID id = ResourceID::Count;
		const char* name() const;
		// desc
		//ResourceType type;
		//MyVariant desc;
		ResourceFlags flags;


		HAL::ResourceHandle alloc_ptr;

		HAL::ResourceDesc d3ddesc;
		HAL::HeapType heap_type;
		// setup
		SyncState used_begin;
		SyncState used_end;


		bool is_static() const;
		bool is_dynamic() const;


		bool enabled = false;

		bool is_new = false;
		bool resource_just_created = true;
		std::vector<ResourceRWState> states;
		int last_writer;
		size_t max_passes = 0; // upper bound on passes per state, known from Pass::id assignment
		//compile

		struct CompiledResource
		{
			HAL::Resource::ptr resource;
			std::shared_ptr<HAL::ResourceView> view;

			operator bool();
		};

		std::map<HAL::ResourceHandle,CompiledResource> resource_places;
		
		HAL::Resource::ptr resource;
		uint64 offset_in_bytes;

		std::shared_ptr<HAL::ResourceView> view;
			HAL::SubResourcesGPU creation_state;

		HAL::SubResourcesGPU last_state;

		std::shared_ptr<ResourceHandler> handler;
	
		bool passed      = false;
		bool non_deleted = false;
		HAL::FenceWaiter fence;
		size_t frame_id;


				Events::Event<Pass*, FrameContext*>	process_debug_resource;




		void add_pass(Pass* pass, ResourceFlags flags);
		void resolve_dependencies(std::vector<std::pair<Pass*, Pass*>>& edges);
		void reset(size_t max_passes);

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

			ResourceID id = ResourceID::Count;
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
			}
			UniversalHandler(ResourceID id) :id(id)
			{
			}

			virtual void init(ResourceAllocInfo& info) override
			{
				info.d3ddesc = desc.create_resource_desc(info.flags);
				
			info.d3ddesc.Flags|=HAL::ResFlags::DisableStateTracking;

			}

			virtual void init_view(ResourceAllocInfo& info,GPUEntityStorageInterface& frame) override
			{
				info.init_view<View>(frame, desc.as_view(info.offset_in_bytes, info.flags));
			}
		};

		struct ByteBufferDesc
		{

			using View = HAL::ByteBufferView;
			uint64 count;

			HAL::ResourceDesc create_resource_desc(ResourceFlags resflags);

			ByteBufferViewDesc as_view(uint64 offset, ResourceFlags resflags);
		};


		template<class T, HAL::Format::Formats format>
		struct FormattedDesc
		{

			using View = HAL::FormattedBufferView<T, format>;
			uint64 count;

			HAL::ResourceDesc create_resource_desc(ResourceFlags resflags)
			{
				HAL::ResFlags flags = HAL::ResFlags::ShaderResource;

				if (check(resflags & ResourceFlags::UnorderedAccess))
				{
					flags |= HAL::ResFlags::UnorderedAccess;
				}
				return HAL::ResourceDesc::Buffer(count * sizeof(Underlying<T>), flags);
			}


			HAL::FormattedBufferViewDesc as_view(uint64 offset,ResourceFlags resflags)
			{
				return { offset, count * sizeof(Underlying<T>) };
			}
		};

		template<class T>
		struct StructuredDesc
		{

			using View = HAL::StructuredBufferView<T>;
			uint64 count;
			bool counted;
			HAL::ResourceDesc create_resource_desc(ResourceFlags resflags)
			{
				HAL::ResFlags flags = HAL::ResFlags::ShaderResource;

				if (check(resflags & ResourceFlags::UnorderedAccess))
				{
					flags |= HAL::ResFlags::UnorderedAccess;
				}

				uint64 size = count * sizeof(Underlying<T>);

				if (counted)
				{
					uint local_offset = Math::roundUp(4, sizeof(Underlying<T>));
					size += local_offset;
				}

				return HAL::ResourceDesc::Buffer(size, flags);
			}


			HAL::StructuredBufferViewDesc as_view(uint64 offset,ResourceFlags resflags)
			{
				return { offset, count * sizeof(Underlying<T>), counted?counterType::SELF:counterType::NONE };
			}
		};


		struct TextureDesc
		{

			using View = HAL::Texture2DView;
			ivec3 size;
			HAL::Format format;
			UINT array_count;
			UINT mip_count;

			HAL::ResourceDesc create_resource_desc(ResourceFlags resflags);

			HAL::TextureViewDesc as_view(uint64 offset,ResourceFlags resflags);
		};

		struct Texture3DDesc
		{

			using View = HAL::Texture3DView;
			ivec3 size;
			HAL::Format format;
			UINT mip_count;

			HAL::ResourceDesc create_resource_desc(ResourceFlags resflags);

			HAL::Texture3DViewDesc as_view(uint64 offset,ResourceFlags resflags);
		};


		struct CubeDesc
		{

			using View = HAL::CubeView;
			ivec3 size;
			HAL::Format format;
			UINT array_count;
			UINT mip_count;

			HAL::ResourceDesc create_resource_desc(ResourceFlags resflags);

			HAL::CubeViewDesc as_view(uint64 offset,ResourceFlags resflags);
		};


		template<class T, HAL::Format::Formats format>
		using FormattedBuffer = UniversalHandler<FormattedDesc<T, format>>;

		using ByteAdressBuffer = UniversalHandler<ByteBufferDesc>;

		template<class T>
		using StructuredBuffer = UniversalHandler<StructuredDesc<T>>;

		using Texture = UniversalHandler<TextureDesc>;


		using TextureCube = UniversalHandler<CubeDesc>;
		using Texture3D = UniversalHandler<Texture3DDesc>;

	};

	struct TaskBuilderResourceAllocationContext
	{
		using AllocatorType = CommonAllocator;
		using LockPolicy = Thread::Free;
	};

	struct ResourceChain
	{
		bool   empty() const { return items.empty(); }
		size_t size()  const { return items.size(); }

		ResourceAllocInfo&       active()       { return items[pos]; }
		const ResourceAllocInfo& active() const { return items[pos]; }

		auto active_span()
		{
			size_t n = std::min(pos + 1, items.size());
			return std::ranges::subrange(items.begin(), items.begin() + (std::ptrdiff_t)n);
		}

		// Ensure at least one slot exists (used by create()).
		void ensure_first()
		{
			if (items.empty())
				items.emplace_back();
		}

		// Advance to the next slot, growing the deque if needed.
		ResourceAllocInfo& emplace_back()
		{
			++pos;
			if (items.size() <= pos)
				items.emplace_back();
			return items[pos];
		}

		void reset_frame() { pos = 0; }

	private:
		std::deque<ResourceAllocInfo> items;
		size_t pos = 0;
	};

	struct TaskBuilder
	{
		using MemoryAllocatorType = Allocators::PagedAllocator<ResourceContext, TaskBuilderResourceAllocationContext>;

	private:

	public:
		std::array<ResourceChain, (size_t)ResourceID::Count> alloc_resources;

		// Compact list of resources enabled this frame, populated during the
		// enable-marking pass in Graph::setup(). Lets later passes (create_resources)
		// iterate only what matters instead of rescanning all of alloc_resources.
		std::vector<ResourceAllocInfo*> enabled_resources;

		// Persistent pass cache, indexed by PassID (+ index for [Multiple] passes),
		// so library passes are reused frame to frame instead of reallocated. Only
		// covers add_library_pass — the raw add_pass<T>(name, ...) path has no PassID
		// (type_id == PassID::Count) and always allocates fresh.
		std::array<std::vector<std::shared_ptr<Pass>>, (size_t)PassID::Count> pass_cache;

		std::set<ResourceAllocInfo*> passed_resources;
		std::shared_ptr<Pass>         external_pass;   // fake creator pass for passed_resources
		std::list<std::shared_ptr<Pass>> passes;
		std::list<std::shared_ptr<Pass>> required_passes;
		std::list<Pass*> enabled_passes;
		MemoryAllocatorType allocator;
		HAL::FrameResourceManager frames;
		HAL::FrameResources::ptr current_frame;

		std::map<uint, Pass*> id_to_pass;

		StaticCompiledGPUData global_frame;

				// Kicked off at the end of render(), once every pass has finished recording and
		// no more descriptor writes for the frame can happen. Waited on in
		// commit_command_lists(), just before the first command list is submitted.
		std::future<void> descriptor_commit_task;


		bool debug = false;
		Pass* current_pass = nullptr;
		Graph* graph = nullptr;
		void begin(Pass* pass);

		void end(Pass* pass);


		void init(ResourceAllocInfo& info, ResourceFlags flags);
		void init_pass(ResourceAllocInfo& info, ResourceFlags flags);


		template<class T>
		void create(T& result, const typename T::Desc& desc, ResourceFlags flags = ResourceFlags::None)
		{
			auto& chain = alloc_resources[(size_t)result.id];
			chain.reset_frame();
			chain.ensure_first();

			ResourceAllocInfo& info = chain.active();
			info.id = result.id;
			T& handler = info.create_handler<T>(desc);
			init(info, flags);
			result = handler;
			result.id = info.id;
		}

		template<class T>
		void recreate(T& result, ResourceFlags flags = ResourceFlags::None)
		{
			auto& chain = alloc_resources[(size_t)result.id];
			ASSERT(!chain.empty());
			ResourceAllocInfo& old = chain.active();
			ResourceAllocInfo& info = chain.emplace_back();

			if (check(old.flags & ResourceFlags::Required))
				flags |= ResourceFlags::Required;
			info.id = result.id;
			T& handler = info.clone_handler<T>(old.handler);
			init(info, flags);
			result = handler;
			result.id = info.id;
		}

		template<class T>
		void recreate(T& result, const typename T::Desc& desc, ResourceFlags flags = ResourceFlags::None)
		{
			auto& chain = alloc_resources[(size_t)result.id];
			ASSERT(!chain.empty());
			ResourceFlags old_flags = chain.active().flags;
			ResourceAllocInfo& info = chain.emplace_back();

			if (check(old_flags & ResourceFlags::Required))
				flags |= ResourceFlags::Required;
			info.id = result.id;
			T& handler = info.create_handler<T>(desc);
			init(info, flags);
			result = handler;
			result.id = info.id;
		}

		template<class T>
		bool exists(T& result)
		{
			auto& chain = alloc_resources[(size_t)result.id];
			return !chain.empty();
		}

		template<class T>
		void need(T& result, ResourceFlags flags = ResourceFlags::None)
		{
			ASSERT(exists(result));
			auto& chain = alloc_resources[(size_t)result.id];
			ResourceAllocInfo& info = chain.active();
			T& handler = info.get_handler<T>();
			init_pass(info, flags);
			result = handler;
			result.id = info.id;
		}

		//void free_texture(ResourceHandler* handler);
		void pass_texture(ResourceID id, HAL::TextureResource::ptr tex, HAL::FenceWaiter fence = {}, ResourceFlags flags = ResourceFlags::None);


		void create_resources();
		void process_transitions();
		void process_fences();
	   	void compile_lists();
		void reset();

		TaskBuilder();



		Pass* get_pass(LiteralWStr name) const;

		Pass* get_pass(uint id) const;

		ResourceAllocInfo* get(ResourceID id);
	};

	  class Graph;

	struct FrameContext
	{
		Graph* graph;
		Pass* pass;
		HAL::FrameResources::ptr frame;
		HAL::CommandList::ptr list;
		HAL::CommandList::ptr& get_list();
		void begin(Graph* graph,Pass* pass, HAL::FrameResources::ptr& frame);
		void end();
		void execute();

		HAL::StructuredBufferView<DispatchArguments>& get_indirect_dispatch_args();
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
		PassID type_id = PassID::Count;
		UINT call_id;
		int dependency_level;
		bool enabled = false;
		bool renderable = true;
		PassFlags flags;
		LiteralWStr name{L""};				  


		uint32_t pass_index = 0;

		uint32_t GetPassIndex() const;
		UsedResources used;
		FrameContext context;

		SyncState sync_state;
		 
		SyncState sync_state_with_self;


		std::unordered_set<Pass*> prev_passes;

		std::unordered_set<Pass*> next_passes;
		std::future<void> render_task;
		std::future<void> compile_task;

		// Populated after compile_lists(); available during on_compile.
		// Transition records have barrier_point == nullptr (resolved into description).
		std::vector<HAL::CommandRecord> debug_commands;

		HAL::FenceWaiter fence_end;

		int graphic_count = 0;
		int compute_count = 0;
		Pass* wait_pass = nullptr;

		bool put_fence = false;
		
		Pass* prev_pass = nullptr;

		virtual bool setup(TaskBuilder& builder) = 0;

		HAL::CommandListType get_type() const;
		void compile(TaskBuilder& builder);

		virtual void render(Graph*graph, HAL::FrameResources::ptr& frame) = 0;
		void wait();
		void execute();

		bool active();

		// optimization
		bool inserted = false;

		// Clears every per-frame-mutable member so a cached Pass instance can be
		// reused next frame instead of reallocated. id/name/setup_func/render_func
		// are reassigned separately by internal_pass right after this call.
		void reset_frame();
	};


	template <class Handler>
	struct TypedPass : public Pass
	{
		using HandlerType = Handler;

		using render_func_type = std::function<void(Handler&, FrameContext&)>;
		using setup_func_type = std::function<bool(Handler&, TaskBuilder&)>;
		using setup_func_type_void = std::function<void(Handler&, TaskBuilder&)>;


		Handler data;

		setup_func_type setup_func;
		render_func_type render_func;

		TypedPass(int id, LiteralWStr name, setup_func_type s, render_func_type r)
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

		virtual void render(Graph*graph,  HAL::FrameResources::ptr& frame) override
		{
			if (!enabled || !renderable)  return;



			render_task = thread_pool::get().enqueue([this, &frame, graph](){
				context.begin(const_cast<Graph*>(graph), this, frame);
				render_func(data, context);
				context.end();
				});

		//		render_task.wait();
		}
	};



	// Fake pass that appears as the creator of resources passed in from outside the graph.
	// Submits an empty command list so FrameContext::end() fires process_debug_resource
	// for each passed resource, enabling thumbnail capture in the timeline debugger.
	struct ExternalPass : public Pass
	{
		inline static const LiteralWStr s_name = L"[External]";

		ExternalPass();

		virtual bool setup(TaskBuilder&) override;

		virtual void render(Graph* graph, HAL::FrameResources::ptr& frame) override;
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
				setter.set(compiled);
			};
		}

		void set_slot(SlotID id, HAL::SignatureDataSetter& setter);

	};

	
	class Graph: public UniversalContext, public VariableContext, public SlotContext
	{
	public:

		// One persistent indirect-dispatch argument buffer per command list type.
		// X is overwritten each frame via copy_buffer; Y and Z are initialized once to 1.
		enum_array<HAL::CommandListType, HAL::StructuredBufferView<DispatchArguments>> indirect_dispatch_args;

		Events::Event<const Graph&> on_compile;
	
		Variable<bool> optimize = { true, "optimize", this };

		std::list<std::function<void(Graph& g)>> pre_run;
		template<class PassT>
		void internal_pass(LiteralWStr name, auto s, auto r, PassFlags flags = PassFlags::General, uint32_t index = 0, PassID type_id = PassID::Count)
		{
			PROFILE(name);

			std::shared_ptr<Pass> pass_ptr;

			if (type_id != PassID::Count)
			{
				auto& slots = builder.pass_cache[(size_t)type_id];
				if (slots.size() <= index)
					slots.resize(index + 1);

				auto& slot = slots[index];
				if (!slot)
				{
					slot = std::make_shared<PassT>((UINT)builder.passes.size(), name, s, r);
				}
				else
				{
					slot->reset_frame();
					auto* typed = static_cast<PassT*>(slot.get());
					// Fresh Handler each frame — matches what a brand-new PassT would
					// have, so fields this frame's setup_func doesn't touch don't leak
					// last frame's resolved values instead of their declared defaults.
					typed->data = typename PassT::HandlerType{};
					typed->setup_func = s;
					typed->render_func = r;
					slot->id = (UINT)builder.passes.size();
					slot->name = name;
				}
				pass_ptr = slot;
			}
			else
			{
				pass_ptr = std::make_shared<PassT>((UINT)builder.passes.size(), name, s, r);
			}

			pass_ptr->flags = flags;
			pass_ptr->pass_index = index;
			pass_ptr->type_id = type_id;

			builder.passes.push_back(pass_ptr);

			if (check(flags & PassFlags::Required))
			{
				builder.required_passes.push_back(pass_ptr);
			}
		}

	public:

		Graph();
		TaskBuilder builder;

		void set_pipeline(Pipelines::PipelineBase* p);
		Pipelines::PipelineBase* get_pipeline() const;

	private:
		Pipelines::PipelineBase* current_pipeline = nullptr;
	public:



		template<class T>
		void add_pass(LiteralWStr name, typename TypedPass<T>::setup_func_type s, typename TypedPass<T>::render_func_type r, PassFlags flags = PassFlags::General)
		{
			internal_pass<TypedPass<T>>(name, s, r, flags);
		}


		template<class T>
		void add_library_pass(typename T::setup_func_type s, typename T::render_func_type r, PassFlags flags = PassFlags::General)
		{
			internal_pass<TypedPass<T::Context>>(T::Name, s, r, flags, 0, T::ID);
		}

		template<class T>
		void add_library_pass(uint32_t index, typename T::setup_func_type s, typename T::render_func_type r, PassFlags flags = PassFlags::General)
		{
			internal_pass<TypedPass<T::Context>>(T::Names[index], s, r, flags, index, T::ID);
		}



		/*template<class T>
		void add_pass(LiteralWStr name, typename TypedPass<T>::setup_func_type_void s, typename TypedPass<T>::render_func_type r, PassFlags flags = PassFlags::General)
		{
			typename TypedPass<T>::setup_func_type f = [s](auto& t, auto& b) {s(t, b); return true; };

			internal_pass<TypedPass<T>>(name, f, r, flags);
		}


		template<class T>
		void add_pass2(LiteralWStr name, typename TypedPass<T>::setup_func_type_void s, typename TypedPass<T>::render_func_type r, PassFlags flags = PassFlags::General)
		{
			typename TypedPass<T>::setup_func_type f = [s](auto& t, auto& b) {s(t, b); return true; };

			internal_pass<TypedPass<T>>(name, f, r, flags);
		}*/


		void add_slot_generator(std::function<void(Graph&)> f)
		{
			pre_run.push_back(f);
		}
		void start_new_frame();

		void setup();
		void compile(int frame);
		void render();


		HAL::FenceWaiter commit_command_lists();
		void reset();

	};



	class GraphGenerator
	{
	public:
		virtual void generate(Graph& graph) = 0;
	};


}
