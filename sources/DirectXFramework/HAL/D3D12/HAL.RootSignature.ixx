export module HAL:RootSignature;

import Data;
import d3d12;
import Math;
import Log;
import :Types;
import :Sampler;
import :Device;
import :Utils;
import stl.core;

export namespace HAL
{


	struct DescriptorTable
	{
		DescriptorRange range;
		ShaderVisibility visibility;
		uint offset;
		uint count;
		uint space;
		DescriptorTable(DescriptorRange _range,
			ShaderVisibility _visibility,
			uint _offset,
			uint _count, uint space = 0) : range(_range), space(space), visibility(_visibility), offset(_offset), count(_count)
		{
		}
	};

	struct DescriptorConstBuffer
	{
		DescriptorRange range;
		ShaderVisibility visibility;
		uint offset;
		uint space;

		DescriptorConstBuffer(uint _offset, ShaderVisibility _visibility = ShaderVisibility::ALL,
			uint space = 0
		) : visibility(_visibility), offset(_offset), space(space)
		{
		}
	};

	struct DescriptorSRV
	{
		ShaderVisibility visibility;
		uint offset;
		uint space;


		DescriptorSRV(uint _offset, ShaderVisibility _visibility = ShaderVisibility::ALL,
			uint space = 0) : visibility(_visibility), offset(_offset), space(space)
		{
		}
	};

	struct DescriptorUAV
	{
		ShaderVisibility visibility;
		uint offset;
		uint space;


		DescriptorUAV(uint _offset, ShaderVisibility _visibility = ShaderVisibility::ALL,
			uint space = 0) : visibility(_visibility), offset(_offset), space(space)
		{
		}
	};

	struct DescriptorConstants
	{
		ShaderVisibility visibility;
		uint count;
		uint buffer_offset;
		DescriptorConstants(uint _buffer_offset, uint _count, ShaderVisibility _visibility = ShaderVisibility::ALL
		) : buffer_offset(_buffer_offset), visibility(_visibility), count(_count)
		{
		}
	};

	enum class RootSignatureType : uint
	{
		Global,
		Local
	};
	struct RootSignatureDesc
	{
		friend class RootSignature;

		bool local = false;

		struct Position
		{
			uint offset;
			uint space;
			GEN_DEF_COMP(Position);
		};
		struct table_info
		{
			int count;
			bool fixed = true;
			DescriptorRange type;
		};

		struct helper
		{
			MyVariant& v;

			uint index;


			void operator=(const DescriptorTable& table)
			{
				v = table;
			}

			void operator=(const DescriptorConstBuffer& table)
			{
				v = table;
			}

			void operator=(const DescriptorSRV& table)
			{
				v = table;
			}

			void operator=(const DescriptorUAV& table)
			{
				v = table;
			}

			void operator=(const DescriptorConstants& table)
			{
				v = table;
			}

			helper(uint index, MyVariant& v) : index(index), v(v)
			{
			}

			helper(const helper& h) : index(h.index), v(h.v)
			{
			}
		};

		helper operator[](uint i)
		{
			return helper(i, parameters[i]);
		}

		void remove(uint i)
		{
			parameters.erase(i);
		}

		const std::map<Position, SamplerDesc>& samplers() const
		{
			return sampler_map;
		}

		void set_sampler(uint i, uint space, ShaderVisibility visibility, SamplerDesc desc)
		{
			sampler_map[{i, space}] = desc;
		}

		std::map<uint, table_info> tables;

		void set_type(RootSignatureType type)
		{
			this->type = type;
		}
	public:
		RootSignatureType type = RootSignatureType::Global;
		std::map<int, MyVariant> parameters;
		std::map<Position, SamplerDesc> sampler_map;
	};


	class RootSignature
	{
		D3D::RootSignature m_rootSignature;
		RootSignatureDesc desc;

	public:
		using ptr = std::shared_ptr<RootSignature>;


		virtual~RootSignature() = default;


		RootSignature(Device& device, const RootSignatureDesc& desc);

		D3D::RootSignature get_native();

		const RootSignatureDesc& get_desc() const
		{
			return desc;
		}
	};




	void init_parameters(std::vector<CD3DX12_ROOT_PARAMETER1>& parameters, std::list<CD3DX12_DESCRIPTOR_RANGE1>& ranges, const  RootSignatureDesc& desc)
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

}



namespace HAL
{

	RootSignature::RootSignature(Device& device, const RootSignatureDesc& desc)
	{
		std::vector<CD3DX12_ROOT_PARAMETER1> parameters;
		std::list<CD3DX12_DESCRIPTOR_RANGE1> ranges;

		init_parameters(parameters, ranges, desc);

		std::vector<D3D12_STATIC_SAMPLER_DESC> descs;

		auto& samplers = desc.samplers();


		for (const auto& [position, desc] : samplers)
		{
			descs.emplace_back();
			auto& sampler = descs.back();
			sampler.AddressU = to_native(desc.AddressU);
			sampler.AddressV = to_native(desc.AddressV);
			sampler.AddressW = to_native(desc.AddressW);

			if (desc.BorderColor[0] == 0)
				sampler.BorderColor = desc.BorderColor[3] == 0 ? D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK : D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
			else
				sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;

			sampler.ComparisonFunc = to_native(desc.ComparisonFunc);
			sampler.Filter = to_native(desc.MinFilter, desc.MagFilter, desc.MipFilter, desc.ComparisonFunc);
			sampler.MaxAnisotropy = desc.MaxAnisotropy;
			sampler.MaxLOD = desc.MaxLOD;
			sampler.MinLOD = desc.MinLOD;
			sampler.MipLODBias = desc.MipLODBias;
			sampler.RegisterSpace = position.space;
			sampler.ShaderRegister = position.offset;

		}

		D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAGS::D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED;


		if (desc.type == RootSignatureType::Local)
		{
			flags = D3D12_ROOT_SIGNATURE_FLAGS::D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;
		}
		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init_1_1(static_cast<UINT>(parameters.size()), parameters.data(), static_cast<UINT>(descs.size()), descs.data(), flags);
		this->desc = desc;

		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;
		auto hr = (D3D12SerializeVersionedRootSignature(&rootSignatureDesc, &signature, &error));

		if (error)
		{
			Log::get().crash_error(static_cast<char*>(error->GetBufferPointer()));
		}

		TEST((&device), hr);

		TEST((&device),device.native_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));
	}

	D3D::RootSignature RootSignature::get_native()
	{
		return m_rootSignature;
	}
}