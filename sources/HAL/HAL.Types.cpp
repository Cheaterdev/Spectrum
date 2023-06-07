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
		layout |= state.layout;

		//assert(layout == state.layout);
		return *this;
	}

	
	ResourceState ResourceState::operator &(const ResourceState& state)const
	{
		ResourceState other = *this;
		other &= state;
		return other;

	}
	const ResourceState& ResourceState::operator&=(const ResourceState& state)
	{

		operation &= state.operation;
		access &= state.access;
		layout &= state.layout;

		//assert(layout == state.layout);
		return *this;
	}
	bool ResourceState::has_write_bits() const
	{

		static const BarrierAccess ACCESS_WRITE = BarrierAccess::DEPTH_STENCIL_WRITE | BarrierAccess::RENDER_TARGET | BarrierAccess::UNORDERED_ACCESS| BarrierAccess::RAYTRACING_ACCELERATION_STRUCTURE_WRITE;
		if(check(access&ACCESS_WRITE)) return true;

			static const TextureLayout LAYOUT_DIRECT = TextureLayout::UNORDERED_ACCESS |TextureLayout::DEPTH_STENCIL_WRITE |TextureLayout::RENDER_TARGET |TextureLayout::COPY_DEST  ;
	if(check(layout&LAYOUT_DIRECT)) 
		return true;

		return false;
	}

	
	CommandListType get_best_cmd_type(TextureLayout layout)
	{
	if(layout==TextureLayout::COPY_QUEUE) return CommandListType::COPY;


		static const TextureLayout LAYOUT_DIRECT = TextureLayout::DEPTH_STENCIL_WRITE | TextureLayout::RENDER_TARGET;
		if (check(layout & LAYOUT_DIRECT))
			return CommandListType::DIRECT;


		
		return CommandListType::COMPUTE;

	}
	CommandListType ResourceState::get_best_cmd_type() const
	{


		if (*this==ResourceStates::NO_ACCESS)
		return CommandListType::COPY;

		const   ResourceState COPY_SOURCE_2 = { BarrierSync::COPY, BarrierAccess::COPY_SOURCE, TextureLayout::COPY_QUEUE };
		const   ResourceState COPY_DEST_2 = { BarrierSync::COPY, BarrierAccess::COPY_DEST, TextureLayout::COPY_QUEUE };

		if (*this==COPY_SOURCE_2||*this==COPY_DEST_2)
			return CommandListType::COPY;


		static const BarrierAccess ACCESS_DIRECT = BarrierAccess::RENDER_TARGET | BarrierAccess::DEPTH_STENCIL_WRITE | BarrierAccess::RENDER_TARGET;//| BarrierAccess::UNORDERED_ACCESS| BarrierAccess::RAYTRACING_ACCELERATION_STRUCTURE_WRITE;
		if (check(access & ACCESS_DIRECT))
			return CommandListType::DIRECT;

		static const TextureLayout LAYOUT_DIRECT = TextureLayout::DEPTH_STENCIL_WRITE | TextureLayout::RENDER_TARGET;
		if (check(layout & LAYOUT_DIRECT))
			return CommandListType::DIRECT;

		static const BarrierSync SYNC_DIRECT = BarrierSync::INDEX_INPUT | BarrierSync::VERTEX_SHADING | BarrierSync::PIXEL_SHADING | BarrierSync::DEPTH_STENCIL | BarrierSync::RENDER_TARGET;
		if (check(operation & SYNC_DIRECT))
			return CommandListType::DIRECT;





		return CommandListType::COMPUTE;

	}


	bool ResourceState::is_no_access() const
	{
		if (check(operation & BarrierSync::NONE))
		{
			
		 return true;

		}
		


		if (check(access & BarrierAccess::NO_ACCESS))
		{

			 return true;

		}

		return false;
	}

	bool ResourceState::is_valid() const
	{
		if (check(operation & BarrierSync::COPY))
		{
			
			bool is_read = check(layout & TextureLayout::COPY_QUEUE)||(check(layout & TextureLayout::COPY_SOURCE) && check(access & BarrierAccess::COPY_SOURCE));
			bool is_write =  check(layout & TextureLayout::COPY_QUEUE)||(check(layout & TextureLayout::COPY_DEST) && check(access & BarrierAccess::COPY_DEST));
			if (!is_read && !is_write)	 return false;

		}
		


		if (check(access & BarrierAccess::SHADER_RESOURCE))
		{

			if (layout!=TextureLayout::NONE&&!check(layout & TextureLayout::SHADER_RESOURCE))	 return false;

		}
		if (check(access & BarrierAccess::UNORDERED_ACCESS))
		{

			if (layout!=TextureLayout::NONE&&!check(layout & TextureLayout::UNORDERED_ACCESS))	 return false;

		}

		if (check(access & BarrierAccess::SHADER_RESOURCE))
		{

			if (layout!=TextureLayout::NONE&&!check(layout & TextureLayout::SHADER_RESOURCE))	 return false;

		}


		return true;
	}


	ResourceState::ResourceState(BarrierSync s, BarrierAccess a, TextureLayout l) : operation(s), access(a), layout(l)
	{

	}
	namespace ResourceStates {

		const  ResourceState INDEX_BUFFER = { BarrierSync::INDEX_INPUT, BarrierAccess::INDEX_BUFFER, TextureLayout::UNDEFINED };
		const   ResourceState COPY_SOURCE = { BarrierSync::COPY, BarrierAccess::COPY_SOURCE, TextureLayout::COPY_SOURCE };
		const   ResourceState COPY_DEST = { BarrierSync::COPY, BarrierAccess::COPY_DEST, TextureLayout::COPY_DEST };

		const  ResourceState SHADER_RESOURCE = { BarrierSync::ALL_SHADING, BarrierAccess::SHADER_RESOURCE, TextureLayout::SHADER_RESOURCE };
//		const  ResourceState NON_PIXEL_SHADER_RESOURCE = { BarrierSync::NON_PIXEL_SHADING, BarrierAccess::SHADER_RESOURCE, TextureLayout::SHADER_RESOURCE };
		const  ResourceState UNORDERED_ACCESS = { BarrierSync::ALL_SHADING, BarrierAccess::UNORDERED_ACCESS, TextureLayout::UNORDERED_ACCESS };
		const  ResourceState RAYTRACING_STRUCTURE = { BarrierSync::RAYTRACING, BarrierAccess::RAYTRACING_ACCELERATION_STRUCTURE_READ, TextureLayout::UNDEFINED };

		const  ResourceState RAYTRACING_STRUCTURE_WRITE = { BarrierSync::RAYTRACING, BarrierAccess::RAYTRACING_ACCELERATION_STRUCTURE_READ | BarrierAccess::RAYTRACING_ACCELERATION_STRUCTURE_WRITE, TextureLayout::UNDEFINED };
		const  ResourceState INDIRECT_ARGUMENT = { BarrierSync::EXECUTE_INDIRECT, BarrierAccess::INDIRECT_ARGUMENT, TextureLayout::UNDEFINED };

		const  ResourceState RENDER_TARGET = { BarrierSync::RENDER_TARGET, BarrierAccess::RENDER_TARGET, TextureLayout::RENDER_TARGET };
		const  ResourceState DEPTH_STENCIL = { BarrierSync::DEPTH_STENCIL, BarrierAccess::DEPTH_STENCIL_WRITE /*| BarrierAccess::DEPTH_STENCIL_READ*/,TextureLayout::DEPTH_STENCIL_WRITE | TextureLayout::DEPTH_STENCIL_READ };

	//	const  ResourceState CONSTANT_BUFFER = { BarrierSync::ALL, BarrierAccess::CONSTANT_BUFFER, TextureLayout::UNDEFINED };
		const  ResourceState NO_ACCESS = { BarrierSync::NONE, BarrierAccess::NO_ACCESS, TextureLayout::UNDEFINED };

//		const  ResourceState WRITE_STATES = UNORDERED_ACCESS | RAYTRACING_STRUCTURE_WRITE | RENDER_TARGET | DEPTH_STENCIL ;
const  ResourceState UNKNOWN = { BarrierSync::NONE, BarrierAccess::NO_ACCESS, TextureLayout::UNDEFINED };

	}
}