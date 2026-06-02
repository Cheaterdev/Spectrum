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
    class IndirectCommand
    {
        IndirectCommand(const UsedSlots& slots) : slots(slots) {}
    public:
        UsedSlots slots;

        IndirectCommand() = default;

        template<class T>
        static void process_one(UsedSlots& slots, uint& total_size)
        {
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
            UsedSlots slots;
            uint total_size = 0;
            (process_one<Args>(slots, total_size), ...);
            // Phase 4: create real Vulkan indirect buffer layout.
            return IndirectCommand(slots);
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
