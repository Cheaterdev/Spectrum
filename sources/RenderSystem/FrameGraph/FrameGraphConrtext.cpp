module FrameGraph;
import HAL;
import Core;
import <HAL.h>;

using namespace HAL;



namespace FrameGraph
{

		CopyContext::CopyContext(Graph *graph, HAL::CommandList::ptr list)
		{
		//this->graph = graph;
		this->context = &list->get_copy();
		}
	
		GraphicsContext::GraphicsContext(Graph *graph, HAL::CommandList::ptr list)
		{
		this->graph = graph;
		if(list->get_type()==HAL::CommandListType::DIRECT)
		{
			this->context = &list->get_graphics();
		this->sig_setter = this->context;
		}
	
		}
		ComputeContext::ComputeContext(Graph *graph, HAL::CommandList::ptr list)
		{
		this->graph = graph;
		if(list->get_type()==HAL::CommandListType::COMPUTE||list->get_type()==HAL::CommandListType::DIRECT)
		{this->context = &list->get_compute();
		this->sig_setter = this->context;
		}
		}
			void FrameSetter::set_signature(const RootSignature::ptr& signature)
			{
			sig_setter->set_signature(signature);
			}
	
		void FrameSetter::set_slot(SlotID id)
	{
			graph->set_slot(id,*this);
		}

					void GraphicsContext::set_viewport(Viewport viewport)
					{
					context->set_viewport(viewport);
	}

			void GraphicsContext::set_viewport(vec4 viewport)	{
					context->set_viewport(viewport);
	}


	void GraphicsContext::set_topology(HAL::PrimitiveTopologyType topology, HAL::PrimitiveTopologyFeed feedType, bool adjusted, uint controlpoints)
	{
		context->set_topology(topology, feedType, adjusted, controlpoints);
	}
	void GraphicsContext::set_stencil_ref(UINT ref) {
		context->set_stencil_ref(ref);
	}
	void GraphicsContext::draw(UINT vertex_count, UINT vertex_offset, UINT instance_count, UINT instance_offset) {
		context->draw(vertex_count, vertex_offset, instance_count, instance_offset);
	}
	void GraphicsContext::draw_indexed(UINT index_count, UINT index_offset, UINT vertex_offset, UINT instance_count, UINT instance_offset) {
		context->draw_indexed(index_count, index_offset, vertex_offset, instance_count, instance_offset);
	}
	void GraphicsContext::set_index_buffer(HAL::Views::IndexBuffer index) {
		context->set_index_buffer(index);
	}

	void GraphicsContext::dispatch_mesh(ivec3 v) {
		context->dispatch_mesh(v);
	}


			void ComputeContext::dispatch(int x,int y, int z)
			{
				context->dispach({x,y,z});
			}
			void ComputeContext::dispatch(ivec2 a , ivec2 b)
			{
				context->dispach(a,b);
			}
			void ComputeContext::dispatch(ivec3 a , ivec3 b)
			{
				context->dispach(a,b);
			}


	void GraphicsContext::set_scissors(sizer_long rect) {
		context->set_scissors(rect);
	}
	void GraphicsContext::set_viewports(std::vector<Viewport> viewports) {
		context->set_viewports(viewports);
	}

	void CopyContext::copy_resource(HAL::Resource::ptr dest, HAL::Resource::ptr source) {
		context->copy_resource(dest, source);
	}
	void CopyContext::copy_buffer(HAL::Resource::ptr dest, uint64 dest_offset, HAL::Resource::ptr source, uint64 source_offset, uint64 size) {
		context->copy_buffer(dest.get(), dest_offset, source.get(), source_offset, size);
	}

	void GraphicsContext::execute_indirect(IndirectCommand& command_types, UINT max_commands, Resource* command_buffer, UINT64 command_offset, Resource* counter_buffer, UINT64 counter_offset) {
		context->execute_indirect(command_types, max_commands, command_buffer, command_offset, counter_buffer, counter_offset);
	}

void ComputeContext::execute_indirect(IndirectCommand& command_types, UINT max_commands, Resource* command_buffer, UINT64 command_offset, Resource* counter_buffer, UINT64 counter_offset) {
		context->execute_indirect(command_types, max_commands, command_buffer, command_offset, counter_buffer, counter_offset);
	}

	void ComputeContext::build_ras(const HAL::RaytracingBuildDescStructure& build_desc, const HAL::RaytracingBuildDescBottomInputs& bottom) {
		context->build_ras(build_desc, bottom);
	}
	void ComputeContext::build_ras(const HAL::RaytracingBuildDescStructure& build_desc, const HAL::RaytracingBuildDescTopInputs& top) {
		context->build_ras(build_desc, top);
	}
		void FrameContext::clear_uav(const UAVHandle& h, vec4 ClearColor)
		{
		get_list()->clear_uav(h, ClearColor);
		}
		
	void CopyContext::copy_texture(const Resource::ptr& dest, int dest_subres, const Resource::ptr& source, int source_subres) {
		context->copy_texture(dest, dest_subres, source, source_subres);
	}
	void CopyContext::copy_texture(const Resource::ptr& to, ivec3 to_pos, const Resource::ptr& from, ivec3 from_pos, ivec3 size) {
		context->copy_texture(to, to_pos, from, from_pos, size);
	}

	//void FrameContext::update_texture(HAL::Resource* resource, ivec3 offset, ivec3 box, UINT sub_resource, ResourceAddress address, texture_layout layout) {
	//	list->get_copy().update_texture(resource, offset, box, sub_resource, address, layout);
	//}
	//void FrameContext::read_texture(const HAL::Resource* resource, ivec3 offset, ivec3 box, UINT sub_resource, ResourceAddress target, texture_layout layout) {
	//	list->get_copy().read_texture(resource, offset, box, sub_resource, target, layout);
	//}

}