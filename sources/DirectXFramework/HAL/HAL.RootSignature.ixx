export module HAL:RootSignature;

export import Data;
export import Math;
export import Log;
export import :Types;
export import :Sampler;
export import :Utils;
export import stl.core;

export import :API.RootSignature;
export import :API.Device;

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

		DescriptorTable() = default;
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

		DescriptorConstBuffer() = default;
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

		DescriptorSRV() = default;
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

		DescriptorUAV() = default;
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

		DescriptorConstants() = default;
	};

	enum class RootSignatureType : uint
	{
		Global,
		Local
	};



	struct RootSignatureDesc
	{
		//friend class RootSignature;

		bool local = false;

		using ParameterType = std::variant<DescriptorTable, DescriptorConstBuffer, DescriptorSRV, DescriptorUAV, DescriptorConstants>;
		struct Position
		{
			uint offset;
			uint space;
			GEN_DEF_COMP(Position);
		};
		//struct table_info
		//{
		//	int count;
		//	bool fixed = true;
		//	DescriptorRange type;
		//};

		struct helper
		{
			ParameterType& v;

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

			helper(uint index, ParameterType& v) : index(index), v(v)
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

	//	std::map<uint, table_info> tables;

		void set_type(RootSignatureType type)
		{
			this->type = type;
		}
	public:
		RootSignatureType type = RootSignatureType::Global;
		std::map<int, ParameterType> parameters;
		std::map<Position, SamplerDesc> sampler_map;
	};

	class RootSignature: public API::RootSignature
	{
		RootSignatureDesc desc;
	public:
		using ptr = std::shared_ptr<RootSignature>;
		virtual~RootSignature() = default;
		RootSignature(Device& device, const RootSignatureDesc& desc);
		const RootSignatureDesc& get_desc() const
		{
			return desc;
		}
	};

}


