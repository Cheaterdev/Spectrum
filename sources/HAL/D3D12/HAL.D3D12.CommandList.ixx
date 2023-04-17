export module HAL:API.CommandList;
import Core;
import :Types;

import :ResourceStates;
import :Resource;
import :DescriptorHeap;
import :Fence;
import :FrameManager;
import :PipelineState;
import :RootSignature;
import :API.IndirectCommand;
import :Debug;

export namespace HAL {

	namespace API
	{

		class CommandList
		{

			friend class HAL::Queue;
			D3D::CommandList m_commandList;
			D3D_PRIMITIVE_TOPOLOGY  native_topology = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
			CommandListType type;
		public:

			D3D::CommandList get_native() const
			{
				return m_commandList;
			}

			void create(CommandListType type);
			void begin(CommandAllocator& allocator);
			void end();

			operator bool()
			{
				return !!m_commandList;
			}

			void clear_uav(const UAVHandle& h, vec4 ClearColor);
			void clear_rtv(const RTVHandle& h, vec4 ClearColor);
     		void clear_stencil(const DSVHandle& dsv, UINT8 stencil);
			void clear_depth(const DSVHandle& dsv, float depth);
			void clear_depth_stencil(const DSVHandle& dsv, bool depth, bool stencil, float fdepth, UINT8 fstencil);
			void set_topology(HAL::PrimitiveTopologyType topology, HAL::PrimitiveTopologyFeed feedType = HAL::PrimitiveTopologyFeed::LIST, bool adjusted = false, uint controlpoints = 0);
			void set_stencil_ref(UINT ref);

			void dispatch_rays(uint hit_size, uint miss_size, uint raygen_sige, ivec2 size, HAL::ResourceAddress hit_buffer, UINT hit_count, HAL::ResourceAddress miss_buffer, UINT miss_count, HAL::ResourceAddress raygen_buffer);
			void set_name(std::wstring_view name);

			void set_descriptor_heaps(DescriptorHeap* cbv, DescriptorHeap* sampler);
			void insert_time(const QueryHandle& handle, uint offset);
			void resolve_times(const QueryHeap* pQueryHeap, uint32_t NumQueries, ResourceAddress destination);
			void set_graphics_signature(const HAL::RootSignature::ptr& s);
			void set_compute_signature(const HAL::RootSignature::ptr& s);
			void draw(UINT vertex_count, UINT vertex_offset, UINT instance_count, UINT instance_offset);
			void draw_indexed(UINT index_count, UINT index_offset, UINT vertex_offset, UINT instance_count, UINT instance_offset);
			void set_index_buffer(HAL::Views::IndexBuffer index);
			void graphics_set_const_buffer(UINT i, const ResourceAddress& adress);
			void compute_set_const_buffer(UINT i, const ResourceAddress& adress);
			void graphics_set_constant(UINT i, UINT offset, UINT value);
			void compute_set_constant(UINT i, UINT offset, UINT value);
			void dispatch_mesh(ivec3 v);
			void dispatch(ivec3 v);
			void set_scissors(sizer_long rect);
			void set_viewports(std::vector<Viewport> viewports);
			void copy_resource(HAL::Resource* dest, HAL::Resource* source);
			void copy_buffer(HAL::Resource* dest, uint64 dest_offset, HAL::Resource* source, uint64 source_offset, uint64 size);
			void set_pipeline(PipelineStateBase* pipeline);
			void execute_indirect(const IndirectCommand& command_types, UINT max_commands, Resource* command_buffer, UINT64 command_offset, Resource* counter_buffer, UINT64 counter_offset);
			void set_rtv(int c, RTVHandle rt, DSVHandle h);
			void start_event(std::wstring_view str);
			void end_event();
			void build_ras(const HAL::RaytracingBuildDescStructure& build_desc, const HAL::RaytracingBuildDescBottomInputs& bottom);
			void build_ras(const HAL::RaytracingBuildDescStructure& build_desc, const HAL::RaytracingBuildDescTopInputs& top);
			void copy_texture(const Resource::ptr& dest, int dest_subres, const Resource::ptr& source, int source_subres);
			void copy_texture(const Resource::ptr& to, ivec3 to_pos, const Resource::ptr& from, ivec3 from_pos, ivec3 size);
			void update_texture( HAL::Resource* resource, ivec3 offset, ivec3 box, UINT sub_resource, ResourceAddress address, texture_layout layout);
			void read_texture(const  HAL::Resource* resource, ivec3 offset, ivec3 box, UINT sub_resource, ResourceAddress target, texture_layout layout);
			void transitions(const HAL::Barriers& _barriers);
		};
	}
}