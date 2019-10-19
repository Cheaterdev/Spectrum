#include "pch.h"

namespace DX12
{
    RootSignature::RootSignature(std::initializer_list<DescriptorTable> table)
    {
        UINT i = 0;

        for (auto && e : table)
        {
            desc[i++] = e;
        }

        finalize();
    }

    void  RootSignature::finalize()
    {
        CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init(static_cast<UINT>(desc.parameters.size()), desc.parameters.data(), 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;
        TEST(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
        TEST(Device::get().get_native_device()->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));
    }

    RootSignature::RootSignature(const RootSignatureDesc& desc, D3D12_ROOT_SIGNATURE_FLAGS flags)
    {
        CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init(static_cast<UINT>(desc.parameters.size()), desc.parameters.data(), 0, nullptr, flags);
        rootSignatureDesc.pStaticSamplers = desc.samplers().data();
        rootSignatureDesc.NumStaticSamplers = static_cast<UINT>(desc.samplers().size());
        this->desc = desc;
        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;
        (D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));

        if (error)
        {
            Log::get().crash_error(static_cast<char*>(error->GetBufferPointer()));
        }

        TEST(Device::get().get_native_device()->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));
    }

    ComPtr<ID3D12RootSignature> RootSignature::get_native()
    {
        return m_rootSignature;
    }
}