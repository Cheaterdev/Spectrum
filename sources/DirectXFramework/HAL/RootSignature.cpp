#include "pch_dx.h"


import RootSignature;
import Log;
import Device;

namespace DX12
{

    void init_parameters(std::vector<CD3DX12_ROOT_PARAMETER1>& parameters, std::list<CD3DX12_DESCRIPTOR_RANGE1>& ranges,const  RootSignatureDesc& desc)
    {
		for (auto e : desc.parameters)
		{
            parameters.emplace_back();
			auto& param = parameters.back();

			MyVariant v = e.second;

			auto contants = v.try_get<DescriptorConstants>();
			auto table = v.try_get<DescriptorTable>();
			auto cbv = v.try_get<DescriptorConstBuffer>();
			auto srv = v.try_get<DescriptorSRV>();
			auto uav = v.try_get<DescriptorUAV>();


			if (contants)
			{
				param.InitAsConstants(contants->count, contants->buffer_offset, 0, static_cast<D3D12_SHADER_VISIBILITY>(contants->visibility));
			}

			if (cbv)
			{
				param.InitAsConstantBufferView(cbv->offset, cbv->space, D3D12_ROOT_DESCRIPTOR_FLAGS::D3D12_ROOT_DESCRIPTOR_FLAG_DATA_VOLATILE, static_cast<D3D12_SHADER_VISIBILITY>(cbv->visibility));
			}

			if (srv)
			{
				param.InitAsShaderResourceView(srv->offset, srv->space, D3D12_ROOT_DESCRIPTOR_FLAGS::D3D12_ROOT_DESCRIPTOR_FLAG_DATA_VOLATILE, static_cast<D3D12_SHADER_VISIBILITY>(srv->visibility));
			}

			if (uav)
			{
				param.InitAsUnorderedAccessView(uav->offset, uav->space, D3D12_ROOT_DESCRIPTOR_FLAGS::D3D12_ROOT_DESCRIPTOR_FLAG_DATA_VOLATILE, static_cast<D3D12_SHADER_VISIBILITY>(uav->visibility));
			}

			if (table)
			{
				ranges.emplace_back();
				auto& range = ranges.back();

				range.Init(static_cast<D3D12_DESCRIPTOR_RANGE_TYPE>(table->range), table->count, table->offset, table->space, D3D12_DESCRIPTOR_RANGE_FLAGS::D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE);
				param.InitAsDescriptorTable(1, &range, static_cast<D3D12_SHADER_VISIBILITY>(table->visibility));
			}

			}

    }

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
		assert(false);
    }

    RootSignature::RootSignature(const RootSignatureDesc& desc, D3D12_ROOT_SIGNATURE_FLAGS flags)
    {

//       if(flags!= D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE) flags |= D3D12_ROOT_SIGNATURE_FLAGS::D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED;
		std::vector<CD3DX12_ROOT_PARAMETER1> parameters;
		std::list<CD3DX12_DESCRIPTOR_RANGE1> ranges;

		init_parameters(parameters, ranges, desc);


        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init_1_1(static_cast<UINT>(parameters.size()), parameters.data(),  static_cast<UINT>(desc.samplers().size()), desc.samplers().data(), flags);
      //  rootSignatureDesc.pStaticSamplers = desc.samplers().data();
      //  rootSignatureDesc.NumStaticSamplers = static_cast<UINT>(desc.samplers().size());
        this->desc = desc;


		
        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;
        TEST(D3D12SerializeVersionedRootSignature(&rootSignatureDesc, &signature, &error));

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