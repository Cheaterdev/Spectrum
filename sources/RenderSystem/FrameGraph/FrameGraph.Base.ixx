#include "autogen/resource_ids.h"
#include "autogen/pass_ids.h"

export module FrameGraph:Base;

import Core;
import HAL;


using namespace HAL;


// Precompiled-graph structs, defined early so PipelineBase can expose them.
// They only depend on ResourceID / PassID from the includes above.
export namespace FrameGraph
{
	// A pass's static declaration of one resource it touches, from the SIG
	// PassNode block: the resource id plus whether the pass declares [Write] on
	// it. Emitted per-pass as Context::resource_accesses[].
	struct ResourceAccess
	{
		ResourceID id;
		bool       write;
	};

	// A reference to a pass in precompiled state data: the pass type plus, for
	// [Multiple] passes, which instance (0 otherwise).
	struct PassRef
	{
		PassID   id;
		uint32_t index;
	};

	// One precomputed RW-state in a pipeline's per-resource timeline (mirrors a
	// runtime ResourceRWState): whether it's a write, and the passes in it.
	struct PrecompiledState
	{
		bool                     write;
		std::span<const PassRef> passes;
	};

	// A pipeline's precomputed timeline for one resource version: the ordered
	// states it passes through. Emitted per-pipeline as resource_infos[]. A
	// recreate() produces a new chain version of the same id — chain_index
	// selects which ResourceAllocInfo in the resource's chain this timeline maps
	// to (0 = the original create). This is a template — at runtime disabled
	// passes are pruned and cross-pipeline timelines concatenated in add order.
	struct PrecompiledResourceInfo
	{
		ResourceID                       id;
		uint32_t                         chain_index;
		std::span<const PrecompiledState> states;
	};

	// A precomputed pass instance in a pipeline: its type, [Multiple] instance,
	// queue (compute vs direct/graphics), and its prev-pass dependency edges —
	// derived at codegen time from the per-resource state timelines (the static
	// equivalent of resolve_dependencies). Emitted per-pipeline as
	// precompiled_passes[]. Also a template: disabled passes are pruned at
	// runtime and cross-pipeline edges resolved at add time.
	struct PrecompiledPass
	{
		PassID                   id;
		uint32_t                 index;    // [Multiple] instance (0 otherwise)
		bool                     compute;  // queue: compute if true, else direct/graphics
		std::span<const PassRef> prev_passes;
	};
}


namespace Pipelines
{

class PipelineBase
{
public:
	virtual ~PipelineBase() = default;
	virtual std::span<const wchar_t* const> GetUsedPassNamesList() const = 0;
	virtual std::span<const wchar_t* const> GetUsedResourcesList() const = 0;

	// Precomputed per-resource RW-state timelines and per-pass dependency data.
	virtual std::span<const FrameGraph::PrecompiledResourceInfo> GetResourceInfos() const = 0;
	virtual std::span<const FrameGraph::PrecompiledPass>         GetPrecompiledPasses() const = 0;
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

	// Which image the UI composites onto the swapchain. Anything other than
	// Final makes UI_Render sample the DebugResult texture instead of the final
	// ResultTexture. Read/set via graph.get_context<DebugContext>().
	enum class DebugMode
	{
		Final,
		Albedo,
		Motion,
		GI_Indirect,
		GI_Reflection,
		VoxelTrace,
		RTX
	};

	struct DebugContext
	{
		DebugMode mode = DebugMode::Final;
	};

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
		// Thread-safe append (populated concurrently in add_pass); all reads
		// happen single-threaded after the setup/append phase joins.
		concurrent_vector<Pass*> passes;

		HAL::SubResourcesGPU merged_read_state;

		SyncState from;
		SyncState to;
	};

	// A resource version's RW-state timeline. Cursor-based like ResourceChain:
	// the state slots — and each slot's passes / merged_read_state buffers —
	// persist across frames. reset_frame() only rewinds the cursor and push()
	// reuses a slot (clearing passes keeps its storage, so reserve() is a no-op
	// once it's large enough), so we don't reallocate the per-state vectors
	// every frame. Exposes a vector-like read interface for the many consumers.
	struct StateTimeline
	{
		std::vector<ResourceRWState> items;
		size_t count = 0;

		void reset_frame() { count = 0; }

		bool   empty() const { return count == 0; }
		size_t size()  const { return count; }

		ResourceRWState&       operator[](size_t i)       { return items[i]; }
		const ResourceRWState& operator[](size_t i) const { return items[i]; }
		ResourceRWState&       front()       { return items[0]; }
		const ResourceRWState& front() const { return items[0]; }
		ResourceRWState&       back()        { return items[count - 1]; }
		const ResourceRWState& back()  const { return items[count - 1]; }

