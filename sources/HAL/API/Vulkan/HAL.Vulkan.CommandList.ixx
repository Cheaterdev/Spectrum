export module HAL:API.CommandList;
import Core;
import vulkan;
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

export namespace HAL
{
    namespace API
    {
        class CommandList
        {
            void* debug_ptr = nullptr;
            friend class HAL::Queue;

            VkCommandBuffer vk_cmd      = VK_NULL_HANDLE;
            // The pool that allocated vk_cmd.  Tracked so begin() can detect when
            // compile() rotates to a different CommandAllocator: in that case we must
            // allocate a fresh buffer from the new pool rather than calling
            // vkResetCommandBuffer on a buffer that belongs to a different pool —
            // doing so would access the old pool without holding its mutex, which
            // causes threading-validation errors when another thread uses that pool.
            VkCommandPool   vk_cmd_pool = VK_NULL_HANDLE;
            CommandListType type;
            Device*         m_device    = nullptr;

            // Pointer to the pool mutex; locked in begin(), unlocked in end().
            // Raw pointer (not unique_lock) so CommandList stays copy-constructible —
            // generic lambdas that take (auto list) copy the list by value, which would
            // fail if CommandList had a non-copyable unique_lock member.
            // The copy shares the pointer but never calls end(), so double-unlock cannot
            // occur.  Only the canonical begin()/end() pair manages the lock.
            std::mutex* _pool_mutex = nullptr;

            // Dynamic rendering state — populated by set_rtv(), consumed by draw calls.
            VkImageView current_color_view = VK_NULL_HANDLE;
            VkImageView current_depth_view = VK_NULL_HANDLE;
            VkExtent2D  current_extent     = {};
            bool        in_render_pass     = false;

            // Pipeline state
            VkPipelineLayout current_pipeline_layout = VK_NULL_HANDLE;
            bool             descriptor_sets_dirty   = false;
            // Last bound graphics pipeline — re-bound before each draw because the
            // recorder may split set_pipeline and the draw across command buffers.
            VkPipeline       current_graphics_pipeline = VK_NULL_HANDLE;
            // Last bound index buffer — re-bound before each indexed draw for the same reason.
            VkBuffer         current_index_buffer = VK_NULL_HANDLE;
            VkDeviceSize     current_index_offset = 0;
            VkIndexType      current_index_type   = VK_INDEX_TYPE_UINT16;
            // Last viewport(s) / scissor set by the engine.  Dynamic state does not
            // survive a command-buffer split, so these are re-applied before every draw;
            // without this the draw inherits an undefined/empty scissor and is fully
            // clipped → nothing renders (the classic Vulkan "black screen" here).
            std::vector<VkViewport> current_viewports;
            VkRect2D                current_scissor   = {};
            bool                    has_scissor       = false;

            // Bound descriptor heaps (set by set_descriptor_heaps())
            VkDescriptorSet  cbv_srv_uav_set = VK_NULL_HANDLE;
            VkDescriptorSet  sampler_set     = VK_NULL_HANDLE;

            // Push constant staging (for graphics_set_constant / compute_set_constant)
            std::array<uint32_t, 32> push_constants = {};

            // Start/end dynamic rendering.  begin_rendering uses LOAD_OP_LOAD so
            // that clear_rtv (which uses its own begin/end with CLEAR) is not
            // overwritten.  end_rendering_if_active() is called from end() and
            // set_rtv() to close any open render pass before a new one starts.
            void end_rendering_if_active();
            void begin_rendering(VkAttachmentLoadOp color_load, VkClearValue color_clear,
                                 VkAttachmentLoadOp depth_load, VkClearValue depth_clear);
            void ensure_rendering_active(); // lazily start render pass for draw calls
            void flush_descriptor_sets();  // bind pending descriptor sets
            void reapply_draw_state();     // re-bind pipeline + viewport/scissor before a draw

        public:
            VkCommandBuffer get_native() const { return vk_cmd; }

            void create(CommandListType type, Device& device);
            void begin(CommandAllocator& allocator);
            void end();

            operator bool() const { return vk_cmd != VK_NULL_HANDLE; }

