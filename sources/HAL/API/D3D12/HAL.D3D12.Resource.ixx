export module HAL:API.Resource;

import :Types;
import :Sampler;
import :Utils;
import :API.Device;
import :Heap;
import d3d12;
import Core;

import :Format;

export namespace HAL
{
    struct PlacementAddress
    {
        Heap* heap;
        size_t offset;
    };

    namespace API {

        // NativeImportHandle: wraps an externally-owned D3D12 resource
        // (e.g. a swapchain back-buffer).  The Vulkan backend provides its own
        // version of this struct in HAL.Vulkan.Resource.ixx.
        struct NativeImportHandle
        {
            D3D::Resource resource;
        };

        class Resource
        {
            GPUAddressPtr address;
        public:
            using ptr = std::shared_ptr<Resource>;
            // clear_value: optimized clear value baked into a RenderTarget
            // resource. Must match the colour passed to ClearRenderTargetView or
            // D3D12 warns (#820) and takes a slower clear path.
            void init(Device& device, const ResourceDesc& desc, const PlacementAddress& address, TextureLayout initialLayout = TextureLayout::UNDEFINED, vec4 clear_value = vec4(0, 0, 0, 0));

            // Replaces the old D3D::Resource constructor; backend-neutral.
            void init(const NativeImportHandle& handle, TextureLayout layout, Device& device);

            GPUAddressPtr get_address();

            // CPU mapping for UPLOAD / READBACK heaps.
            void* get_cpu_mapping();

            D3D::Resource native_resource;

            auto get_dx() const
            {
                return native_resource.Get();
            }

            // Backend-uniform native handle (see HAL.Vulkan.Resource.ixx).
            void* get_native() const
            {
                return native_resource.Get();
            }
        };
    }
}

export
{

    namespace cereal
    {
        template<class Archive>
        void serialize(Archive& ar, HAL::API::Resource*& g)
        {
        /*    if (g)
            {
                auto desc = g->get_desc();
                auto native_desc = g->native_resource->GetDesc();

                ar& NVP(desc);
                ar& NVP(native_desc);
            }*/
        }
    }
}