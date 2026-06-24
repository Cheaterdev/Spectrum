export module HAL:RootSignature;

import Core;

import :Utils;
import :Types;
import :Sampler;


import :API.RootSignature;
import :Device;
import :Enums;
import :Concepts;

export namespace HAL
{
	struct DescriptorTable
	{
		DescriptorRange range;
		ShaderVisibility visibility;
		uint offset;
		uint count;
		uint space;
		DescriptorTable(DescriptorRange _range, ShaderVisibility _visibility, uint _offset, uint _count, uint space = 0);
		DescriptorTable() = default;
	};

	struct DescriptorConstBuffer
	{
		DescriptorRange range;
		ShaderVisibility visibility;
		uint offset;
		uint space;

		DescriptorConstBuffer(uint _offset, ShaderVisibility _visibility = ShaderVisibility::ALL, uint space = 0);
		DescriptorConstBuffer() = default;
	};

	struct DescriptorSRV
	{
		ShaderVisibility visibility;
		uint offset;
		uint space;

		DescriptorSRV(uint _offset, ShaderVisibility _visibility = ShaderVisibility::ALL, uint space = 0);
		DescriptorSRV() = default;
	};

	struct DescriptorUAV
	{
		ShaderVisibility visibility;
		uint offset;
		uint space;

		DescriptorUAV(uint _offset, ShaderVisibility _visibility = ShaderVisibility::ALL, uint space = 0);
		DescriptorUAV() = default;
	};

	struct DescriptorConstants
	{
		ShaderVisibility visibility;
		uint count;
		uint offset;
		uint space;

		DescriptorConstants(uint offset, uint _count, ShaderVisibility _visibility = ShaderVisibility::ALL, uint space = 0);
		DescriptorConstants() = default;
	};

	enum class RootSignatureType : uint
	{
		Global,
		Local
	};



	struct RootSignatureDesc
	{
		using ParameterType = std::variant<DescriptorTable, DescriptorConstBuffer, DescriptorSRV, DescriptorUAV, DescriptorConstants>;
		struct Position
		{
			uint offset;
			uint space;
			GEN_DEF_COMP(Position);
		};

		struct helper
		{
			ParameterType& v;
			uint index;

			void operator=(const DescriptorTable& table);
			void operator=(const DescriptorConstBuffer& table);
			void operator=(const DescriptorSRV& table);
			void operator=(const DescriptorUAV& table);
			void operator=(const DescriptorConstants& table);

			helper(uint index, ParameterType& v);
			helper(const helper& h);
		};

		helper operator[](uint i);
		void remove(uint i);
		const std::map<Position, SamplerDesc>& samplers() const;
		void set_sampler(uint i, uint space, ShaderVisibility visibility, SamplerDesc desc);
		void set_type(RootSignatureType type);
	public:
		RootSignatureType type = RootSignatureType::Global;
		std::map<int, ParameterType> parameters;
		std::map<Position, SamplerDesc> sampler_map;
	};

	class RootSignature: public API::RootSignature
	{
	protected:
		HAL::Device& device;
		RootSignatureDesc desc;
	public:
		using ptr = std::shared_ptr<RootSignature>;
		virtual~RootSignature() = default;
		RootSignature(Device& device, const RootSignatureDesc& desc);
		const RootSignatureDesc& get_desc() const;
		Device& get_device();
	};





	class RootLayout :public RootSignature
	{
		

		template<class T>
		void process_one_sig(RootSignatureDesc& desc) const
		{
				desc.remove(T::Slot::ID);
		}

		template<>
		void process_one_sig<int>(RootSignatureDesc& desc) const
		{
		
		}

	public:
		using ptr = std::shared_ptr<RootLayout>;
		const Layouts layout;
		RootLayout(HAL::Device& device, const RootSignatureDesc& desc, Layouts layout);

		template< class ...A>
		RootSignature::ptr create_global_signature() const
		{
			RootSignatureDesc desc = get_desc();

			(process_one_sig<A>(desc), ...);

			return std::make_shared<RootSignature>(device, desc);
		}

	};

}


