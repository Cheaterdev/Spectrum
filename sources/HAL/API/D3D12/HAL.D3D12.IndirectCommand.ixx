export module HAL:API.IndirectCommand;

import :Types;
import :Utils;
import :RootSignature;
import :Slots;
import d3d12;

namespace HAL
{
    // D3D12-layer conversions: argument types in SIG.ixx are API-independent,
    // so the D3D12_INDIRECT_ARGUMENT_DESC mappings live here instead.

    inline D3D12_INDIRECT_ARGUMENT_DESC create_indirect_for(DispatchArguments*)
    {
        D3D12_INDIRECT_ARGUMENT_DESC desc;
        desc.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH;
        return desc;
    }

    inline D3D12_INDIRECT_ARGUMENT_DESC create_indirect_for(DispatchMeshArguments*)
    {
        D3D12_INDIRECT_ARGUMENT_DESC desc;
        desc.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH_MESH;
        return desc;
    }

    inline D3D12_INDIRECT_ARGUMENT_DESC create_indirect_for(DrawIndexedArguments*)
    {
        D3D12_INDIRECT_ARGUMENT_DESC desc;
        desc.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;
        return desc;
    }

    // Fallback for slot types that still carry their own create_indirect()
    template<class T>
    D3D12_INDIRECT_ARGUMENT_DESC create_indirect_for(T*)
    {
        return T::create_indirect();
    }

    template<class T>
    D3D12_INDIRECT_ARGUMENT_DESC get_indirect_arg()
    {
        return create_indirect_for(static_cast<T*>(nullptr));
    }
}

export namespace HAL
{
    class IndirectCommand
    {
        IndirectCommand(D3D::CommandSignature command_signature, const UsedSlots& slots);

        template <class T>
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

    public:
        UsedSlots slots;
        // TODO: make private
        D3D::CommandSignature command_signature;

        IndirectCommand();

        template <class... Args>
        static IndirectCommand create_command(Device& device, RootSignature* layout = nullptr)
        {
            D3D::CommandSignature command_signature;

            D3D12_INDIRECT_ARGUMENT_DESC argumentDescs[] = {
                get_indirect_arg<Args>()...
            };

            UsedSlots slots;
            uint total_size = 0;
            (process_one<Args>(slots, total_size), ...);

            D3D12_COMMAND_SIGNATURE_DESC commandSignatureDesc = {};
            commandSignatureDesc.pArgumentDescs = argumentDescs;
            commandSignatureDesc.NumArgumentDescs = _countof(argumentDescs);
            commandSignatureDesc.ByteStride = total_size; //(0 + ... + sizeof(Underlying<Args>));

            //    ASSERT(commandSignatureDesc.ByteStride == size);
            TEST(device,
                 device.native_device->CreateCommandSignature(&commandSignatureDesc, layout ? layout->get_native().Get()
                     : nullptr, IID_PPV_ARGS(&command_signature)));

            return IndirectCommand(command_signature, slots);
        }

        template <class... Args>
        static IndirectCommand create_command_layout(Device& device)
        {
            return create_command<Args...>(device, nullptr);
        }

        template <class... Args>
        static IndirectCommand create_command_layout(Device& device, auto layout)
        {
            return create_command<Args...>(device, device.get_engine_root_layout_holder().GetSignature(layout).get());
        }
    };
}