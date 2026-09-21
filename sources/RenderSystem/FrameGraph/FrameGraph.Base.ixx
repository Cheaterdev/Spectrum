#include "autogen/resource_ids.h"
#include "autogen/pass_ids.h"
// Per-pass Table:: context-field dependency masks, derived from the parsed
// [SetupCondition]/[RenderCondition]/[Optional] expressions. Pulls in <bitset>
// itself; it must therefore stay in this global module fragment rather than
// move below `export module`, where a #include would attach to the module.
#include "autogen/context_deps.h"

export module FrameGraph:Base;

import Core;
import HAL;
import :UpscalerViewport;


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

		bool operator==(const PassRef&) const = default;
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
		// Was PixelRead/ComputeRead/DSRead/CopySource -- four names for the
		// same thing. Nothing in the barrier/RW-state timeline (ResourceAllocInfo::add_pass)
		// or anywhere else ever branched on which one was used, only on
		// whether a flag was in WRITEABLE_FLAGS or was ExclusiveRead; the
		// four-way split was purely documentation that cost every read call
		// site a choice with no behavioral difference. CopySource in
		// particular had zero real call sites anywhere in RenderSystem.
		Read = (1 << 1),

		UnorderedAccess = (1 << 4),
		RenderTarget = (1 << 5),
		DepthStencil = (1 << 6),
		CopyDest = (1 << 7),


	//	GenCPU = (1 << 9),
	//	ReadCPU = (1 << 10),

		Temporal = 0,
		Static = (1 << 11),
		Required = (1 << 12),

		// Marks a read that must not be folded into a neighboring canonical-read
		// window — for passes that manage their own transition to a layout other
		// passes never request (e.g. PRESENT/COMMON for a Streamline hand-off).
		ExclusiveRead = (1 << 14)
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
		VoxelTrace,
		RTX,
		// REBLUR-denoised RTX indirect GI (NRD_REBLUR_Execute, see
		// [[project-nrd-integration]]). Raw = packed (YCoCg + normalized
		// hit-distance, matches what RTXCombine actually consumes);
		// Unpacked = true-color debug preview.
		RTXIndirectDenoised,
		RTXIndirectDenoisedUnpacked,
		// NRD_GBufferPack's own outputs, for inspecting its front-end
		// packing directly (see [[project-nrd-integration]]).
		NRDViewZ,
		NRDNormalRoughness,
		// Raw depth, bypassing NRD_GBufferPack's shader entirely -- to
		// isolate whether GBuffer_DepthMips itself has real content by the
		// time this pass runs, vs. a compute-shader-specific read issue.
		RawDepthMips,
		// DDGI probe-volume indirect term (see [[project-ddgi]] planning
		// notes) -- full-screen ddgi_sample_irradiance() at every pixel's
		// real surface normal, isolated from direct lighting. Distinct from
		// DDGISelectors::show_probes (the discrete probe-marker overlay,
		// which composites onto ResultTexture directly rather than going
		// through this DebugMode switch).
		DDGIIndirect
	};

	// DebugContext lives further down, after Handlers:: (it holds a
	// Handlers::Texture).

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

	public:
		friend struct TaskBuilder;
		friend struct FrameContext;
		ResourceAllocInfo* info = nullptr;

		virtual void init(ResourceAllocInfo& info) = 0;

		virtual void init_view(ResourceAllocInfo& info, GPUEntityStorageInterface& frame) = 0;

		// Raw access to the handler's Desc, so LoadGraph can copy one link's desc
		// to the next without knowing the concrete Desc type. Safe because a
		// resource's handler type is fixed by its .sig declaration and never
		// changes for a given ResourceID.
		virtual void*  desc_data() = 0;
		virtual size_t desc_size() const = 0;
	};





	struct Pass;

	// Fatal diagnostic for TaskBuilder::need() finding a resource its own
	// chain hasn't created this frame -- see that call site's own comment for
	// why this exists (the ASSERT it replaced is a no-op outside DEV builds).
	// Out-of-line because `Pass` is only forward-declared at need()'s point
	// in this file; defined in FrameGraph.cpp, which sees Pass's full
	// definition.
	[[noreturn]] void report_resource_not_created(ResourceID id, Pass* requesting_pass);

	// Identifies one chain link of a resource -- the pair every pointer-keyed
	// container is being converted to, and the form a serialized graph plan
	// stores.
	//
	// NOT a bare ResourceID: [Recreate] gives one id several chain links, and a
	// pass legitimately touches two of them in the same frame (reads the old,
	// writes the new). FSR/SMAA/UpscalingDLSS/UpscalingDLSSRR all do this to
	// ResultTexture, so collapsing to the id alone would silently merge a read
	// state with a write state and destroy the ordering between them.
	//
	// Stable across frames: ResourceChain::reset_frame() rewinds pos but never
	// clears items, so link N is the same object next frame. That is what makes
	// this usable as a persisted key -- given the same generated ID space, which
	// a plan header must check, since editing a .sig renumbers ResourceID.
	struct ResourceVersion
	{
		ResourceID id      = ResourceID::Count;
		uint32_t   version = 0;   // chain index; 0 = the original create()

		auto operator<=>(const ResourceVersion&) const = default;
	};

	// A handler field's chain link as stored in a pass's generated Cache.
	export enum class ChainIndex : uint32_t { Unresolved = ~0u };

	struct UsedResources
	{
		std::list<HAL::FenceWaiter> fences;

		struct Access
		{
			ResourceVersion version;
			ResourceFlags   flags;
		};

		// Flat vectors keyed by ResourceVersion rather than sets/maps keyed by
		// ResourceAllocInfo*. Three reasons: a pass touches a handful of the 89
		// resources so a linear scan over contiguous memory beats a red-black
		// tree and leaves no per-frame node churn to tear down; iteration order
		// becomes deterministic instead of heap-address order; and the contents
		// are directly serializable into a stored graph plan.
		std::vector<Access>          resources;
		std::vector<ResourceVersion> resource_creations;
		std::vector<ResourceVersion> resource_deletions_before;
		std::vector<ResourceVersion> resource_deletions_after;

		// Records one access. Latest access wins, EXCEPT Required, which is
		// carried forward.
		//
		// Capability flags deliberately do not accumulate here: this value feeds
		// ResourceAllocInfo::add_pass(), i.e. the RW-state timeline, so OR-ing a
		// stale UnorderedAccess into a later plain read would claim a UAV state
		// the pass never asks for. Required is different -- it is not a
		// capability but a "do not cull this" marker, and dropping it on a
		// second access would let the resource be culled out from under the
		// first one.
		//
		// NOT the same accumulation as ResourceAllocInfo::flags, which does OR
		// everything and drives the D3D12 resource desc. That one is reset each
		// frame by create(), so it already reflects only the passes that ran.
		void touch(ResourceVersion v, ResourceFlags flags)
		{
			for (auto& a : resources)
				if (a.version == v)
				{
					a.flags = flags | (a.flags & ResourceFlags::Required);
					return;
				}
			resources.push_back({ v, flags });
		}

		// The three lists below stood in for std::set, so they keep its
		// set-like "insert once" behaviour.
		static void add_unique(std::vector<ResourceVersion>& v, ResourceVersion x)
		{
			for (auto e : v) if (e == x) return;
			v.push_back(x);
		}
		static bool contains(const std::vector<ResourceVersion>& v, ResourceVersion x)
		{
			for (auto e : v) if (e == x) return true;
			return false;
		}
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
		// True for a state forced open by ResourceFlags::ExclusiveRead — always
		// a singleton, excluded from process_transitions()'s merge-back.
		bool exclusive = false;
		// Passes in this state, by (PassID, instance) rather than by pointer.
		//
		// Same move as ResourceVersion for resources: an ID is stable, orders
		// deterministically, and serializes directly into a stored plan -- which
		// removes the Pass*<->PassRef translation DumpGraph/LoadGraph used to do
		// on every recorded state. Resolve with TaskBuilder::get_pass().
		//
		// Thread-safe append (populated concurrently in add_pass); all reads
		// happen single-threaded after the setup/append phase joins.
		concurrent_vector<PassRef> passes;

		SyncState from;
		SyncState to;
	};

	// A resource version's RW-state timeline. Cursor-based like ResourceChain:
	// the state slots — and each slot's passes buffers —
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
			s.exclusive = false;
			s.passes.clear();            // keeps the concurrent_vector's storage
			s.passes.reserve(reserve_n); // no-op once large enough
			s.from.reset();
			s.to.reset();
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

		// This info's index within its ResourceChain; 0 for the original create().
		// Together with `id` this is its ResourceVersion -- the identity used by
		// pass containers and by a serialized plan, in place of the pointer.
		uint32_t chain_index = 0;
		const char* name() const;
		// desc
		//ResourceType type;
		//MyVariant desc;
		ResourceFlags flags;


		HAL::ResourceHandle alloc_ptr;

		HAL::ResourceDesc d3ddesc;
		// Initialised, because not every path assigns it: create_resources() skips
		// passed resources, and these slots are reused across frames, so an
		// uninitialised value is indeterminate rather than merely wrong. Anything
		// that filters on heap_type would then drop resources at random.
		HAL::HeapType heap_type = HAL::HeapType::DEFAULT;
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

		// This link's desc was copied from the previous link (a [Recreate]
		// without its own [Size]/[Format]) rather than computed from context.
		// Replay has to repeat the copy after every pass has loaded, because the
		// previous link belongs to another pass.
		bool desc_from_prev = false;


				Events::Event<Pass*, FrameContext*>	process_debug_resource;




		void add_pass(Pass* pass, ResourceFlags flags);
		void reset(size_t max_passes);

		// Takes the builder because filtering needs to resolve each PassRef back
		// to its Pass to ask whether it is enabled/active.
		void remove_inactive(TaskBuilder& builder);

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

			// Which chain link this handler last resolved to. Today it is implied
			// by call order -- create() takes the active link, recreate() advances
			// pos and takes the next -- which works only because setup() runs one
			// pass after another.
			//
			// Recorded here so resolution can stop depending on that order: a
			// replayed frame resolves straight to builder.get({id, chain_index}),
			// which is what lets the resolve half of setup run per-pass in
			// parallel. Matters specifically for [Recreate], where two fields of
			// one pass (ResultTexture / ResultTextureNew) share an id and differ
			// only by this.
			uint32_t chain_index = 0;

			ResourceVersion version() const { return { id, chain_index }; }

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

				// "The FrameGraph owns this resource's transitions" is now
				// Resource::frame_graph_managed, set on the resource itself in
				// create_resources -- no longer a creation-desc flag.
			}

			virtual void init_view(ResourceAllocInfo& info,GPUEntityStorageInterface& frame) override
			{
				info.init_view<View>(frame, desc.as_view(info.offset_in_bytes, info.flags));
			}

			virtual void*  desc_data() override      { return &desc; }
			virtual size_t desc_size() const override { return sizeof(Desc); }
		};

		struct ByteBufferDesc
		{

			using View = HAL::ByteBufferView;
			uint64 count;

			HAL::ResourceDesc create_resource_desc(ResourceFlags resflags) const;

			ByteBufferViewDesc as_view(uint64 offset, ResourceFlags resflags) const;
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
				// Must mirror create_resource_desc()'s own padding exactly:
				// StructuredBufferView::init() (for counterType::SELF) does
				// `size -= local_offset` on whatever size this desc reports,
				// on the assumption that it already includes the counter's
				// padding -- true for the resource this same struct just
				// created via create_resource_desc(), but this used to
				// report the UNPADDED count*sizeof(T) instead, silently
				// under-sizing the view by exactly one element (confirmed
				// live: a counted list created for exactly N tiles only ever
				// exposed N-1 to the append shader, dropping whichever
				// append happened to land on the boundary -- a different
				// screen tile each frame depending on GPU dispatch
				// scheduling, since nothing about WHICH append lands last is
				// deterministic).
				uint64 size = count * sizeof(Underlying<T>);
				if (counted)
					size += Math::roundUp(4, sizeof(Underlying<T>));
				return { offset, size, counted?counterType::SELF:counterType::NONE };
			}
		};


		struct TextureDesc
		{

			using View = HAL::Texture2DView;
			ivec3 size;
			HAL::Format format;
			UINT array_count;
			UINT mip_count;

			// mip_count == 0 means "full chain". Resolving it returns the count
			// instead of writing it back into this desc: create_resource_desc used
			// to mutate mip_count in place, which made the desc a hidden state
			// machine -- 0 before the first create_resources(), the resolved count
			// after. Nothing noticed while every frame recomputed it, but a cached
			// desc then restored the pre-resolution 0 and as_view() built a view
			// over zero mips.
			UINT resolved_mips() const;

			HAL::ResourceDesc create_resource_desc(ResourceFlags resflags) const;

			HAL::TextureViewDesc as_view(uint64 offset,ResourceFlags resflags) const;
		};

		struct Texture3DDesc
		{

			using View = HAL::Texture3DView;
			ivec3 size;
			HAL::Format format;
			UINT mip_count;

			// See TextureDesc::resolved_mips().
			UINT resolved_mips() const;

			HAL::ResourceDesc create_resource_desc(ResourceFlags resflags) const;

			HAL::Texture3DViewDesc as_view(uint64 offset,ResourceFlags resflags) const;
		};


		struct CubeDesc
		{

			using View = HAL::CubeView;
			ivec3 size;
			HAL::Format format;
			UINT array_count;
			UINT mip_count;

			// See TextureDesc::resolved_mips().
			UINT resolved_mips() const;

			HAL::ResourceDesc create_resource_desc(ResourceFlags resflags) const;

			HAL::CubeViewDesc as_view(uint64 offset,ResourceFlags resflags) const;
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

	struct DebugContext
	{
		DebugMode mode = DebugMode::Final;

		// No result_texture handle any more: UI_Render declares ResultTexture as
		// an ordinary field and GUI/Base.cpp's create_graph redirects it with
		// Graph::override_resource when `mode` is not Final. Only the selector
		// itself needs to live on the graph now.
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
				items.emplace_back().chain_index = 0;
		}

		// Advance to the next slot, growing the deque if needed.
		ResourceAllocInfo& emplace_back()
		{
			++pos;
			if (items.size() <= pos)
				items.emplace_back();
			// Stamped once, when the link first appears. items is never cleared
			// (reset_frame only rewinds pos), so this stays correct for the
			// lifetime of the chain.
			items[pos].chain_index = (uint32_t)pos;
			return items[pos];
		}

		// True only if create() ran for this resource in the current frame.
		// Distinct from empty()/size(), which stay "non-empty" forever once any
		// earlier frame created the resource — that weaker check let exists()/
		// need() silently operate on stale, un-reset state on frames where
		// create() was skipped (see UI_PreDraw_Sync staleness bug).
		bool created_this_frame = false;

		void reset_frame() { pos = 0; created_this_frame = false; }

		// Restores the active link when replaying a plan, instead of arriving at
		// it through a sequence of create()/recreate() calls.
		void set_pos(size_t p) { pos = p; }

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
		// and what each pass touched (used.resources).
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

		// Redirects one of a pass's DECLARED resource fields to a different
		// resource for this frame -- the debug-view selector is the only user
		// (GUI/Base.cpp): UI_Render declares ResultTexture statically and an
		// override points it at a GBuffer target instead.
		//
		// This replaced a [NeedDynamic] option that resolved the resource id
		// from a context every frame. The difference that matters is not
		// ergonomics: a runtime-chosen ResourceID cannot be represented in a
		// pass's context-field dependency mask (autogen/context_deps.h), so it
		// was an input to graph structure that no cache key could ever see.
		// As an override it becomes an explicit, enumerable condition instead --
		// see has_resource_overrides().
		struct ResourceOverride
		{
			PassID     pass;
			ResourceID from;
			ResourceID to;
		};

		std::vector<ResourceOverride> resource_overrides;

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
			result.chain_index = info.chain_index;

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
			result.chain_index = info.chain_index;
		}

		// bind_history_prev(), plus a check that `result` really is the linked
		// prev of `current` -- call right after create(current). The link
		// itself is normally registered by a [PrevFor=...]-generated
		// link_history_always() (run before setup_func, see TypedPass::setup()),
		// not by a hand-written link_history() call at this point; this just
		// verifies that registration actually points at `current` before
		// resolving the handle, catching a mismatched current/prev pair here
		// instead of silently binding the wrong chain.
		template<class T>
		void create_prev(T& result, T& current)
		{
			HistoryLink* link = history_by_current(current.id);
			ASSERT(link && link->prev == result.id);
			bind_history_prev(result);
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
			info.desc_from_prev = true;
			result = handler;
			result.id = info.id;
			result.chain_index = info.chain_index;
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
			result.chain_index = info.chain_index;
		}

		template<class T>
		bool exists(T& result)
		{
			auto& chain = alloc_resources[(size_t)result.id];
			return chain.created_this_frame;
		}

		// Which resource the CURRENT pass actually reads for one of its declared
		// fields, after any active override. See TaskBuilder::resource_overrides.
		// Defined in FrameGraph.cpp: it reads Pass::type_id, and Pass is still
		// incomplete here.
		ResourceID resolve_override(ResourceID declared) const;

		// Generated Cache support (save_to_cache / load_from_cache in each pass
		// header). Loading reads only the restored chains and writes only the
		// calling pass's own Context, so passes can load in any order or at once.
		template<class T>
		static ChainIndex cache_slot(const T& handler, ResourceID declared)
		{
			if (!handler)
				return ChainIndex::Unresolved;
			// Replay looks the link up under the field's declared id. A handler
			// resolved to a different resource can only come from
			// override_resource, and an overridden frame is never recorded.
			ASSERT(handler.info->id == declared);
			return (ChainIndex)handler.info->chain_index;
		}

		// Points a Context field at its link. Deliberately does NOT copy the
		// link's handler: another pass may be writing that handler's desc in
		// create_versioned at the same time, and render code only goes through
		// info->view anyway.
		template<class T>
		void load(T& result, ResourceID id, ChainIndex index) const
		{
			if (index == ChainIndex::Unresolved)
				return;
			ResourceAllocInfo* info = get(ResourceVersion{ id, (uint32_t)index });
			ASSERT(info && info->handler);
			result.info        = info;
			result.id          = id;
			result.chain_index = (uint32_t)index;
		}

		// Replay counterpart of create()/recreate(desc): recomputes the desc of
		// a link only the calling pass creates. Flags and states were already
		// restored from the plan, and d3ddesc is derived from the desc later in
		// create_resources, so the desc is all that is left to write.
		template<class T>
		void create_versioned(T& result, ChainIndex index, const typename T::Desc& desc) const
		{
			if (index == ChainIndex::Unresolved)
				return;
			ResourceAllocInfo* info = get(ResourceVersion{ result.id, (uint32_t)index });
			ASSERT(info && info->handler);
			info->get_handler<T>().desc = desc;

			// A history current's prev takes the same desc, as
			// provision_history_prev does live. The prev chain is written only by
			// its current's creator, so this stays within the calling pass.
			for (const auto& link : history_links)
				if (link.current == result.id)
					if (ResourceAllocInfo* prev = get(ResourceVersion{ link.prev, 0 }); prev && prev->handler)
						prev->get_handler<T>().desc = desc;

			load(result, result.id, index);
		}

		template<class T>
		void need(T& result, ResourceFlags flags = ResourceFlags::None)
		{
			// Overrides are rare (only an active debug-view selection registers
			// one), so the common path below is left exactly as it was rather
			// than folding the redirect into it -- no extra work, and nothing
			// about the un-overridden graph can shift.
			if (!resource_overrides.empty())
			{
				const ResourceID declared       = result.id;
				const uint32_t   declared_chain = result.chain_index;
				ResourceID effective = resolve_override(declared);

				if (effective != declared)
				{
					// An override naming a resource no pass produced this frame
					// falls back to the declared one rather than failing the
					// graph build. The debug views this exists for are genuinely
					// conditional -- the NRD-denoised targets don't exist under
					// DLSS-RR -- and showing the normal image beats asserting
					// because a view was picked that this configuration never
					// renders.
					if (alloc_resources[(size_t)effective].created_this_frame)
					{
						ResourceAllocInfo& info = alloc_resources[(size_t)effective].active();
						T& handler = info.get_handler<T>();
						init_pass(info, flags);
						result = handler;
						// id stays DECLARATIVE: `result` belongs to a Context
						// cached across frames (pass_cache), so stamping the
						// replacement's id here would make the override sticky --
						// next frame's need() would start its lookup from the
						// replacement even after the override was cleared.
						// render() is unaffected either way, because it
						// dereferences `info`, which the copy above already
						// points at the replacement.
						result.id          = declared;
						result.chain_index = declared_chain;
						return;
					}
				}
			}

			auto& chain = alloc_resources[(size_t)result.id];

			// Unconditional, not the ASSERT(exists(result)) this replaced --
			// ASSERT compiles to ((void)0) outside DEV builds (Core/Defines.h),
			// so in an ordinary Debug-D3D12 build that check was silently a
			// no-op and this exact failure fell straight through to
			// ResourceChain::active() indexing an empty deque, which
			// null-derefed several calls later inside HAL code with an access
			// violation address that named neither the resource nor the pass.
			// Usual cause: this pass is registered in the Pipeline block
			// (test.sig) BEFORE whichever pass creates/writes this resource --
			// Graph::setup() runs every pass's setup() in pipeline declaration
			// order, so a resource's creator must appear earlier in that block
			// than anything that need()s it. Fix the pipeline order, not this
			// check.
			if (!chain.created_this_frame)
				report_resource_not_created(result.id, current_pass);

			ResourceAllocInfo& info = chain.active();
			T& handler = info.get_handler<T>();
			init_pass(info, flags);

			result = handler;
			result.id          = info.id;
			result.chain_index = info.chain_index;
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

		// O(1): pass_cache is indexed by PassID then instance, and holds exactly
		// the Pass objects add_library_pass reuses each frame.
		Pass* get_pass(PassRef ref) const
		{
			if (ref.id == PassID::Count) return nullptr;
			auto& slots = const_cast<TaskBuilder*>(this)->pass_cache[(size_t)ref.id];
			if (ref.index >= slots.size()) return nullptr;
			return slots[ref.index].get();
		}

		static PassRef ref_of(const Pass& pass);

		// Resolves a ResourceVersion to its chain link. Distinct from get(id)
		// above on purpose: that one returns chain.active() and nulls out when
		// the resource is disabled, which is right for its callers but wrong
		// here -- a [Recreate] pass needs the NON-active link too.
		//
		// const, returning a mutable link: the chain OWNS its links and is only
		// being indexed here, and the read-only consumer this exists for (the
		// timeline debugger, which takes a const Graph&) still needs the same
		// mutable ResourceAllocInfo* the pass containers used to hand it
		// directly. Making it const-correct all the way down would mean
		// threading constness through ResourceCell and the preview widgets for
		// no actual safety gain.
		ResourceAllocInfo* get(ResourceVersion v) const;

		// The identity of a link, for storing in pass containers or a plan.
		static ResourceVersion version_of(const ResourceAllocInfo& info)
		{
			return { info.id, info.chain_index };
		}
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
		Compute = 2,

		// Which compute queue an async pass lands on. Compute alone = the first
		// compute queue; these pick the extra ones.
		Compute2 = 4,
		Compute3 = 8
	};


	// A plain bool return from setup_func conflates two different questions:
	// does this pass exist in the graph at all this frame (need()/create()
	// calls happen, barriers get computed) vs should its render() actually
	// run. Most passes never need the distinction (false means fully
	// disabled), but a pass that must touch a resource every frame
	// regardless of whether there's anything to draw this frame (e.g. a
	// ResourceChain that has to reset even on an empty frame) needs a third
	// state -- IgnoreRender -- instead of hand-ordering its own
	// need()/create() calls before an early return the way that pattern
	// used to be faked.
	//
	// Implicitly convertible both ways with bool (true <-> NeedsRender,
	// false <-> Disabled), so every existing `return true;`/`return false;`
	// setup_func keeps compiling and behaving unchanged -- only a pass that
	// wants the middle state needs to say so explicitly, by having its
	// setup() declared to return SetupResult instead of bool (see pass_defaults.jinja's
	// [TriState] option for PassDefault<T>-style passes).
	struct SetupResult
	{
		enum Value
		{
			Disabled,     // pass does not exist this frame: no need()/create(), no render()
			IgnoreRender, // need()/create() run as normal, but render() does not
			NeedsRender,  // normal case: need()/create() run AND render() runs
		};

		Value value;

		SetupResult(bool b) : value(b ? NeedsRender : Disabled) {}
		SetupResult(Value v) : value(v) {}

		// Preserves the existing bool contract every Pass::setup() caller
		// (Graph's enabled/renderable bookkeeping) already relies on.
		operator bool() const { return value == NeedsRender; }

		bool touches_resources() const { return value != Disabled; }
	};


	struct Pass
	{
		UINT id = 0;
		PassID type_id = PassID::Count;

		// Last frame's setup decision, reused when none of this pass's
		// condition_fields changed. Lives on the Pass because Pass objects (and
		// their Contexts) are recycled per PassID across frames via
		// TaskBuilder::pass_cache -- a per-frame structure would have nothing to
		// remember. `valid` guards the case where this object was just allocated
		// or its pass was absent last frame, when there is no prior result to
		// trust.
		bool          setup_result_valid = false;
		SetupResult   cached_setup_result = false;

		// Frame this pass last actually evaluated its condition. Compared
		// against the graph's per-field "last changed on frame N" table rather
		// than against a single last-frame dirty mask: a pass can be ABSENT from
		// the graph for many frames (an AssetPreview slot nobody claimed, a
		// PSSM cascade whose render_func is unset), during which a last-frame
		// mask would report those frames' changes to nobody and then let this
		// pass reuse a result that predates them.
		uint64_t      last_setup_frame = 0;

		// Whether this pass may skip re-evaluating its setup condition this
		// frame. Defined in FrameGraph.cpp: it reads the generated
		// pass_context_deps table and the graph's dirty mask, neither of which
		// is reachable from here. Also bumps the graph's hit/miss tally.
		bool can_reuse_setup(TaskBuilder& builder);
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

		// The pass's generated Cache as a flat ChainIndex span, so a plan can pool
		// every pass's slots without knowing any Context type. load_cache is what
		// replaces setup() on a replayed frame; it writes only this pass's own
		// Context, so it has no ordering requirement against other passes.
		virtual uint32_t cache_slot_count() const { return 0; }
		virtual void save_cache(std::span<ChainIndex> out) const {}
		virtual void load_cache(std::span<const ChainIndex> in, const TaskBuilder& builder) {}

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
		using setup_func_type = std::function<SetupResult(Handler&, TaskBuilder&)>;
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

		static constexpr uint32_t count_cache_slots()
		{
			if constexpr (requires { typename Handler::Cache; })
				return std::is_empty_v<typename Handler::Cache> ? 0 : uint32_t(sizeof(typename Handler::Cache) / sizeof(ChainIndex));
			else
				return 0;
		}
		static constexpr uint32_t cache_slots = count_cache_slots();

		virtual uint32_t cache_slot_count() const override { return cache_slots; }

		virtual void save_cache(std::span<ChainIndex> out) const override
		{
			if constexpr (cache_slots > 0)
			{
				typename Handler::Cache cache;
				Handler::save_to_cache(data, cache);
				auto slots = std::bit_cast<std::array<ChainIndex, cache_slots>>(cache);
				std::ranges::copy(slots, out.begin());
			}
		}

		virtual void load_cache(std::span<const ChainIndex> in, const TaskBuilder& builder) override
		{
			if constexpr (requires { data.pass_index = pass_index; })
				data.pass_index = pass_index;

			if constexpr (cache_slots > 0)
			{
				std::array<ChainIndex, cache_slots> slots;
				std::ranges::copy(in.first(cache_slots), slots.begin());
				Handler::load_from_cache(data, std::bit_cast<typename Handler::Cache>(slots), builder);
			}
		}

		virtual bool setup(TaskBuilder& builder) override
		{
			builder.begin(this);
			// [Multiple=N]-generated: writes this pass instance's own index
			// (already tracked generically on every Pass, see internal_pass's
			// own pass_ptr->pass_index assignment) into data.pass_index, before
			// setup_func or anything else runs -- so a [Multiple] pass's own
			// setup_func, and any [Optional=`data.pass_index == ...`] guard in
			// generated need_always()/create_always(), can read it without the
			// pass author hand-writing their own index field + assignment
			// (data.cascade_index = i; and equivalents) the way PSSM_Cascade
			// used to.
			if constexpr (requires { data.pass_index = pass_index; })
				data.pass_index = pass_index;
			// [PrevFor=X]-generated: registers this pass's *Prev history links
			// (builder.link_history()) before setup_func runs. link_history()
			// must run before its current-frame resource's own create() --
			// manual (inside setup_func) or auto (create_always(), which runs
			// after setup_func) -- so "before setup_func" is the one hook that
			// covers both. Unconditional (not gated on touches_resources()):
			// link_history() only registers metadata, doesn't allocate, and is
			// documented idempotent -- safe even the frame this pass is disabled.
			if constexpr (requires { Handler::link_history_always(data, builder); })
				Handler::link_history_always(data, builder);

			// Reuse last frame's decision when nothing this pass's condition
			// reads has changed. Only the DECISION is cached -- create_always()/
			// need_always() below still run either way, because they record into
			// a builder that was reset this frame.
			SetupResult res = false;
			if (can_reuse_setup(builder))
			{
				res = cached_setup_result;
			}
			else
			{
				res = setup_func(data, builder);
				cached_setup_result = res;
				setup_result_valid   = true;
			}
			if (res.touches_resources())
			{
				// Only when the pass isn't fully Disabled - a pass can
				// legitimately not exist this frame, and builder.need()
				// asserts exists() first, so needing [Always] resources
				// unconditionally would crash the moment a pass bails out.
				// IgnoreRender still runs these: that's the whole point of
				// the third state.
				//
				// create_always() before need_always(): a [PrevFor=X] *Prev
				// field can be BOTH auto-created by this same pass's own
				// create_always() (via create_prev(), e.g. Scene's
				// GBuffer_DepthPrev) AND auto-needed by this same pass's own
				// [Always=Read] (e.g. a consumer reading it back later that
				// frame) -- need_always() running first would need() a
				// resource this pass's own create_always() hasn't provisioned
				// yet. No existing [Always] field depends on the reverse order
				// (need_always() only ever reads resources OTHER passes, or
				// this pass's OWN create()/create_prev(), already provisioned).
				if constexpr (requires { Handler::create_always(data, builder); })
					Handler::create_always(data, builder);
				if constexpr (requires { Handler::need_always(data, builder); })
					Handler::need_always(data, builder);
			}
			builder.end(this);

			return (bool)res;
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



	// ---- Stored graph plan --------------------------------------------------
	// The result of one setup(), flattened to POD keyed by PassID/ResourceVersion
	// so it can be replayed (and eventually written to disk) instead of
	// recomputed.
	//
	// Deliberately NOT a copy of the live objects: those are dense pointer graphs
	// (ResourceAllocInfo*, Pass*, spans into per-resource pools), and rebuilding
	// the pointers on replay would cost about what recomputing costs. Everything
	// here is an index or an ID, so replay is a flat walk that fills the
	// persistent objects the builder already owns.
	//
	// Shape mirrors the PrecompiledPass/PrecompiledResourceInfo tables SIGParser
	// already emits per pipeline -- this is the runtime-pruned instance of that
	// same template, which is why the two use the same vocabulary.

	// Re-exported from the generated header: context_deps.h sits in this
	// interface's global module fragment, so its constants are invisible to
	// implementation units and importers. Binding it to an exported constant
	// here is the cheap fix -- re-including the header in FrameGraph.cpp would
	// drag <bitset> into a module purview and collide with `import HAL`.
	inline constexpr uint64_t sig_id_space_hash = generated_id_space_hash;

	// Same reason: ContextFieldID and desc_only_fields live in the global module
	// fragment, so an implementation unit can hold values of those types but
	// cannot NAME them. These two give FrameGraph.cpp what it needs without
	// re-including the header (which would pull <bitset> into a purview).
	inline constexpr unsigned int sig_context_field_count = (unsigned int)ContextFieldID::Count;

	// Not inline, for the same linkage reason as find_context_deps below: an
	// inline function in a module interface that reads a GMF-local constant
	// leaves every importing TU with an unresolvable reference.
	bool sig_is_desc_only_field(unsigned int i);

	// Whether a field can change graph topology: some pass's condition or
	// [Optional] guard reads it. A field nothing reads (SkyState::prev_sun_dir is
	// a pre_setup's own comparison state) must stay out of the graph key, or
	// every change to it forks a new plan.
	bool sig_is_graph_key_field(unsigned int i);

	struct PlanAccess
	{
		ResourceVersion version;
		ResourceFlags   flags;

		bool operator==(const PlanAccess&) const = default;
	};

	struct PlanPass
	{
		PassID   type_id;
		uint32_t pass_index;
		uint32_t call_id;
		bool     enabled;
		bool     renderable;

		// Spans into the plan's flat pools, so a PlanPass stays trivially
		// copyable and the whole plan serializes as a handful of arrays.
		//
		// No creations/deletions here on purpose: those three used.* vectors are
		// filled by create_resources(), which runs in compile() AFTER the plan is
		// recorded -- so they were always captured empty, and they are rebuilt on
		// every frame anyway, replayed or not. Recording them made the plan
		// unstable without ever being read.
		uint32_t access_begin = 0,      access_count = 0;

		// Span into GraphPlan::cache_slots: this pass's generated Cache.
		uint32_t cache_begin = 0,       cache_count = 0;

		bool operator==(const PlanPass&) const = default;
	};

	struct PlanState
	{
		bool     write;
		bool     exclusive;
		uint32_t ref_begin, ref_count;   // into GraphPlan::pass_refs

		bool operator==(const PlanState&) const = default;
	};

	struct PlanResource
	{
		ResourceVersion version;
		bool     enabled;

		// Creation flags OR'd with every access this frame -- topology-dependent
		// (which passes ran), so it belongs here and not in the viewport-keyed
		// desc table. Drives the D3D12 resource desc via create_resource_desc.
		ResourceFlags flags;

		bool     is_history_current;
		bool     is_history_prev;


		// See ResourceAllocInfo::desc_from_prev. Descs themselves are not in the
		// plan: create_versioned recomputes them from context on every replay, so
		// a resize reuses the same plan.
		bool     desc_from_prev;

		uint32_t state_begin, state_count;

		bool operator==(const PlanResource&) const = default;
	};

	struct GraphPlan
	{
		uint64_t key = 0;

		// History links registered this frame ([PrevFor]-generated
		// link_history_always). Replay has to re-register them: they are consumed
		// by create()'s provision_history_prev and by roll_history at frame end.
		std::vector<std::pair<ResourceID, ResourceID>> history_links;

		// Must equal generated_id_space_hash to be usable. A plan recorded before
		// a .sig edit renumbered PassID/ResourceID would not fail to apply, it
		// would apply to the WRONG passes and resources -- so this is checked
		// before anything else, and a mismatch is a miss, not an error.
		uint64_t id_space_hash = 0;

		std::vector<PlanPass>        passes;
		std::vector<PlanResource>    resources;
		std::vector<PlanState>       states;
		std::vector<PlanAccess>      accesses;
		std::vector<PassRef>         pass_refs;  // PlanState -> passes pool

		// builder.enabled_resources, in order.
		//
		// Recorded explicitly rather than derived from PlanResource::enabled,
		// because those are not the same set. ResourceAllocInfo::enabled is only
		// cleared by info.reset() (i.e. by create()), so a resource not created
		// this frame keeps last frame's flag, while the cull pushes to
		// enabled_resources only on the false->true transition. Deriving the list
		// from the flag over-enables -- measured 70 vs 61 -- and the surplus gets
		// allocated for nothing. Order matters too: create_resources walks this
		// list, so allocation order follows it.
		std::vector<ResourceVersion> enabled_resources;

		// Pool for every pass's Cache (PlanPass::cache_begin/count).
		std::vector<ChainIndex> cache_slots;

		bool operator==(const GraphPlan&) const = default;
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

		UpscalerViewport upscaler_viewport;
		uint32_t get_upscaler_viewport() const { return upscaler_viewport; }


		// Diagnostic: serialize all queues (fence after EVERY pass, all other
		// queues wait) — kills any cross-queue concurrency. Garbage that
		// survives this is not an async/lifetime race but a rogue writer
		// (out-of-bounds dispatch, wrong descriptor, stale indirect args).
		Variable<bool> serialize_queues = { false, "serialize_queues", this };

		// Points one of `pass`'s declared resource fields at a different resource
		// for this frame. Must be called before setup() (graph construction is
		// the intended place); cleared every frame by start_new_frame().
		void override_resource(PassID pass, ResourceID from, ResourceID to)
		{
			builder.resource_overrides.emplace_back(pass, from, to);
		}

		// True when any override is active this frame.
		//
		// This is the contract for a future setup/allocation cache: an
		// overridden frame must neither READ nor WRITE the cache. Skipping only
		// the read while still storing would poison the entry for the next
		// normal frame that shares its key. Honour both halves and the frame
		// after an override is removed needs no special case -- it loads the
		// entry stored before the override, which is still valid precisely
		// because an override never altered the un-overridden computation.
		//
		// Recomputing wholesale is the right response rather than invalidating
		// just the overridden pass: redirecting a read extends the replacement
		// resource's used_end to this pass, which moves aliasing and placement
		// decisions in create_resources() for unrelated resources too.
		bool has_resource_overrides() const { return !builder.resource_overrides.empty(); }

		// Identifies the graph this frame will build, for looking up a stored
		// plan (DumpGraph/LoadGraph). Valid only AFTER run_pre_setups(), which is
		// the last thing that may write a context -- every setup_func is
		// generated and does nothing but read Table:: contexts and return a
		// SetupResult, so from that point the inputs are frozen.
		//
		// The snapshot alone is not enough. Three inputs decide the graph and are
		// not context fields:
		//   - WHICH passes were added. add_passes() registers a pass only when its
		//     render_func is set, so AssetPreview's claimed slots and PSSM's
		//     cascades change the pass set with no context involved.
		//   - Graph::optimize, which strips Compute flags inside setup().
		//   - the pass_texture()'d resources, which arrive from outside the graph.
		//
		// Returns 0 when no plan may be used at all -- see has_resource_overrides().
		uint64_t compute_graph_key() const;

		// Records the graph just built by setup() into a plan keyed by
		// compute_graph_key(), and replays one instead of running setup().
		// Recording happens on the dynamic path only, so it costs nothing on a hit.
		void DumpGraph(uint64_t key);
		bool LoadGraph(uint64_t key);

		// Recorded plans, keyed by compute_graph_key(). Linear lookup over a
		// handful of entries: distinct keys only appear when something
		// structural changes (a debug toggle, a resize), which the measured
		// 99.3% condition-reuse rate says is rare.
		std::vector<GraphPlan> graph_plans;

		GraphPlan* find_graph_plan(uint64_t key)
		{
			if (!key) return nullptr;
			for (auto& p : graph_plans)
				if (p.key == key && p.id_space_hash == sig_id_space_hash)
					return &p;
			return nullptr;
		}

		// Records the plan every frame and compares it against the stored one
		// for the same key, instead of trusting that a key collision implies an
		// identical graph. Off by default: it defeats the point of the cache
		// (recording on every frame) and exists to prove the key is sound before
		// LoadGraph is allowed to skip work on it.
		Variable<bool> validate_graph_plan = { false, "validate_graph_plan", this };

		// Replay a stored plan instead of running setup(). Off by default: this
		// is the path that skips the serial declare loop, and it wants proving
		// against validate_graph_plan before it becomes the default.
		Variable<bool> use_graph_plan = { false, "use_graph_plan", this };

		// Set when validation found a recorded plan that differs from the stored
		// one under the same key -- i.e. the key is missing an input.
		uint32_t graph_plan_mismatches = 0;

		// ---- Setup-result cache ------------------------------------------
		// Last frame's context values and which fields differ from them, filled
		// by update_context_dirty_mask() (autogen/context_snapshot.cpp) after
		// run_pre_setups() and before setup(). A pass whose condition_fields
		// (autogen/context_deps.h) miss this mask cannot produce a different
		// SetupResult than it did last frame, so it reuses it instead of
		// re-evaluating.
		//
		// Note what this does NOT skip: create_always()/need_always() still run.
		// Those record into a builder that is reset every frame, so their effects
		// have to be reproduced whatever the condition result was. Only the
		// decision is cached here, not the resource declarations that follow it.
		ContextSnapshot  prev_context_snapshot;
		ContextFieldMask dirty_context_fields;
		bool             has_context_snapshot = false;

		// Frame number each field last changed on, and the running frame count.
		// This is what makes reuse safe for a pass that skipped frames -- see
		// Pass::last_setup_frame.
		uint64_t         context_field_changed_frame[(unsigned int)ContextFieldID::Count] = {};
		uint64_t         context_frame_index = 0;

		// Hash of the whole snapshot, updated alongside it. One half of the
		// stored-plan key; see compute_graph_key().
		uint64_t         context_snapshot_hash = 0;

		// Per-frame tally, reset in start_new_frame(). Kept as real state rather
		// than a debug-only counter because the hit rate is the number that says
		// whether caching more of setup() is worth doing at all.
		uint32_t setup_cache_hits = 0;
		uint32_t setup_cache_misses = 0;

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

		// The derived tail of graph construction, shared by setup() and
		// LoadGraph(). Nothing here is recorded in a plan -- it all follows from
		// the enabled set -- so both paths must run it.
		void finalize_graph();
		void compile(int frame);
		void render();


		HAL::FenceWaiter commit_command_lists();
		void reset();

	};


	// Defined here rather than in FrameGraph.cpp because it reads
	// pass_context_deps, which lives in this interface's global module fragment
	// and is therefore invisible to an implementation unit -- and re-including
	// context_deps.h there is not an option either: it pulls <bitset>, and a
	// standard header in an implementation unit's fragment collides with the
	// same types arriving through `import HAL`/`import Core`.
	//
	// Out-of-line rather than inside Pass because it needs Graph complete.

	// Looks this pass up in the generated dependency table. Linear over ~55
	// entries, only on the decision path; if it ever shows up in a profile the
	// table is indexable by PassID directly (it is emitted in PassID order).
	// NOT inline: an inline function in a module interface that references
	// pass_context_deps (internal linkage, from the global module fragment)
	// leaves every importing TU with an unresolvable external reference.
	bool sig_is_desc_only_field(unsigned int i)
	{
		return i < sig_context_field_count && desc_only_fields.test(i);
	}

	bool sig_is_graph_key_field(unsigned int i)
	{
		static const ContextFieldMask key_fields = []
		{
			ContextFieldMask m;
			for (const auto& d : pass_context_deps)
			{
				// An incomplete mask is only a lower bound, so no field can be
				// proven irrelevant: fall back to every non-desc field.
				if (!d.deps_complete)
					return ~desc_only_fields;
				m |= d.condition_fields | d.optional_fields;
			}
			return m & ~desc_only_fields;
		}();
		return i < sig_context_field_count && key_fields.test(i);
	}

	const PassContextDeps* find_context_deps(PassID id)
	{
		for (const auto& d : pass_context_deps)
			if (d.pass == id)
				return &d;
		return nullptr;
	}

	bool Pass::can_reuse_setup(TaskBuilder& builder)
	{
		Graph* graph = builder.graph;

		// Every early-out below is followed by setup_func() actually running, so
		// stamping here is the same thing as stamping at the call site -- which
		// TypedPass::setup() cannot do, because Graph is still incomplete there.
		auto miss = [&]
		{
			++graph->setup_cache_misses;
			last_setup_frame = graph->context_frame_index;
			return false;
		};

		// No prior result, or a pass with no PassID at all (a hand-written
		// add_pass<T>, which has no generated dependency data and therefore no
		// way to know what it reads).
		if (!graph)
			return false;

		if (!setup_result_valid || type_id == PassID::Count)
			return miss();

		// An override redirects which resource a pass reads, which is not
		// expressible as a context field -- so the whole frame is uncacheable.
		// Same contract Graph::has_resource_overrides() documents.
		if (graph->has_resource_overrides())
			return miss();

		const PassContextDeps* deps = find_context_deps(type_id);

		// deps_complete == false means the mask is a lower bound, not the full
		// set: the pass reads something the extractor could not prove. Treat it
		// as permanently dirty rather than trusting an incomplete mask -- an
		// under-reported dependency is the one failure here that is silent.
		if (!deps || !deps->deps_complete)
			return miss();

		// Any field this pass's condition reads that changed at or after the
		// frame this pass last evaluated. Not a test against the current frame's
		// dirty mask: that would miss changes made while this pass was absent
		// from the graph entirely.
		for (unsigned int i = 0; i < (unsigned int)ContextFieldID::Count; ++i)
		{
			if (!deps->condition_fields.test(i))
				continue;

			if (graph->context_field_changed_frame[i] >= last_setup_frame)
				return miss();
		}

		++graph->setup_cache_hits;
		return true;
	}


	class GraphGenerator
	{
	public:
		virtual void generate(Graph& graph) = 0;
	};


}
