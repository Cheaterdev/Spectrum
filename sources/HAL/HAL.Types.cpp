module HAL:Types;
namespace HAL
{


	ResourceState ResourceState::operator |(const ResourceState& state)const
	{
		ResourceState other = *this;
		other |= state;
		return other;

	}
	const ResourceState& ResourceState::operator |=(const ResourceState& state)
	{

		operation |= state.operation;
		access |= state.access;

		assert(layout == state.layout);
		return *this;
	}



	namespace ResourceStates {

		const  ResourceState INDEX_BUFFER = { BarrierSync::INDEX_INPUT, BarrierAccess::INDEX_BUFFER, TextureLayout::UNDEFINED };
		const   ResourceState COPY_SOURCE = { BarrierSync::COPY, BarrierAccess::COPY_SOURCE, TextureLayout::COPY_SOURCE };
		const   ResourceState COPY_DEST = { BarrierSync::COPY, BarrierAccess::COPY_DEST, TextureLayout::COPY_DEST };

		const  ResourceState PIXEL_SHADER_RESOURCE = { BarrierSync::PIXEL_SHADING, BarrierAccess::SHADER_RESOURCE, TextureLayout::SHADER_RESOURCE };
		const  ResourceState NON_PIXEL_SHADER_RESOURCE = { BarrierSync::NON_PIXEL_SHADING, BarrierAccess::SHADER_RESOURCE, TextureLayout::SHADER_RESOURCE };
		const  ResourceState UNORDERED_ACCESS = { BarrierSync::ALL_SHADING, BarrierAccess::UNORDERED_ACCESS, TextureLayout::UNORDERED_ACCESS };
		const  ResourceState RAYTRACING_STRUCTURE = { BarrierSync::RAYTRACING, BarrierAccess::RAYTRACING_ACCELERATION_STRUCTURE_READ, TextureLayout::UNDEFINED };

		const  ResourceState RAYTRACING_STRUCTURE_WRITE = { BarrierSync::RAYTRACING, BarrierAccess::RAYTRACING_ACCELERATION_STRUCTURE_READ | BarrierAccess::RAYTRACING_ACCELERATION_STRUCTURE_WRITE, TextureLayout::UNDEFINED };
		const  ResourceState INDIRECT_ARGUMENT = { BarrierSync::EXECUTE_INDIRECT, BarrierAccess::INDIRECT_ARGUMENT, TextureLayout::UNDEFINED };

		const  ResourceState RENDER_TARGET = { BarrierSync::RENDER_TARGET, BarrierAccess::RENDER_TARGET, TextureLayout::RENDER_TARGET };
		const  ResourceState DEPTH_STENCIL = { BarrierSync::DEPTH_STENCIL, BarrierAccess::DEPTH_STENCIL_WRITE /*| BarrierAccess::DEPTH_STENCIL_READ*/,TextureLayout::DEPTH_STENCIL_WRITE /*| TextureLayout::DEPTH_STENCIL_READ */};

		const  ResourceState CONSTANT_BUFFER = { BarrierSync::ALL, BarrierAccess::CONSTANT_BUFFER, TextureLayout::UNDEFINED };


	}
}