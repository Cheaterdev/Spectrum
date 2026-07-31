export module HAL:CommandListRecorder;

import Core;
import :Types;
import :ResourceStates;
import :Resource;
import :DescriptorHeap;
import :Fence;
import :FrameManager;
import :PipelineState;
import :API.IndirectCommand;
import :API.CommandList;
import :QueryHeap;
import :ResourceViews;
import :PSO;
import :CommandAllocator;

export namespace HAL
{
	enum class CommandType : uint8_t
	{
		Transition,
		Draw, DrawIndexed, DispatchMesh,
		Dispatch, DispatchGraph, DispatchRays,
		CopyResource, CopyBuffer, CopyTexture, UpdateTexture, ReadTexture,
		BuildRAS,
		SetPipeline, SetRTV, SetTopology, SetIndexBuffer,
		SetScissor, SetViewport,
		ClearRTV, ClearUAV, ClearDepth, ClearStencil, ClearDepthStencil,
		SetGraphicsSignature, SetComputeSignature,
		GraphicsSetConstBuffer, ComputeSetConstBuffer,
		GraphicsSetConstant, ComputeSetConstant,
		ExecuteIndirect, SetProgram,
		InsertTime, ResolveTime,
		StartEvent, EndEvent,
		SetDescriptorHeaps, SetStencilRef, Discard,
		Func,
	};

	struct CommandRecord
	{
		// Per-resource detail stored for Transition records after snapshot.
		struct BarrierDetail
		{
			std::string        resource_name;
			HAL::ResourceState before;
			HAL::ResourceState after;
			uint               subres = 0;
			HAL::BarrierFlags  flags  = HAL::BarrierFlags::NONE;
			// Opaque per-instance id (the source Resource address). A recreate()
			// makes a NEW Resource object with the SAME name, so the debugger uses
			// this to tell old/new instances apart and separate them. Never
			// dereferenced — identity comparison only.
			uint64             resource_id = 0;
		};

		CommandType  type          = CommandType::Func;
		std::string  description;
		UsagePoint*  barrier_point = nullptr; // non-null only until snapshot
		std::vector<BarrierDetail> barrier_details; // non-empty only for Transition records
	};

	class DelayedCommandList
	{
		// Dev builds: full recording vector (24 bytes).
		// Non-Dev builds: zero-size no-op sink — [[no_unique_address]] guarantees no storage cost.
		struct DevRecorder
		{
			std::vector<CommandRecord> records;
			void push_back(CommandRecord r)              { records.push_back(std::move(r)); }
			void clear()                                 { records.clear(); }
			const std::vector<CommandRecord>& get() const { return records; }
		};
		struct NullRecorder
		{
			void push_back(CommandRecord) noexcept {}
			void clear()                 noexcept {}
			const std::vector<CommandRecord>& get() const
			{
				static const std::vector<CommandRecord> s_empty;
				return s_empty;
			}
		};

		// Typed command — trivial data stored inline (no heap, no type-erasure).
		// Commands with shared_ptr captures (handles, signatures, …) use fn_pool.
		struct Cmd
		{
			CommandType type;
			uint32_t    fn_idx = 0; // index into fn_pool for fallback commands

			union
			{
				UsagePoint*   barrier;

				struct { UINT vc, vo, ic, io; }                               draw;
				struct { UINT ic, ioff, vo, inst, io; }                       draw_indexed;
				ivec3                                                          dispatch_args;   // Dispatch / DispatchMesh
				ResourceAddress                                                dispatch_graph;  // DispatchGraph
				struct { DescriptorHeap* cbv; DescriptorHeap* sampler; }      desc_heaps;
				struct { UINT i, offset, value; }                              set_constant;    // Gfx/Cmp SetConstant
				struct { UINT i; ResourceAddress addr; }                       set_cb;          // Gfx/Cmp SetConstBuffer
				struct { Resource* dst; Resource* src; }                       copy_res;
				const Resource*                                                discard_res;
				UINT                                                           stencil_ref;
				struct {
					HAL::PrimitiveTopologyType topo;
					HAL::PrimitiveTopologyFeed feed;
					bool adjusted; uint cpoints;
				}                                                              set_topology;
				std::wstring_view                                              event_str;       // StartEvent
				struct { const QueryHeap* heap; uint32_t count; ResourceAddress dest; } resolve;
				struct { StateObject* obj; ResourceAddress buf; uint size; bool init; } set_program;

				uint8_t _ensure_size[32]; // keeps union at least 32 bytes
			};

