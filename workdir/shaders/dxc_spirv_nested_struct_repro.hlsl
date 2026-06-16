// Minimal repro for DXC SPIRV codegen bug: nested struct type mismatch.
//
// Error:
//   fatal error: generated SPIR-V is invalid:
//   OpAccessChain result type <id> '3[%Inner]' (OpTypeStruct) does not match
//   the type that results from indexing into the base <id> '7[%Inner_0]'
//   (OpTypeStruct). (The types must be the exact same Id, so the two types
//   referenced are slightly different)
//
// DXC emits two structurally-identical OpTypeStruct definitions for Inner:
//   %Inner   — layout-decorated (Offset members), used inside ConstantBuffer<Outer>
//   %Inner_0 — plain (no decorations), used when Outer is copied to a value variable
//
// When g.GetInner() accesses the field on the value-typed Outer (using %Inner_0
// as the struct type), but the OpAccessChain into the backing ResourceDescriptorHeap
// storage uses %Inner (the decorated type), the validator rejects the type mismatch.
//
// The struct-as-value copy (static const Outer g = CreateOuter()) is what creates
// the split: DXC needs two type IDs for Inner but then uses them inconsistently.
//
// The array variant of this bug is in dxc_spirv_array_repro.hlsl.
//
// Compile command:
//   dxc -spirv -T cs_6_6 -E CS -fvk-bind-resource-heap 0 0 dxc_spirv_nested_struct_repro.hlsl
//
// DXC version that reproduces: 1.9.2602 (and earlier)

struct Inner { uint4 a; uint4 b; };
struct Outer { uint handle; Inner inner; Inner GetInner() { return inner; } };

// Implicit conversion from ConstantBuffer<Outer> to Outer creates a value copy.
// A direct register binding (ConstantBuffer<Outer> g : register(b0)) does not trigger
// the bug — ResourceDescriptorHeap loading is required.
ConstantBuffer<Outer> CreateOuter() { return ResourceDescriptorHeap[0]; }
static const Outer g = CreateOuter();

static const uint4 VA = g.GetInner().a;
static const uint4 VB = g.GetInner().b;

RWBuffer<uint> Out : register(u0);

[numthreads(1, 1, 1)]
void CS(uint3 tid : SV_DispatchThreadID)
{
    Out[0] = (VA + VB).x + tid.x;
}
