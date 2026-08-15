module;


export module HAL:ResourceStates;

import Core;

import :Types;

using namespace HAL;

export
{

	namespace HAL
	{
		class Transitions;
		class TiledResourceManager;


		enum class TransitionType :int
		{
			ZERO,
			LAST
		};

		bool IsCompatible(CommandListType a, CommandListType b);
		bool IsFullySupport(CommandListType type, const ResourceState& states);
		CommandListType Merge(CommandListType a, CommandListType b);
		std::optional<ResourceState> merge_state(const ResourceState& source, const ResourceState& need);
		std::optional<TextureLayout> merge_layout(const TextureLayout& source, const TextureLayout& need);

		enum class BarrierFlags : UINT
		{
			NONE = 0,
			BEGIN = 1,
			END = 2,
			SINGLE = BEGIN | END,
			DISCARD = 4
		};

		struct Barrier
		{
			Resource* resource;
			ResourceState before;
			ResourceState after;
			UINT subres;
			BarrierFlags flags;
		};


		class Barriers
		{
			std::vector<Barrier> barriers;

			CommandListType type;
		public:

			Barriers(CommandListType type);
			operator bool() const;
			void clear();
			const std::vector<Barrier>& get_barriers() const;

			void transition(const Resource* resource, ResourceState before, ResourceState after, UINT subres, BarrierFlags flags = BarrierFlags::SINGLE);

		};

		struct UsagePoint;

		struct ResourceUsage
		{
			Resource* resource = nullptr;
			ResourceState wanted_state;
			UINT subres = -1;

			ResourceUsage* prev_usage = nullptr;
  			ResourceUsage* next_usage = nullptr;

			UsagePoint* point = nullptr;

			bool debug = false;

			// Set by ResourceStateManager::chain_lists when this usage is a
			// SYNC_NONE list-boundary marker (a seed, a prepare_state zero
			// transition, or a prepare_after_state release) that got bypassed
			// because the neighbouring list is in the same ExecuteCommandLists
			// group.  compile_transitions must not emit a barrier for it: inside
			// one ECL scope a SyncBefore/SyncAfter of NONE is illegal once the
			// resource has been touched (#1417).
			bool suppressed = false;

			// Phase 5 maximal split: set by chain_lists when this is a uniform
			// (ALL_SUBRESOURCES) usage whose source list left the resource in a
			// MIXED per-subresource state (e.g. a producer that writes mip 0 as
			// UAV but leaves the rest in SHADER_RESOURCE). A single ALL barrier
			// can only carry one before-layout, so it mismatches whichever
			// subresources differ (D3D12 #1334) and its SYNC_NONE decay makes them
			// untouchable (#1417). When non-empty, compile_transitions emits one
			// barrier per subresource using split_before[i] as that subresource's
			// real before-state instead of the single ALL barrier; entries equal to
			// wanted_state (untouched or already-matching subresources) emit
			// nothing. After this usage the resource is uniform again.
			std::vector<ResourceState> split_before;
		};

		struct UsagePoint
		{
			uint index;
			std::vector<HAL::ResourceUsage*> usages;
			HAL::Barriers  transitions;
			bool start = false;
			UsagePoint* prev_point = nullptr;
			UsagePoint* next_point = nullptr;
			Transitions* cmd_list;
			BarrierSync operation;
			UsagePoint(CommandListType type);
		};

		struct ResourceListStateCPU
		{
			ResourceUsage* first_usage = nullptr;
			ResourceUsage* last_usage = nullptr;

			bool used = false;
			bool need_discard = false;

			// A LATER list in the same ExecuteCommandLists group also uses THIS
			// subresource, so its state was handed over directly (chain_lists) and
			// this list must not emit the end-of-list release to
			// NO_ACCESS/SYNC_NONE — inside one ECL
			// scope that makes it untouchable afterwards (D3D12 #1417). Tracked
			// per subresource: chaining is per subresource, and a resource whose
			// mips are only partially carried forward must still release the rest,
			// or it never returns to `initial_layout` and the next group's seed
			// mismatches (D3D12 #1334).
			bool skip_end_decay = false;

			ResourceState get_first_usage();
			ResourceState get_usage();
			void reset();
#ifdef DEV
			void check_valid(const Resource* resource);
#else
			void check_valid(const Resource*) {}
#endif
			ResourceUsage* add_usage(ResourceUsage* usage);
			ResourceUsage* set_zero_transition(ResourceUsage* usage);
		};

		struct SubResourcesGPU;
		struct SubResourcesCPU
		{
			std::vector<ResourceListStateCPU> subres;

			bool used = false;

			// Operation batching: the id of the batch this resource was last
			// touched in, and the state it was transitioned to. The command
			// list compares batch_touch_id against its monotonic current_batch_id
			// to detect a same-batch reuse that needs a split. Reset per frame.
			uint          batch_touch_id = 0;
			ResourceState batch_touch_state;

			// Which subresources were read/written so far in this batch, for
			// detecting data dependencies that need ordering even when the
			// state does not change (UAV->UAV). Subresource index hashed into
			// 64 bits: exact for typical mip/slice counts, conservative past
			// that (a false overlap costs a split, never misses a hazard).
			uint64        batch_write_mask = 0;
			uint64        batch_read_mask  = 0;

			// Uniform fast path: while `uniform`, all subresources are tracked as
			// one chain (`uniform_state`) whose usage nodes carry subres ==
			// ALL_SUBRESOURCES, so a full-resource transition costs one node/one
			// barrier instead of one per mip/array/plane. The first subresource-
			// scoped transition calls expand(): each per-subres slot is seeded
			// from uniform_state (sharing the chain history) and the resource is
			// per-subres for the rest of the list. slot() resolves either mode, so
			// read helpers work unchanged; only mutating loops must run once in
			// uniform mode (see transition/prepare_state).
			bool uniform = true;
			ResourceListStateCPU uniform_state;

			ResourceListStateCPU& slot(UINT id);
			const ResourceListStateCPU& slot(UINT id) const;
			void expand(UINT count);

			// Reverse of expand(): caller asserts every subresource has
			// re-converged to the same state (e.g. a batched pass that touched
			// every slice independently but landed them all back on
			// SHADER_RESOURCE) and folds back to the uniform fast path so the
			// NEXT full-resource transition costs one node/barrier again instead
			// of one per subresource. DEV builds verify the claim (ASSERT per
			// subresource); Retail trusts it unconditionally and collapses in
			// O(1) — same contract as the standard library's assert(). Returns
			// false only if there is nothing to collapse (already uniform, or
			// never expanded/touched).
			//
			// The DEV verify is THIS LIST's local bookkeeping only (every
			// subres[i].wanted_state agrees) — it is NOT proof the fold is
			// safe, and the reason is a specific, known mechanism, not a vague
			// one: once folded, slot(i) resolves to the single shared
			// uniform_state for ANY index, including ones the fold's own
			// verification never individually covered. SubResourcesGPU::merge()
			// persists per-subresource layout ACROSS FRAMES (not just this
			// list) by looping the full declared subresource count and calling
			// other.get_first_usage(i) unconditionally, no `used` guard — post-
			// fold that silently stamps the persistent layout for every
			// declared subresource, including ones never actually verified,
			// corrupting the record for whichever one the debug layer
			// disagrees with later. Confirmed live twice (2026-08): VSM_PageHiZ
			// (#1334, 3584x, its once-ever cold-start clear_uav path) and
			// VSM_Atlas even WITH the mapped-subresource mask below applied
			// correctly and the DEV assert passing (#1334, 72x) — see
			// [[project-assert-shared-state-vsm-pagehiz]]. A real fix needs
			// merge() (or an equivalent) to stop trusting slot(i) for indices
			// the fold didn't individually prove; not attempted yet. Until
			// then, treat "DEV assert passed" as necessary, not sufficient —
			// any resource whose layout persists across frames via merge()
			// (i.e. most things) is not provably safe to fold, live validation
			// or not.
			//
			// `mapped` (optional): for a reserved/Virtual resource, pass its
			// TiledResourceManager so subresources with no tile currently
			// mapped to real memory are skipped entirely -- neither required
			// to be `used` nor compared against the representative state.
			// Without this a resource like VSM_Atlas (2048 declared array
			// slices, ~256 ever tile-mapped) could never collapse: the
			// declared-but-never-backed slices would always fail the "every
			// subresource used" precondition. Mapped subresources still go
			// through the exact same used/state checks as before.
			bool assert_shared_state(const TiledResourceManager* mapped = nullptr);

			void reset();
			CommandListType get_best_list_type_last();
			CommandListType get_best_list_type_first();
			const ResourceListStateCPU& get_subres_state(UINT id) const;
			ResourceListStateCPU& get_subres_state(UINT id);
			ResourceUsage* get_first_usage(UINT id) const;
			ResourceUsage* get_last_usage(UINT id) const;
			ResourceState get_first_state(UINT id) const;
			ResourceState get_last_state(UINT id) const;
			void merge_read_state(CommandListType type, SubResourcesGPU& state);
		};


		struct ResourceListStateGPU
		{
			TextureLayout layout;
		};

		struct SubResourcesGPU
		{
			std::vector<ResourceListStateGPU> subres;

			bool is_valid() const;
			void operator=(const TextureLayout& layout);
			CommandListType get_best_list_type();
			void set_cpu_state(const SubResourcesCPU& cpu_state);
			void set_cpu_state_first(const SubResourcesCPU& cpu_state);
			const ResourceListStateGPU& get_subres_state(UINT id) const;
			ResourceListStateGPU& get_subres_state(UINT id);
			void merge(SubResourcesCPU& other);
		};



		class ResourceStateManager : public ObjectState<SubResourcesCPU>
		{
			const Resource* resource;

		protected:
			mutable SubResourcesGPU gpu_state;


		protected:
			// Non-FG resources: true until their first-ever use. The first
			// activation of a virgin resource discards (UNDEFINED->state) since
			// its contents are meaningless (fresh/placed memory) — point R4.
			// Flips false permanently after the first transition; later uses
			// preserve contents. FG resources ignore this (own activation path).
			mutable bool virgin = true;

		public:
			virtual ~ResourceStateManager() = default;
			TextureLayout initial_layout;
			using ptr = std::unique_ptr<ResourceStateManager>;
			UINT get_subres_count();
			ResourceStateManager(const Resource* resource);
			SubResourcesGPU copy_gpu() const;
			void init_subres(int count, TextureLayout layout);

			// The canonical READ state this resource should rest in after upload,
			// derived from its declared usage flags (ShaderResource -> SHADER_RESOURCE,
			// etc.). Used by the UploadManager to transition a freshly-uploaded
			// resource out of COPY_DEST/COMMON into a strictly-defined read state.
			ResourceState get_desired_state() const;

			// Pin the persistent resting state to `layout`. Called once by the
			// UploadManager's post-upload transition so every later command list
			// seeds this resource in its read state (a no-op) and never decays it
			// back to COMMON. This is the strict replacement for the old
			// gpu_state-as-first-list-link behaviour.
			void set_resting_state(TextureLayout layout);

			SubResourcesCPU& get_cpu_state(Transitions* list) const;

			bool is_used(Transitions* list) const;

			// See SubResourcesCPU::assert_shared_state — collapses this
			// resource's per-subresource tracking on `list` back to the uniform
			// fast path if (DEV: verified; Retail: trusted) every subresource is
			// currently at the same state. For a reserved/Virtual resource,
			// automatically narrows that requirement to tile-mapped
			// subresources only (via this resource's own TiledResourceManager)
			// — declared-but-never-mapped subresources (e.g. VSM_Atlas's
			// unused array headroom) are skipped rather than blocking the
			// fold.
			bool assert_shared_state(Transitions* list) const;


			void transition(Transitions* list, ResourceState state, unsigned int subres) const;

			void prepare_state(Transitions* from, const SubResourcesGPU& subres) const;
			void prepare_state(Transitions* from, ResourceState state) const;

			void prepare_after_state(Transitions* from, const SubResourcesGPU& subres) const;

			void alias_begin(Transitions* list) const;
			void alias_end(Transitions* list) const;

			void connect(Transitions* from, Transitions* to);

			// `from` and `to` are two lists that
			// will be submitted inside ONE ExecuteCommandLists scope, `from`
			// first. Gives `to`'s first real usage `from`'s last usage as its
			// predecessor — so the emitted barrier carries a real SyncBefore
			// instead of the SYNC_NONE seed — and marks `from` to skip its
			// end-of-list release. Result: one SYNC_NONE entry and one
			// SYNC_NONE exit per resource per group instead of one per list.
			void chain_lists(Transitions* from, Transitions* to) const;

		};


	}

}
