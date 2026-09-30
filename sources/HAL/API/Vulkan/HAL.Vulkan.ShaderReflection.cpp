module HAL:ShaderCompiler;

import Core;
import DXCompiler;
import :Slots;       // get_slot
import :Autogen;     // get_table_index

// Vulkan implementation of the reflect_shader() seam declared in
// DXC/DXC.ShaderCompiler.cpp. D3D12 reads slot usage from the DXIL reflection
// blob, by the names of the "pass_X" cbuffers the shader reads. SPIR-V keeps
// neither the blob nor those names after optimization: a slot table becomes
// `static pass_X = { _hal_push.sN }`, and all that survives is the read of
// push-constant member N (the slot's root index, get_table_index).
//
// So slot usage is the intersection of two sets:
//   * the slots whose generated header the shader includes (autogen/<Slot>.h,
//     recorded by the include handler) -- names, but over-inclusive;
//   * the push-constant members the SPIR-V actually reads -- exact, but only
//     root indices, which several slots share.
// Empty slot usage leaves commit_tables recording no resource at all, so the
// barrier compiler emits nothing and every hazard inside a pass goes
// unsynchronized.

namespace HAL
{
    namespace
    {
        // Push-constant member indices read anywhere in a SPIR-V module.
        std::set<uint32_t> read_push_constant_members(const std::vector<std::byte>& blob)
        {
            std::set<uint32_t> members;
            if (blob.size() < 20 || blob.size() % 4) return members;

            const uint32_t* words = reinterpret_cast<const uint32_t*>(blob.data());
            const size_t    count = blob.size() / 4;
            if (words[0] != 0x07230203u) return members;   // SPIR-V magic

            constexpr uint32_t OpConstant            = 43;
            constexpr uint32_t OpVariable            = 59;
            constexpr uint32_t OpLoad                = 61;
            constexpr uint32_t OpAccessChain         = 65;
            constexpr uint32_t OpInBoundsAccessChain = 66;
            constexpr uint32_t OpCompositeExtract    = 81;
            constexpr uint32_t StorageClassPushConstant = 9;

            std::set<uint32_t> push_vars, push_loads;
            std::unordered_map<uint32_t, uint32_t> constants;

            // Two passes: constants and variables are declared before the
            // function bodies that use them, but keep it order-independent.
            for (int pass = 0; pass < 2; pass++)
            {
                for (size_t i = 5; i < count;)
                {
                    const uint32_t op  = words[i] & 0xFFFFu;
                    const uint32_t len = words[i] >> 16;
                    if (len == 0 || i + len > count) break;
                    const uint32_t* w = words + i;

                    if (pass == 0)
                    {
                        if (op == OpConstant && len >= 4) constants[w[2]] = w[3];
                        if (op == OpVariable && len >= 4 && w[3] == StorageClassPushConstant) push_vars.insert(w[2]);
                    }
                    else
                    {
                        if ((op == OpAccessChain || op == OpInBoundsAccessChain) && len >= 5 && push_vars.contains(w[3]))
                        {
                            if (auto it = constants.find(w[4]); it != constants.end())
                                members.insert(it->second);
                        }
                        if (op == OpLoad && len >= 4 && push_vars.contains(w[3]))
                            push_loads.insert(w[2]);
                        if (op == OpCompositeExtract && len >= 5 && push_loads.contains(w[3]))
                            members.insert(w[4]);
                    }
                    i += len;
                }
            }
            return members;
        }
    }

    void reflect_shader(IDxcUtils* /*library*/, const DxcBuffer& /*reflectionBuffer*/,
                        const std::string& entry_point, const std::set<std::string>& included_names,
                        CompiledShader& blob_str)
    {
        if (entry_point.empty()) return;   // DXR libraries: not used on Vulkan yet

        blob_str.functions.emplace_back();
        auto& f = blob_str.functions.back();
        f.name  = entry_point;
        f.wname = convert(f.name);

        const auto read = read_push_constant_members(blob_str.blob);
        for (auto& name : included_names)
        {
            auto slot = get_slot(name);
            if (slot && read.contains(get_table_index(*slot)))
                f.slots.merge(*slot);
        }
    }

    // Vulkan: compile HLSL to SPIR-V via DXC.
    // -fvk-use-dx-layout          : preserve cbuffer/structured-buffer memory layout
    // -fvk-b-shift 0 all          : CBVs keep their register number as the binding index
    // -fvk-t-shift 128 all        : SRVs start at binding 128
    // -fvk-u-shift 256 all        : UAVs start at binding 256
    // -fvk-s-shift 384 all        : Samplers start at binding 384
    // (Must match the VkDescriptorSetLayoutBinding layout in Device::init)
    // The heap flags take <binding> <set> -- binding first:
    // -fvk-bind-resource-heap 0 0 : SM6.6 ResourceDescriptorHeap → set 0, binding 0 (MUTABLE)
    //                               All types (CBV/SRV/UAV/buffer) land at binding 0.
    // -fvk-bind-counter-heap 2 0  : Append/Consume counters → set 0, binding 2, same index
    // -fvk-bind-sampler-heap 0 1  : SM6.6 SamplerDescriptorHeap → set 1, binding 0
    // Slot push constants: slot HLSL uses [[vk::push_constant]] ConstantBuffer<_CB_X>
    //                      with member [[vk::offset(slot.id*4)]].  DXC places the member
    //                      at byte slot.id*4 in the SPIRV push constant block, matching
    //                      the HAL's vkCmdPushConstants offset = slot*4.
    //                      [[vk::push_constant]] is silently ignored by the DXIL path.
    //
    // float16_tN aliases: -enable-16bit-types is intentionally suppressed for SPIRV
    // (Vulkan SPIRV spec requires image sampled types to be 32-bit).  Shaders that
    // declare float16_tN variables/params still need the types to exist, so we
    // define them as float aliases via the preprocessor — arithmetic stays correct,
    // image declarations become Texture2D<floatN> which is valid SPIRV.
    std::vector<std::wstring> get_extra_compile_args(const std::string& /*target*/)
    {
        return {
            L"-spirv",
            L"-fspv-target-env=vulkan1.3",
            L"-fvk-use-dx-layout",
            L"-fvk-b-shift", L"0",   L"all",
            L"-fvk-t-shift", L"128", L"all",
            L"-fvk-u-shift", L"256", L"all",
            L"-fvk-s-shift", L"384", L"all",
            L"-fvk-bind-resource-heap", L"0", L"0",
            L"-fvk-bind-counter-heap",  L"2", L"0",
            L"-fvk-bind-sampler-heap",  L"0", L"1",
            // Slot ConstantBuffers use [[vk::push_constant]] + [[vk::offset(slot*4)]] in HLSL.
            // No -fvk-push-constant-space needed; the offset is encoded directly in the struct.
            // float16_t aliases — map 16-bit scalar/vector types to 32-bit
            // so shaders compile without -enable-16bit-types (blocked for SPIRV;
            // see DXC.ShaderCompiler.cpp).  Texture2D<float16_tN> → Texture2D<floatN>.
            L"-Dfloat16_t=float",
            L"-Dfloat16_t2=float2",
            L"-Dfloat16_t3=float3",
            L"-Dfloat16_t4=float4",
            L"-Dint16_t=int",
            L"-Dint16_t2=int2",
            L"-Dint16_t3=int3",
            L"-Dint16_t4=int4",
            L"-Duint16_t=uint",
            L"-Duint16_t2=uint2",
            L"-Duint16_t3=uint3",
            L"-Duint16_t4=uint4",
        };
    }
}
