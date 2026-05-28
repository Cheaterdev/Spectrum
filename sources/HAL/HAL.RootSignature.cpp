module HAL:RootSignature;
import Core;
import :Types;

namespace HAL
{

	DescriptorTable::DescriptorTable(DescriptorRange _range, ShaderVisibility _visibility, uint _offset, uint _count, uint space)
		: range(_range), space(space), visibility(_visibility), offset(_offset), count(_count)
	{
	}

	DescriptorConstBuffer::DescriptorConstBuffer(uint _offset, ShaderVisibility _visibility, uint space)
		: visibility(_visibility), offset(_offset), space(space)
	{
	}

	DescriptorSRV::DescriptorSRV(uint _offset, ShaderVisibility _visibility, uint space)
		: visibility(_visibility), offset(_offset), space(space)
	{
	}

	DescriptorUAV::DescriptorUAV(uint _offset, ShaderVisibility _visibility, uint space)
		: visibility(_visibility), offset(_offset), space(space)
	{
	}

	DescriptorConstants::DescriptorConstants(uint offset, uint _count, ShaderVisibility _visibility, uint space)
		: offset(offset), visibility(_visibility), count(_count), space(space)
	{
	}

	RootSignatureDesc::helper RootSignatureDesc::operator[](uint i)
	{
		return helper(i, parameters[i]);
	}

	void RootSignatureDesc::remove(uint i) { parameters.erase(i); }

	const std::map<RootSignatureDesc::Position, SamplerDesc>& RootSignatureDesc::samplers() const
	{
		return sampler_map;
	}

	void RootSignatureDesc::set_sampler(uint i, uint space, ShaderVisibility visibility, SamplerDesc desc)
	{
		sampler_map[{i, space}] = desc;
	}

	void RootSignatureDesc::set_type(RootSignatureType type) { this->type = type; }

	void RootSignatureDesc::helper::operator=(const DescriptorTable& table) { v = table; }
	void RootSignatureDesc::helper::operator=(const DescriptorConstBuffer& table) { v = table; }
	void RootSignatureDesc::helper::operator=(const DescriptorSRV& table) { v = table; }
	void RootSignatureDesc::helper::operator=(const DescriptorUAV& table) { v = table; }
	void RootSignatureDesc::helper::operator=(const DescriptorConstants& table) { v = table; }

	RootSignatureDesc::helper::helper(uint index, ParameterType& v) : index(index), v(v) {}
	RootSignatureDesc::helper::helper(const helper& h) : index(h.index), v(h.v) {}

	const RootSignatureDesc& RootSignature::get_desc() const { return desc; }
	Device& RootSignature::get_device() { return device; }

	RootLayout::RootLayout(HAL::Device& device, const RootSignatureDesc& desc, Layouts layout)
		: RootSignature(device, desc), layout(layout)
	{
	}

}
