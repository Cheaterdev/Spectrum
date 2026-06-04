// Minimal repro for DXC SPIRV codegen bug.
//
// Error:
//   fatal error: generated SPIR-V is invalid:
//   OpAccessChain result type <id> 'N[%_arr_uint_uint_4]' (OpTypeArray) does not
//   match the type that results from indexing into the base <id>
//   'M[%_arr_uint_uint_4_0]' (OpTypeArray).
//   (The types must be the exact same Id, so the two types referenced are
//   slightly different)
//
// DXC emits two structurally-identical OpTypeArray definitions for uint[4]
// — one for the struct field, one for the heap element type — and then uses
// them inconsistently in the OpAccessChain operands/result type.
//
// Compile command:
//   dxc -spirv -T cs_6_6 -E CS -fvk-bind-resource-heap 0 0 dxc_spirv_array_repro.hlsl
//
// DXC version that reproduces: 1.9.2602 (and earlier)

struct Data
{
    uint arr[4];   // <-- any fixed-size array in the struct is enough to trigger it
};

RWBuffer<uint> Out : register(u0);

[numthreads(1, 1, 1)]
void CS(uint3 tid : SV_DispatchThreadID)
{
    // Accessing the struct as a ConstantBuffer via ResourceDescriptorHeap
    // is what triggers the bug.  Direct register binding works fine:
    //   ConstantBuffer<Data> d : register(b0);  // <-- no bug
    ConstantBuffer<Data> d = ResourceDescriptorHeap[0];  // <-- bug

    // Runtime index so the compiler can't constant-fold the access away.
    Out[0] = d.arr[tid.x & 3];
}