		ResourceRWState*       begin()       { return items.data(); }
		ResourceRWState*       end()         { return items.data() + count; }
		const ResourceRWState* begin() const { return items.data(); }
		const ResourceRWState* end()   const { return items.data() + count; }

		// Append a new state, reusing a persisted slot's storage when available.
		ResourceRWState& push(bool write, size_t reserve_n)
		{
			if (count >= items.size())
				items.emplace_back();
			ResourceRWState& s = items[count++];
			s.write = write;
			s.passes.clear();            // keeps the concurrent_vector's storage
			s.passes.reserve(reserve_n); // no-op once large enough
			s.from.reset();
			s.to.reset();
			s.merged_read_state.subres.clear();
			return s;
		}

		// Stable removal that keeps removed slots' storage in items for reuse.
		template<class Pred>
		void remove_if(Pred pred)
		{
			size_t w = 0;
			for (size_t r = 0; r < count; ++r)
				if (!pred(items[r]))
				{
					if (w != r) std::swap(items[w], items[r]);
					++w;
				}
			count = w;
		}
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
		StateTimeline states;
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

		// --- History (previous-frame) support -------------------------------
		// Role flags for a resource participating in a history link (set in init()).
		// A `current` allocates fresh each frame but its allocation is NOT freed this
		// frame — it's carried one frame forward to become next frame's `prev`. A
		// `prev` doesn't allocate: it adopts the carried resource+allocation and gets
		// a freshly-created view every frame (views are frame-linked). See
		// create_resources() and roll_history().
		bool is_history_prev    = false;
		bool is_history_current = false;


				Events::Event<Pass*, FrameContext*>	process_debug_resource;




		void add_pass(Pass* pass, ResourceFlags flags);
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
		// Transient memory is recycled within a frame, and a range may be handed to a
		// consumer on a different queue than its previous owner. SyncAwareAllocator
		// tags each free block with the point its previous owner finished and refuses
		// to hand it to a requester that is not ordered after that, relocating the
		// allocation instead of needing a cross-queue barrier.
		using AllocatorType = SyncAwareAllocator;
		using LockPolicy = Thread::Free;
	};

	struct ResourceChain
	{
		bool   empty() const { return items.empty(); }
		size_t size()  const { return items.size(); }

		ResourceAllocInfo&       active()       { return items[pos]; }
		const ResourceAllocInfo& active() const { return items[pos]; }

		// Number of chain versions active this frame (0 if not created), and
		// direct access to a specific version. Used to map a precomputed
		// resource timeline's chain_index to its ResourceAllocInfo.
		size_t             active_count() const { return created_this_frame ? pos + 1 : 0; }
		ResourceAllocInfo& at(size_t i)         { return items[i]; }

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

		// True only if create() ran for this resource in the current frame.
		// Distinct from empty()/size(), which stay "non-empty" forever once any
		// earlier frame created the resource — that weaker check let exists()/
		// need() silently operate on stale, un-reset state on frames where
		// create() was skipped (see UI_PreDraw_Sync staleness bug).
		bool created_this_frame = false;

		void reset_frame() { pos = 0; created_this_frame = false; }

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

		// One physical allocation carried across a frame boundary: last frame's
		// `current`, reserved (not freed) to serve as this frame's `prev`.
		struct HistorySlot
		{
			HAL::Resource::ptr   resource;
			HAL::ResourceHandle  alloc_ptr;
			HAL::ResourceDesc    desc;
			HAL::SubResourcesGPU last_state;

			bool valid() const { return !!resource; }
		};

		// History link {current, prev}. `carried` holds current's allocation from
		// the previous frame; create_resources() binds it into `prev` (fresh view),
		// and roll_history() frees the consumed one and carries this frame's current.
		//
		// `pending_release` delays the consumed slot's heap-range free by one
		// extra frame: prev's last readers run on the async compute queue, and
		// freeing immediately lets next frame's direct-queue transients adopt
		// the range with no fence edge against that compute tail (unlike
		// normal transients, whose frees go through the sync-aware deletion
		// scheduling) — the range could be stomped while still being read,
		// blending garbage into the history chain.
		struct HistoryLink
		{
			ResourceID  current = ResourceID::Count;
			ResourceID  prev    = ResourceID::Count;
			HistorySlot carried;
			HistorySlot pending_release;
		};
		std::vector<HistoryLink> history_links;

		// Register `prev` as the previous-frame alias of `current`. Idempotent;
		// safe to call every frame from a pass setup.
		void link_history(ResourceID current, ResourceID prev);

		HistoryLink* history_by_current(ResourceID id);
		HistoryLink* history_by_prev(ResourceID id);

		// End-of-frame: free each link's consumed carried slot and carry this
		// frame's current allocation forward. Called in commit_command_lists()
		// before the normal transient free pass.
		void roll_history();

