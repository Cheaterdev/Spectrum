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
		

		// Register space for the placeholders below; no shader declares
		// anything this high.
		static constexpr uint LocalSlotPlaceholderSpace = 1000;

		// A local slot must leave the global signature, but not by erasing it:
		// parameters are flattened in key order into the native signature, so
		// an erased slot shifts every later slot one native index down while
		// binding still addresses them by slot ID. That was latent while only
		// never-bound slots followed MaterialData, and surfaced as D3D12 #709
		// once VSMShadow was added after it. A never-set 1-DWORD constant in
		// its own register space keeps slot ID == native index.
		template<class T>
		void process_one_sig(RootSignatureDesc& desc) const
		{
			desc[T::Slot::ID] = DescriptorConstants(0, 1, ShaderVisibility::ALL, LocalSlotPlaceholderSpace + T::Slot::ID);
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


