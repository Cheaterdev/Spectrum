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
            // Last bound graphics pipeline — re-bound before each draw because the
            // recorder may split set_pipeline and the draw across command buffers.
            VkPipeline       current_graphics_pipeline = VK_NULL_HANDLE;
            // Mesh pipelines have no primitive-topology dynamic state, so the
            // per-draw topology re-apply must be skipped for them.
            bool             current_is_mesh = false;
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
            VkPrimitiveTopology     current_topology  = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

            // Bound descriptor heaps (set by set_descriptor_heaps()).  Under
            // VK_EXT_descriptor_heap a heap is a device-address range bound via
            // vkCmdBindResourceHeapEXT / vkCmdBindSamplerHeapEXT.  Like D3D12's
            // SetDescriptorHeaps, the binding persists — flushed once per command
            // buffer (and re-flushed after a recorder-induced CB split).
            VkDeviceAddress cbv_srv_uav_addr           = 0;
            VkDeviceSize    cbv_srv_uav_size           = 0;
            VkDeviceSize    cbv_srv_uav_reserved_off   = 0;
            VkDeviceSize    cbv_srv_uav_reserved_size  = 0;
            VkDeviceAddress sampler_addr               = 0;
            VkDeviceSize    sampler_size               = 0;
            VkDeviceSize    sampler_reserved_off       = 0;
            VkDeviceSize    sampler_reserved_size      = 0;
            bool            heaps_dirty                = false;

            // Push constant staging (for graphics_set_constant / compute_set_constant),
            // re-pushed via vkCmdPushDataEXT after a command-buffer split.
            std::array<uint32_t, 32> push_constants = {};

            // Deferred PRESENT_SRC_KHR transitions.
            //
            // The FrameGraph's non_tracked_resources loop places the swapchain's
            // RENDER_TARGET→PRESENT barrier immediately after set_rtv() (at P_post,
            // the next usage point), which fires BEFORE the draw calls.  On D3D12
            // this is harmless because PRESENT==COMMON and COMMON is implicitly
            // promoted back to RENDER_TARGET on use.  Vulkan has no such promotion:
            // draws would see the image in PRESENT_SRC_KHR rather than
            // COLOR_ATTACHMENT_OPTIMAL → black output.
            //
            // Fix: any barrier whose newLayout is PRESENT_SRC_KHR is held here and
            // flushed at end(), after ALL draws, so the image stays in
            // COLOR_ATTACHMENT_OPTIMAL throughout the render pass.
            std::vector<VkImageMemoryBarrier2> deferred_present_barriers;

            // Start/end dynamic rendering.  begin_rendering uses LOAD_OP_LOAD so
            // that clear_rtv (which uses its own begin/end with CLEAR) is not
            // overwritten.  end_rendering_if_active() is called from end() and
            // set_rtv() to close any open render pass before a new one starts.
            void end_rendering_if_active();
            void begin_rendering(VkAttachmentLoadOp color_load, VkClearValue color_clear,
                                 VkAttachmentLoadOp depth_load, VkClearValue depth_clear);
            void ensure_rendering_active(); // lazily start render pass for draw calls
            void flush_heaps();            // bind pending descriptor heaps (resource + sampler)
            void push_full_constants();    // push the whole staged root-constant block
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
            void clear_uav(const Handles::UAV& h, vec4 ClearColor);
            void clear_rtv(const Handles::RTV& h, vec4 ClearColor);
            void clear_stencil(const Handles::DSV& dsv, UINT8 stencil);
            void clear_depth(const Handles::DSV& dsv, float depth);
            void clear_depth_stencil(const Handles::DSV& dsv, bool depth, bool stencil, float fdepth, UINT8 fstencil);
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
            void set_rtv(int c, Handles::RTV rt, Handles::DSV h);
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