		// Compact list of resources enabled this frame, populated during the
		// enable-marking pass in Graph::setup(). Lets later passes (create_resources)
		// iterate only what matters instead of rescanning all of alloc_resources.
		std::vector<ResourceAllocInfo*> enabled_resources;

		// Builds each resource's RW-state timeline after all setups have run —
		// from the added pipelines' precomputed resource_infos (recreate chains
		// fall back to add_pass). Setup itself only records resource desc/flags
		// and what each pass touched (used.resources / used.resource_flags).
		void build_resource_states();

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

		// Epoch handed to the transient allocator. Bumped once per create_resources,
		// so everything released in a previous frame is unconstrained — justified by
		// the frame-boundary cross-queue wait in commit_command_lists. Per-builder
		// rather than global: the inference belongs to THIS graph's frame boundary.
		uint64 alias_epoch = 0;
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
			chain.created_this_frame = true;

			ResourceAllocInfo& info = chain.active();
			info.id = result.id;
			T& handler = info.create_handler<T>(desc);
			init(info, flags);
			result = handler;
			result.id = info.id;

			// If this resource is a history `current`, provision its linked `prev`
			// here — derived from current's desc, marked existing so consumers can
			// need() it, but created by NO pass (no write state, no dependency, no
			// alias barriers). prev only ever adopts current's carried allocation.
			if (HistoryLink* link = history_by_current(result.id))
				provision_history_prev<T>(link->prev, desc, flags);
		}

		// Bind a caller's local handler to the already-provisioned prev chain, so
		// dereferencing it (e.g. in a GBuffer actualize view) resolves the adopted
		// view. Needed because provision sets up the chain, not the caller's handle.
		template<class T>
		void bind_history_prev(T& result)
		{
			auto& chain = alloc_resources[(size_t)result.id];
			ASSERT(chain.created_this_frame); // provisioned by create(current) already
			ResourceAllocInfo& info = chain.active();
			result    = info.get_handler<T>();
			result.id = info.id;
		}

		template<class T>
		void provision_history_prev(ResourceID prev_id, const typename T::Desc& desc, ResourceFlags flags)
		{
			auto& chain = alloc_resources[(size_t)prev_id];
			chain.reset_frame();
			chain.ensure_first();
			chain.created_this_frame = true;

			ResourceAllocInfo& info = chain.active();
			info.id = prev_id;
			info.create_handler<T>(desc);
			// Deliberately NOT init(): init() registers the resource against
			// current_pass and gives it a creation/write state. prev has no creator.
			info.reset(passes.size());
			info.flags              = flags;
			info.frame_id           = current_frame->get_frame();
			info.is_history_prev    = true;
			info.is_history_current = false;
		}

		template<class T>
		void recreate(T& result, ResourceFlags flags = ResourceFlags::None)
		{
			auto& chain = alloc_resources[(size_t)result.id];
			ASSERT(chain.created_this_frame);
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
			ASSERT(chain.created_this_frame);
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
			return chain.created_this_frame;
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

		// Mirror commit_command_lists' batching to find the sets of lists that
		// will be submitted in ONE ExecuteCommandLists, and chain
		// resource state across the boundaries inside each set. Must run after
		// process_transitions (all usages recorded) and process_fences (which
		// decides where batches are flushed), and before compile_lists.
		void link_list_groups();
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

		// Static [Write]/read declaration from the pass's SIG (empty for passes
		// whose Context has no resource_accesses, e.g. custom add_pass<T>).
		virtual std::span<const ResourceAccess> declared_accesses() const { return {}; }

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

		virtual std::span<const ResourceAccess> declared_accesses() const override
		{
			if constexpr (requires { Handler::resource_accesses; Handler::resource_count; })
				return { Handler::resource_accesses, Handler::resource_count };
			else
				return {};
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

		// Each queue's final fence of the previous frame — waited on by the
		// OTHER queues before this frame's first submission, so re-placed
		// transient heap ranges can't race the prior frame's async tail (see
		// commit_command_lists).
		enum_array<HAL::CommandListType, HAL::FenceWaiter> prev_frame_end_fence;

		Events::Event<const Graph&> on_compile;
	
		Variable<bool> optimize = { true, "optimize", this };

		// Diagnostic: serialize all queues (fence after EVERY pass, all other
		// queues wait) — kills any cross-queue concurrency. Garbage that
		// survives this is not an async/lifetime race but a rogue writer
		// (out-of-bounds dispatch, wrong descriptor, stale indirect args).
		Variable<bool> serialize_queues = { false, "serialize_queues", this };

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
			pass_ptr->wait_pass = nullptr;
			pass_ptr->prev_pass = nullptr;

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

		// All pipelines added this frame, in add order (for concatenating their
		// precomputed per-resource timelines). Cleared in reset().
		std::vector<Pipelines::PipelineBase*> added_pipelines;

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
