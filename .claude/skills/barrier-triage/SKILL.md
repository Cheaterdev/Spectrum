---
name: barrier-triage
description: Diagnose D3D12 barrier, resource-layout, and resource-state validation errors in the operation-based barrier system (CmdListOperation / TrackedResourceState / CommandListGroup). Use this skill whenever the debug layer reports a barrier, layout, sync, access, or discard error — especially message IDs #1334, #1417, and #1422 — or when a resource is in an unexpected layout, a transition is rejected, a GPU hang or TDR is suspected to come from a missing barrier, or work in HAL.CommandList, HAL.CommandListRecorder, HAL.ResourceStates, or HAL.Queue needs its barrier consequences reasoned through.
---

# Barrier triage

Barriers here are not emitted where the code asks for them. Work is recorded
into `CmdListOperation` runs, and the barriers are *computed afterwards*, across
a whole `CommandListGroup`, by `compile_transitions`. So the fix for a barrier
error is almost never "insert a barrier at the call site" — it is a wrong
declared state, a wrong resting layout, or a group-boundary handoff that the
compiler was never told about.

## Read the source before theorising

The implementation is heavily commented and the comments carry the invariants,
including the specific validation IDs each one prevents. Read the relevant part
rather than reconstructing the design from the error text:

| File | What it settles |
|---|---|
| `sources/HAL/HAL.ResourceStates.ixx` | `TrackedResourceState`, `CmdListOperation`, resting layout, `SubresRange` |
| `sources/HAL/HAL.CommandList.ixx` | `CommandListGroup`, `PlannedResource`, how barriers are computed across lists |
| `sources/HAL/HAL.CommandListRecorder.ixx` | How reserved barrier points map back to operations |
| `sources/HAL/HAL.Queue.cpp` | Submission ordering, `execute(group)` |
| `sources/HAL/API/D3D12/HAL.D3D12.Queue.cpp` | Where the compiled barriers reach the API |

## The core model

- A resource at rest sits in its **resting layout**, derived from what it was
  created for (`resting_layout`). Every group leaves a resource there when
  finished, so the next group can assume it without cross-group bookkeeping.
- Within a group, lists hand a resource to each other **directly**. A resource
  read by several consecutive lists does not bounce through the resting layout
  at each list boundary.
- A resource whose previous state cannot be known group-locally must have an
  **entry state** declared (`has_entry_state` / `entry_state`). The FrameGraph
  knows the whole pass graph, so it declares this for resources crossing a group
  boundary. Unset means "assume resting", which is correct only when the group
  model owns the resource end to end.
- `compile_transitions` must run **exactly once per list**. A list processed
  twice gets two independently computed barrier sets spliced into one command
  stream, which produces contradictory `LayoutBefore` values.

## Message IDs and what they actually mean

**#1417 — `SyncBefore` is `NONE` on a resource that has already been accessed.**
`NONE` is only legal for a resource entering from the creation/undefined layout.
`CommandListGroup::plan_resources` decides this via `PlannedResource::from_undefined`.
If you see this, the group believes it is the first-ever toucher of a resource
that something already used — look at whether an earlier group touched it
without being accounted for, or whether the resource was reset/recreated without
its tracked state being reset.

**#1422 — missing initializing discard on a resource's first write.**
The group holding the first-ever write owns the discard
(`PlannedResource::owns_discard`). This fires when the first write happens
somewhere the planner didn't identify as the first write — commonly a resource
written by a pass that isn't declared as a writer, or an aliased/transient
resource whose first use after aliasing wasn't treated as initializing.

**#1334 — `LayoutBefore` doesn't match the resource's actual layout.**
The most common structural cause is a barrier group being emitted twice: the
second run declares a `LayoutBefore` the first already moved past. `CmdListOperation::closed`
exists specifically to prevent this — it guards against reserving
`barriers_after` a second time when something appends to a list after
`CommandList::end()` closed it. Also check for a resource used mid-frame in
`COMMON` because it was created or deserialized lazily during a render pass
rather than at init.

## How to work a specific error

1. Get the full message text, not just the ID — it names the resource and the
   two layouts, which localises the problem far faster than the ID alone.
2. Identify which pass or list touches that resource around the failure.
3. Determine what the compiler *believed*: is this the group's first touch, is
   there an entry state, is the resource declared as read or written by that
   pass?
4. Compare against what actually happens. The mismatch is the bug.
5. Fix the declaration — the pass's read/write flags, the entry state, the
   resting layout, or the planning decision — not the emitted barrier.

## Verifying a fix

Barrier reasoning is not self-verifying, so a fix is unproven until the debug
layer agrees. Use the `build-and-validate` skill to build `Debug-D3D12`, run,
and count errors by ID before and after. Report the counts.

Be careful about one failure mode in particular: a change that suppresses an
error by making a resource stop being tracked (rather than by making its
transitions correct) shows up as a clean log while leaving a genuine missing
barrier. If an error count drops sharply, confirm the resource is still being
transitioned rather than merely no longer being visited.
