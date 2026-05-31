module HAL:API.CommandList;
import stl.core;
import vulkan;
import Core;

// Phase 3: implement full VkCommandBuffer recording.
// Phase 0: stub implementations — all no-ops.

namespace HAL::API
{
    void CommandList::create(CommandListType t, Device& dev)
    {
        type     = t;
        m_device = &dev;
    }

    void CommandList::begin(CommandAllocator& /*allocator*/)
    {
        if (vk_cmd == VK_NULL_HANDLE) return;
        VkCommandBufferBeginInfo info{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
        info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(vk_cmd, &info);
    }

    void CommandList::end()
    {
        if (vk_cmd != VK_NULL_HANDLE)
            vkEndCommandBuffer(vk_cmd);
    }

    void CommandList::set_name(std::wstring_view) {}
    void CommandList::discard(const HAL::Resource*) {}
    void CommandList::set_descriptor_heaps(DescriptorHeap*, DescriptorHeap*) {}
    void CommandList::insert_time(const QueryHandle&, uint) {}
    void CommandList::resolve_times(const QueryHeap*, uint32_t, ResourceAddress) {}
    void CommandList::set_graphics_signature(const HAL::RootSignature::ptr&) {}
    void CommandList::set_compute_signature(const HAL::RootSignature::ptr&) {}
    void CommandList::clear_uav(const UAVHandle&, vec4) {}
    void CommandList::clear_rtv(const RTVHandle&, vec4) {}
    void CommandList::clear_stencil(const DSVHandle&, UINT8) {}
    void CommandList::clear_depth(const DSVHandle&, float) {}
    void CommandList::clear_depth_stencil(const DSVHandle&, bool, bool, float, UINT8) {}
    void CommandList::set_topology(HAL::PrimitiveTopologyType, HAL::PrimitiveTopologyFeed, bool, uint) {}
    void CommandList::set_stencil_ref(UINT) {}
    void CommandList::draw(UINT, UINT, UINT, UINT) {}
    void CommandList::draw_indexed(UINT, UINT, UINT, UINT, UINT) {}
    void CommandList::set_index_buffer(HAL::Views::IndexBuffer) {}
    void CommandList::graphics_set_const_buffer(UINT, const ResourceAddress&) {}
    void CommandList::compute_set_const_buffer(UINT, const ResourceAddress&) {}
    void CommandList::graphics_set_constant(UINT, UINT, UINT) {}
    void CommandList::compute_set_constant(UINT, UINT, UINT) {}
    void CommandList::dispatch_mesh(ivec3) {}
    void CommandList::dispatch(ivec3) {}
    void CommandList::set_scissors(sizer_long) {}
    void CommandList::set_viewports(std::vector<Viewport>) {}
    void CommandList::copy_resource(HAL::Resource*, HAL::Resource*) {}
    void CommandList::copy_buffer(HAL::Resource*, uint64, HAL::Resource*, uint64, uint64) {}
    void CommandList::set_pipeline(std::shared_ptr<TrackedPipeline>) {}
    void CommandList::execute_indirect(const IndirectCommand&, UINT, Resource*, UINT64, Resource*, UINT64) {}
    void CommandList::set_rtv(int, RTVHandle, DSVHandle) {}
    void CommandList::start_event(std::wstring_view) {}
    void CommandList::end_event() {}
    void CommandList::copy_texture(const Resource::ptr&, int, const Resource::ptr&, int) {}
    void CommandList::copy_texture(const Resource::ptr&, ivec3, const Resource::ptr&, ivec3, ivec3) {}
    void CommandList::update_texture(HAL::Resource*, ivec3, ivec3, UINT, ResourceAddress, texture_layout) {}
    void CommandList::read_texture(const HAL::Resource*, ivec3, ivec3, UINT, ResourceAddress, texture_layout) {}
    void CommandList::transitions(const HAL::Barriers&) {}
}
