module HAL:HLSL;

import :Descriptors;
import :DescriptorHeap;
import :Resource;
import :Resource.Buffer;
import Core;

using namespace HAL;

namespace HLSL
{
	void RaytracingAccelerationStructure::create(const Resource::ptr& resource)
	{
		ASSERT(check(resource->get_desc().Flags & ResFlags::Raytracing));
		HAL::Views::ShaderResource desc = { resource, HAL::Format::UNKNOWN, HAL::Views::ShaderResource::Raytracing {0} };
		Handle::operator=(desc);
	}

	void FeedbackTexture2DMip::create(const Resource::ptr& texture, const Resource::ptr& feedback)
	{
		HAL::Views::UnorderedAccess desc = { texture, Format::UNKNOWN, HAL::Views::UnorderedAccess::TextureFeedback {feedback} };
		Handle::operator=(desc);
	}

	void ByteAddressBuffer::create(const Resource::ptr& resource, uint64 offset, uint64 size)
	{
		auto buffer_desc = resource->get_desc().as_buffer();
		if (size == 0) size = static_cast<uint>(buffer_desc.SizeInBytes);
		HAL::Views::ShaderResource desc = { resource, Format::R32_TYPELESS, HAL::Views::ShaderResource::Buffer {(uint)offset / 4, (uint)size / 4, 0, true} };
		Handle::operator=(desc);
	}

	void RWByteAddressBuffer::create(const Resource::ptr& resource, uint64 offset, uint64 size)
	{
		auto buffer_desc = resource->get_desc().as_buffer();
		if (size == 0) size = static_cast<uint64>(buffer_desc.SizeInBytes);
		HAL::Views::UnorderedAccess desc = { resource, Format::R32_TYPELESS, HAL::Views::UnorderedAccess::Buffer {(uint)offset / 4, (uint)size / 4, 0, true, 0, nullptr} };
		Handle::operator=(desc);
	}
}