			Cmd() noexcept : type(CommandType::Func), fn_idx(0) {}
		};
		static_assert(sizeof(Cmd) <= 48);

		template<class F>
		void push_fn(CommandType type, F&& f)
		{
			Cmd cmd{};
			cmd.type   = type;
			cmd.fn_idx = static_cast<uint32_t>(fn_pool.size());
			fn_pool.emplace_back(std::forward<F>(f));
			tasks.push_back(cmd);
		}

		bool compiled = false;
		API::CommandList list;
		std::wstring name;
		std::vector<Cmd> tasks;
		std::vector<std::function<void(API::CommandList&)>> fn_pool; // shared_ptr captures
		[[no_unique_address]]
		std::conditional_t<BuildOptions::Dev, DevRecorder, NullRecorder> debug_recorder;
	public:
		const API::CommandList& get_list() const;
		void create(CommandListType type, Device& device);
		void reset();

		bool is_compiled() const;

		void compile(CommandAllocator& allocator);

		void func(std::function<void(API::CommandList&)> f);

		void set_program(StateObject*, ResourceAddress buffer, uint size, bool init);
		void dispatch_graph(ResourceAddress addr);
		void clear_uav(const Handles::UAV& h, vec4 ClearColor = vec4(0, 0, 0, 0));
		void clear_rtv(const Handles::RTV& h, vec4 ClearColor);
		void clear_stencil(const Handles::DSV& dsv, UINT8 stencil);
		void clear_depth(const Handles::DSV& dsv, float depth);
		void clear_depth_stencil(const Handles::DSV& dsv, bool depth, bool stencil, float fdepth, UINT8 fstencil);

		void set_topology(HAL::PrimitiveTopologyType topology, HAL::PrimitiveTopologyFeed feedType = HAL::PrimitiveTopologyFeed::LIST, bool adjusted = false, uint controlpoints = 0);
		void set_stencil_ref(UINT ref);
		void discard(const  HAL::Resource* resource);

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
		void set_scissors(std::vector<sizer_long> rects);
		void set_viewports(std::vector<Viewport> viewports);
		void copy_resource(HAL::Resource* dest, HAL::Resource* source);
		void copy_buffer(HAL::Resource* dest, uint64 dest_offset, HAL::Resource* source, uint64 source_offset, uint64 size);
		void set_pipeline(PipelineStateBase* pipeline);
		void execute_indirect(IndirectCommand& command_types, UINT max_commands, Resource* command_buffer, UINT64 command_offset, Resource* counter_buffer, UINT64 counter_offset);
		void set_rtv(int c, Handles::RTV rt, Handles::DSV h);
		void start_event(std::wstring_view str);
		void end_event();
		void build_ras(const HAL::RaytracingBuildDescStructure& build_desc, const HAL::RaytracingBuildDescBottomInputs& bottom);
		void build_ras(const HAL::RaytracingBuildDescStructure& build_desc, const HAL::RaytracingBuildDescTopInputs& top);
		void copy_texture(const Resource::ptr& dest, int dest_subres, const Resource::ptr& source, int source_subres);
		void copy_texture(const Resource::ptr& to, ivec3 to_pos, const Resource::ptr& from, ivec3 from_pos, ivec3 size);
		void update_texture(HAL::Resource* resource, ivec3 offset, ivec3 box, UINT sub_resource, ResourceAddress address, texture_layout layout);
		void read_texture(const HAL::Resource* resource, ivec3 offset, ivec3 box, UINT sub_resource, ResourceAddress target, texture_layout layout);

		void func_barrier(UsagePoint* point);
		const std::vector<CommandRecord>& get_debug_records() const;

		template<class Hit, class Miss, class Raygen>
		void dispatch_rays(ivec2 size, HAL::ResourceAddress hit_buffer, UINT hit_count, HAL::ResourceAddress miss_buffer, UINT miss_count, HAL::ResourceAddress raygen_buffer) {
			if constexpr (BuildOptions::Dev)
				debug_recorder.push_back({CommandType::DispatchRays,
					"DispatchRays " + std::to_string(size.x) + "x" + std::to_string(size.y)});
			Cmd cmd; cmd.type = CommandType::DispatchRays;
			cmd.fn_idx = static_cast<uint32_t>(fn_pool.size());
			fn_pool.emplace_back([=](API::CommandList& list) {
				list.dispatch_rays(sizeof(Hit), sizeof(Miss), sizeof(Raygen), size, hit_buffer, hit_count, miss_buffer, miss_count, raygen_buffer);
			});
			tasks.push_back(cmd);
		}
	};
}
