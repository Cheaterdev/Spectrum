module;


export module HAL:ResourceStates;

import Core;

import :Types;
import :DescriptorHeap;   // ResourceInfo, ALL_SUBRESOURCES

using namespace HAL;

export
{

	namespace HAL
	{
		class Transitions;
		class TiledResourceManager;


		// Per-(resource, command list) state. Extends the generic tracked-object
		// state (used / alias_ended, which the command list's track_object needs)
		// with the resource-specific tracking the barrier system builds on, so a
		// resource carries ONE per-list state object rather than a separate
		// parallel store.
		//
		// reset() runs whenever a command list is reused for a new frame (see
		// ObjectState::get_state) -- it must chain to the base explicitly, since
		// declaring reset() here hides TrackedObjectState::reset().
		// One place a resource was used inside a single operation. A resource can
		// be bound several times within one operation (several views, several
		// copy regions), so an operation holds a list of these rather than a
		// single entry.
		//
		// Two ways a use names its subresources:
		//   - through a bound view (`info`), which carries the mip/array/plane
		//     range the barrier system has to expand;
		//   - as a bare subresource index (`subres`), for uses that have no
		//     descriptor at all -- copies name a subresource directly.
		// `info` is null in the second case.
		struct OperationUsage
		{
			const ResourceInfo* info = nullptr;
			UINT subres = ALL_SUBRESOURCES;

			// How the resource is used here. Kept even though the operation
			// already carries a BarrierSync: the sync class alone does not
			// determine access/layout for a bare-subresource use -- a COPY
			// operation is COPY_SOURCE for one resource and COPY_DEST for
			// another, and nothing in `subres` distinguishes them.
			ResourceState state;

			// Which command within the operation recorded this. An operation is
			// a RUN of same-class work, so several dispatches share one -- and
			// two usages with different `step` values are different dispatches,
			// which is what distinguishes "bound twice for one dispatch" from
			// "written by one dispatch, then touched by the next".
			uint step = 0;

			OperationUsage() = default;
			OperationUsage(const ResourceInfo* info, ResourceState state) : info(info), state(state) {}
			OperationUsage(UINT subres, ResourceState state) : subres(subres), state(state) {}
		};

		struct TrackedResourceState : TrackedObjectState
		{
			// True once this resource has been added to the list's
			// used_resources, i.e. the barrier pass will visit it.
			//
			// Deliberately NOT TrackedObjectState::used, which means "added to
			// tracked_resources" (kept alive for the list's lifetime) and is set
			// by CommandListBase::track_object. A resource now carries ONE state
			// object for both, so a single flag cannot serve both: track_object
			// runs first for non-FrameGraph resources, and use_resource would
			// see `used` already set and never list the resource -- leaving
			// compile_transitions with nothing to do and no barriers emitted.
			bool listed = false;

			// The state this list will FIND the resource in, when the group has
			// no way to know it -- i.e. this list holds the group's first touch
			// of it. Declared by whoever owns the resource's scheduling: the
			// FrameGraph knows the whole pass graph, so for its own resources it
			// can say what the previous pass left behind, across a group
			// boundary the group itself cannot see over.
			//
			// Unset means "assume the resting layout", which is correct for any
			// resource the group model owns end to end (every group hands those
			// back at rest). This is the replacement for the old
			// prepare_state()/zero-transition seed node.
			bool          has_entry_state = false;
			ResourceState entry_state;

			// Every operation on this list that touched this resource, keyed by
			// CmdListOperation::index -> the places it was used in that
			// operation. Ordered map: iteration is already in operation order,
			// which is the order the barrier pass needs.
			//
			// Keyed by index, not pointer: the list's `operations` is a vector,
			// so pointers into it would dangle as it grows -- and
			// CmdListOperation isn't visible from here anyway (CommandList
			// imports ResourceStates, not the reverse).
			std::map<uint, std::vector<OperationUsage>> operations;

			void reset()
			{
				TrackedObjectState::reset();
				listed = false;
				has_entry_state = false;
				operations.clear();
			}
		};


		// The LAYOUT a resource sits in whenever no command list is actively
		// using it, derived from what it was created to be used for. Every list
		// leaves a resource here when it is done with it, so the next list can
		// assume it without any cross-list bookkeeping.
		//
		// Only a layout: sync and access describe a specific access at a
		// specific point, and a resource at rest is by definition not being
		// accessed by anyone.
		TextureLayout resting_layout(const Resource* resource);

		// Fill a resting layout back out into the ResourceState a barrier needs.
		// The access follows from the layout; the sync stays generic, because a
		// barrier against a resting resource is crossing into (or out of) a
		// scope where no specific producer/consumer stage is known -- and it
		// must NOT be BarrierSync::NONE, which D3D12 rejects once a resource has
		// been accessed.
		ResourceState state_at_rest(TextureLayout layout);

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

			// How many of `barriers` target a buffer and how many a texture.
			// D3D12 wants those split into two separate native arrays, and the
			// resource desc is already in hand here at record time -- so the
			// backend can size both arrays exactly instead of growing them and
			// re-walking the list to find out how big they should have been.
			uint buffer_count = 0;
			uint texture_count = 0;

			CommandListType type;
		public:

			Barriers(CommandListType type);
			operator bool() const;
			void clear();
			const std::vector<Barrier>& get_barriers() const;

			uint get_buffer_count() const { return buffer_count; }
			uint get_texture_count() const { return texture_count; }

			void transition(const Resource* resource, ResourceState before, ResourceState after, UINT subres, BarrierFlags flags = BarrierFlags::SINGLE);

		};

		// One contiguous run of same-class work on a command list. A list is a
		// sequence of these: begin_op() starts a new one only when the incoming
		// operation class differs from the one currently at the back, so a run
		// of same-class commands grows a single CmdListOperation instead of
		// producing one entry per command.
		//
		// Also owns the two barrier groups that bracket its work. Batching is
		// therefore structural rather than a separate layer: everything that
		// must happen ahead of this operation lands in the one `barriers_before`
		// group, and the recorder emits each group at its own reserved point in
		// the command stream (DelayedCommandList::func_barrier).
		//
		// Lives here rather than in :CommandList because the recorder has to
		// name the type, and :CommandList imports :CommandListRecorder, not the
		// other way round.
		struct CmdListOperation
		{
			BarrierSync type = BarrierSync::NONE;

			// Position of this operation in its list's `operations` sequence
			// (0-based). Stamped at creation and never changed, so it stays
			// valid as a stable ordering key -- "which operation came first"
			// is a plain integer compare, no pointer chasing.
			uint index = 0;

			// Runs immediately before this operation's commands: the state
			// transitions it needs, plus first-use initialization (discards).
			HAL::Barriers barriers_before;

			// Runs immediately after this operation's commands, before the next
			// operation begins -- for things that must trail the work rather
			// than precede it, e.g. end-of-aliasing barriers.
			HAL::Barriers barriers_after;

			// Whether barriers_after has already been given its point in the
			// command stream. Reserving it twice emits the SAME barrier group
			// twice: the second run declares a LayoutBefore the first one
			// already moved past, which D3D12 rejects (#1334). That happens
			// whenever something appends to a list after CommandList::end()
			// closed it -- process_transitions does exactly that.
			bool closed = false;

			CmdListOperation(CommandListType list_type, BarrierSync type, uint index)
				: type(type), index(index), barriers_before(list_type), barriers_after(list_type) {}
		};

	}

}