            // --- D3D12-only stubs (no-op in Vulkan) ---
            void set_program(StateObject* id, ResourceAddress buffer, uint size, bool init) {}
            void dispatch_graph(ResourceAddress addr) {}

            // --- Common recording API ---
            void clear_uav(const UAVHandle& h, vec4 ClearColor);
            void clear_rtv(const RTVHandle& h, vec4 ClearColor);
            void clear_stencil(const DSVHandle& dsv, UINT8 stencil);
            void clear_depth(const DSVHandle& dsv, float depth);
            void clear_depth_stencil(const DSVHandle& dsv, bool depth, bool stencil, float fdepth, UINT8 fstencil);
            void set_topology(HAL::PrimitiveTopologyType topology,
                              HAL::PrimitiveTopologyFeed feedType = HAL::PrimitiveTopologyFeed::LIST,
                              bool adjusted = false, uint controlpoints = 0);
            void set_stencil_ref(UINT ref);

            // Raytracing — no-op stubs for Vulkan Phase 0
            void dispatch_rays(uint hit_size, uint miss_size, uint raygen_size,
                               ivec2 size, HAL::ResourceAddress hit_buffer, UINT hit_count,
                               HAL::ResourceAddress miss_buffer, UINT miss_count,
                               HAL::ResourceAddress raygen_buffer) {}

            void set_name(std::wstring_view name);
            void discard(const HAL::Resource* resource);
            void set_descriptor_heaps(DescriptorHeap* cbv, DescriptorHeap* sampler);
            void insert_time(const QueryHandle& handle, uint offset);
            void resolve_times(const QueryHeap* pQueryHeap, uint32_t NumQueries, ResourceAddress destination);
            void set_graphics_signature(const HAL::RootSignature::ptr& s);
            void set_compute_signature(const HAL::RootSignature::ptr& s);
            void draw(UINT vertex_count, UINT vertex_offset, UINT instance_count, UINT instance_offset);
            void draw_indexed(UINT index_count, UINT index_offset, UINT vertex_offset, UINT instance_count, UINT instance_offset);
            void set_index_buffer(HAL::Views::IndexBuffer index);
            void graphics_set_const_buffer(UINT i, const ResourceAddress& address);
            void compute_set_const_buffer(UINT i, const ResourceAddress& address);
            void graphics_set_constant(UINT i, UINT offset, UINT value);
            void compute_set_constant(UINT i, UINT offset, UINT value);
            void dispatch_mesh(ivec3 v);
            void dispatch(ivec3 v);
            void set_scissors(sizer_long rect);
            void set_viewports(std::vector<Viewport> viewports);
            void copy_resource(HAL::Resource* dest, HAL::Resource* source);
            void copy_buffer(HAL::Resource* dest, uint64 dest_offset,
                             HAL::Resource* source, uint64 source_offset, uint64 size);
            void set_pipeline(std::shared_ptr<TrackedPipeline> pipeline);
            void execute_indirect(const IndirectCommand& command_types, UINT max_commands,
                                  Resource* command_buffer, UINT64 command_offset,
                                  Resource* counter_buffer, UINT64 counter_offset);
            void set_rtv(int c, RTVHandle rt, DSVHandle h);
            void start_event(std::wstring_view str);
            void end_event();

            // Raytracing acceleration structure — no-op in Vulkan Phase 0
            void build_ras(const HAL::RaytracingBuildDescStructure& build_desc,
                           const HAL::RaytracingBuildDescBottomInputs& bottom) {}
            void build_ras(const HAL::RaytracingBuildDescStructure& build_desc,
                           const HAL::RaytracingBuildDescTopInputs& top) {}

            void copy_texture(const Resource::ptr& dest, int dest_subres,
                              const Resource::ptr& source, int source_subres);
            void copy_texture(const Resource::ptr& to, ivec3 to_pos,
                              const Resource::ptr& from, ivec3 from_pos, ivec3 size);
            void update_texture(HAL::Resource* resource, ivec3 offset, ivec3 box,
                                UINT sub_resource, ResourceAddress address, texture_layout layout);
            void read_texture(const HAL::Resource* resource, ivec3 offset, ivec3 box,
                              UINT sub_resource, ResourceAddress target, texture_layout layout);
            void transitions(const HAL::Barriers& barriers);
        };
    }
}
