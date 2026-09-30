export module HAL:API.IndirectCommand;

import vulkan;
import :Types;
import :Utils;
import :RootSignature;
import :Slots;

// Vulkan indirect command — VkDrawIndexedIndirectCommand / VkDispatchIndirectCommand.
// For Phase 0: stub that mirrors the D3D12 IndirectCommand interface so
// common code that creates IndirectCommands compiles unchanged.
// Phase 4: implement real vkCmdDrawIndexedIndirect / vkCmdDispatchIndirect.

namespace HAL
{
    // Vulkan-side helpers (equivalent to D3D12's create_indirect_for chain)
    // These keep the same API so HAL::IndirectCommand::create_command<Args...>
    // can be used without #ifdefs.
}

export namespace HAL
{
    // Which vkCmd*Indirect the command buffer feeds.  The engine's argument
    // structs (SIG.ixx) are laid out identically to Vulkan's indirect command
    // structs, so no command signature is needed — only the kind + stride.
    enum class IndirectKind : uint { Unknown, DrawIndexed, Dispatch, DispatchMesh };

    template<class T>
    constexpr IndirectKind indirect_kind_of()
    {
        if      constexpr (std::is_same_v<T, DrawIndexedArguments>)  return IndirectKind::DrawIndexed;
        else if constexpr (std::is_same_v<T, DispatchArguments>)     return IndirectKind::Dispatch;
        else if constexpr (std::is_same_v<T, DispatchMeshArguments>) return IndirectKind::DispatchMesh;
        else                                                         return IndirectKind::Unknown;
    }

    class IndirectCommand
    {
        IndirectCommand(const UsedSlots& slots, IndirectKind kind, uint stride, uint args_offset)
            : slots(slots), kind(kind), stride(stride), args_offset(args_offset) {}
    public:
        UsedSlots    slots;
        IndirectKind kind   = IndirectKind::Unknown; // selects the vkCmd*Indirect call
        uint         stride = 0;                      // bytes between commands in the buffer
        // Where the hardware arguments start inside a command. A slot value in
        // front of them (DrawMeshCommand's draw_id) is D3D12's root constant;
        // Vulkan skips it and the shader reads it back by DrawIndex.
        uint         args_offset = 0;

        IndirectCommand() = default;

        template<class T>
        static void process_one(UsedSlots& slots, uint& total_size, IndirectKind& kind, uint& args_offset)
        {
            constexpr IndirectKind k = indirect_kind_of<T>();
            if constexpr (k != IndirectKind::Unknown)
            {
                kind        = k;
                args_offset = total_size;
            }

            if constexpr (HasID<T>)
            {
                slots.merge(T::ID);
                total_size += sizeof(uint);
            }
            else
                total_size += sizeof(Underlying<T>);
        }

        template<class... Args>
        static IndirectCommand create_command(Device& /*device*/,
                                              RootSignature* /*layout*/ = nullptr)
        {
            UsedSlots    slots;
            uint         total_size  = 0;
            uint         args_offset = 0;
            IndirectKind kind = IndirectKind::Unknown;
            (process_one<Args>(slots, total_size, kind, args_offset), ...);
            return IndirectCommand(slots, kind, total_size, args_offset);
        }

        template<class... Args>
        static IndirectCommand create_command_layout(Device& device)
        {
            return create_command<Args...>(device, nullptr);
        }

        template<class... Args>
        static IndirectCommand create_command_layout(Device& device, auto layout)
        {
            return create_command<Args...>(device, nullptr);
        }
    };
}
