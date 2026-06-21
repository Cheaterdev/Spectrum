module;

module HAL:Device;
import :Debug;
import :Utils;
import :HeapAllocators;

import d3d12;
import Core;

#undef THIS
namespace HAL
{
    texture_layout Device::get_texture_layout(const ResourceDesc& rdesc, UINT sub_resource)
    {
        auto& desc = rdesc.as_texture();

        UINT64 RequiredSize = 0;
        D3D12_PLACED_SUBRESOURCE_FOOTPRINT Layouts;
        UINT NumRows;
        UINT64 RowSizesInBytes;
        D3D::ResourceDesc Desc = ::to_native(rdesc);
        get_native_device()->GetCopyableFootprints1(&Desc, sub_resource, 1, 0, &Layouts, &NumRows,
                                                    &RowSizesInBytes, &RequiredSize);

        return {
            RequiredSize, NumRows, Layouts.Footprint.RowPitch, static_cast<uint>(NumRows * Layouts.Footprint.RowPitch),
            D3D12::TEXTURE_DATA_PLACEMENT_ALIGNMENT, from_native(Layouts.Footprint.Format)
        };
    }

    texture_layout Device::get_texture_layout(const ResourceDesc& rdesc, UINT sub_resource, ivec3 box)
    {
        auto& desc = rdesc.as_texture();

        UINT rows_count = box.y;

        if (desc.Format == Format::BC7_UNORM_SRGB || desc.Format == Format::BC7_UNORM)
            rows_count /= 4;
        UINT64 RequiredSize = 0;
        D3D12_PLACED_SUBRESOURCE_FOOTPRINT Layouts;
        UINT NumRows;
        UINT64 RowSizesInBytes;
        D3D::ResourceDesc Desc = ::to_native(rdesc);
        get_native_device()->GetCopyableFootprints1(&Desc, sub_resource, 1, 0, &Layouts, &NumRows,
                                                    &RowSizesInBytes, &RequiredSize);
        UINT64 res_stride = Math::AlignUp(RowSizesInBytes, D3D12::TEXTURE_DATA_PITCH_ALIGNMENT);
        UINT64 size = res_stride * rows_count * box.z;

        return {
            size, rows_count, static_cast<uint>(res_stride), static_cast<uint>(res_stride * rows_count),
            D3D12::TEXTURE_DATA_PLACEMENT_ALIGNMENT, from_native(Layouts.Footprint.Format)
        };
    }

    std::vector<std::byte> Device::compress(std::span<std::byte> source)
    {
        std::vector<std::byte> dest;
        dest.assign(source.data(), source.data() + source.size());

        size_t maxSize = g_bufferCompression->CompressBufferBound(static_cast<uint32_t>(source.size()));

        dest.resize(maxSize);

        size_t actualCompressedSize = 0;

        HRESULT compressionResult = g_bufferCompression->CompressBuffer(
            reinterpret_cast<const void*>(source.data()),
            static_cast<uint32_t>(source.size()),
            DSTORAGE_COMPRESSION_BEST_RATIO,
            reinterpret_cast<void*>(dest.data()),
            static_cast<uint32_t>(dest.size()),
            &actualCompressedSize);

        TEST(*this, compressionResult);
        dest.resize(actualCompressedSize);
        return dest;
    }

    namespace API
    {
        uint Device::get_descriptor_size(DescriptorHeapType type) const
        {
            return descriptor_sizes[type];
        }

        HRESULT Device::get_device_removed_reason() const
        {
            return native_device->GetDeviceRemovedReason();
        }

        uint Device::Subresources(const ResourceDesc& desc) const
        {
            if (desc.is_buffer())
                return 1;

            auto texture_desc = desc.as_texture();
            uint count = D3D12GetFormatPlaneCount(native_device.Get(), ::to_native(texture_desc.Format));
            return texture_desc.MipLevels * texture_desc.ArraySize * count;
        }

        void Device::dump_dred()
        {
            ComPtr<ID3D12DeviceRemovedExtendedData> pDred;
            TEST(*this, native_device->QueryInterface(IID_PPV_ARGS(&pDred)));

            D3D12_DRED_AUTO_BREADCRUMBS_OUTPUT DredAutoBreadcrumbsOutput = {};
            D3D12_DRED_PAGE_FAULT_OUTPUT DredPageFaultOutput = {};
            TEST(*this, pDred->GetAutoBreadcrumbsOutput(&DredAutoBreadcrumbsOutput));
            TEST(*this, pDred->GetPageFaultAllocationOutput(&DredPageFaultOutput));

            auto parse_node = [](const D3D12_AUTO_BREADCRUMB_NODE& node)
            {
                Log::get() << node << Log::endl;
            };

            auto node = DredAutoBreadcrumbsOutput.pHeadAutoBreadcrumbNode;

            while (node)
            {
                parse_node(*node);
                node = node->pNext;
            }
        }

        void Device::process_result(HRESULT hr, std::string_view line) const
        {
            if (FAILED(hr))
            {
                std::string message = std::system_category().message(hr);
                Log::get().crash_error(hr, line);
                hr = get_device_removed_reason();
                __debugbreak();
                ASSERT(false);
            }
        }

        RaytracingPrebuildInfo Device::calculateBuffers(const RaytracingBuildDescBottomInputs& desc)
        {
            auto inputs = to_native(desc);

            inputs.NumDescs = static_cast<UINT>(inputs.descs.size());
            inputs.pGeometryDescs = inputs.descs.data();

            D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info;
            native_device->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &info);
            return { info.ResultDataMaxSizeInBytes, info.ScratchDataSizeInBytes, info.UpdateScratchDataSizeInBytes };
        }

        RaytracingPrebuildInfo Device::calculateBuffers(const RaytracingBuildDescTopInputs& desc)
        {
            auto inputs = to_native(desc);
            D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info;
            native_device->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &info);
            return { info.ResultDataMaxSizeInBytes, info.ScratchDataSizeInBytes, info.UpdateScratchDataSizeInBytes };
        }

        Device::~Device()
        {
            g_bufferCompression.Reset();

            native_device.Reset();

            IDXGIDebug* debugDev = nullptr;
            HRESULT hr = DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debugDev));
            if (SUCCEEDED(hr) && debugDev)
                debugDev->ReportLiveObjects(DXGI::DEBUG_ALL, DXGI::DEBUG_RLO_ALL);
        }

    } // namespace API

    HAL::DeviceProperties Device::probe(HAL::Adapter::ptr adapter)
    {
        HAL::DeviceProperties props;

        ComPtr<ID3D12Device> tempDevice;
        D3D12CreateDevice(adapter->native_adapter.Get(), D3D_FEATURE_LEVEL_12_2, IID_PPV_ARGS(&tempDevice));
        if (!tempDevice)
            return props;

        props.name = convert(std::wstring_view(adapter->get_desc().Description));

        D3D12_FEATURE_DATA_D3D12_OPTIONS5 options5 = {};
        D3D12_FEATURE_DATA_D3D12_OPTIONS7 options7 = {};
        D3D12_FEATURE_DATA_D3D12_OPTIONS16 options16 = {};
        D3D12_FEATURE_DATA_D3D12_OPTIONS21 options21 = {};
        D3D12_FEATURE_DATA_SHADER_MODEL supportedShaderModel = { D3D_SHADER_MODEL_6_8 };

        tempDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &options5, sizeof(options5));
        tempDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS7, &options7, sizeof(options7));
        tempDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS16, &options16, sizeof(options16));
        tempDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS21, &options21, sizeof(options21));
        tempDevice->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &supportedShaderModel, sizeof(supportedShaderModel));

        props.rtx                    = options5.RaytracingTier != D3D12_RAYTRACING_TIER_NOT_SUPPORTED;
        props.mesh_shader            = options7.MeshShaderTier >= D3D12_MESH_SHADER_TIER_1;
        props.full_bindless          = supportedShaderModel.HighestShaderModel >= D3D_SHADER_MODEL_6_6;
        props.direct_gpu_upload_heap = options16.GPUUploadHeapSupported;
        props.work_graph             = false;//options21.WorkGraphsTier != D3D12_WORK_GRAPHS_TIER_NOT_SUPPORTED;

        return props;
    }

    namespace API
    {

        size_t Device::get_vram()
        {
            auto THIS = static_cast<HAL::Device*>(this);

            DXGI_QUERY_VIDEO_MEMORY_INFO videoMemoryInfo;
            THIS->adapter->native_adapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &videoMemoryInfo);
            size_t usedVRAM = videoMemoryInfo.CurrentUsage / 1024 / 1024;
            return usedVRAM;
        }

        size_t Device::get_upload_heap()
        {
            auto THIS = static_cast<HAL::Device*>(this);
            return THIS->get_heap_factory().get_upload_bytes() / 1024 / 1024;
        }

        size_t Device::get_readback_heap()
        {
            auto THIS = static_cast<HAL::Device*>(this);
            return THIS->get_heap_factory().get_readback_bytes() / 1024 / 1024;
        }

        void Device::init(DeviceDesc& desc)
        {
            auto THIS = static_cast<HAL::Device*>(this);

            D3D12CreateDevice(
                desc.adapter->native_adapter.Get(),
                D3D_FEATURE_LEVEL_12_2,
                IID_PPV_ARGS(&native_device)
            );

            if (!native_device) return;

            THIS->adapter = desc.adapter;
            THIS->properties.name = convert(std::wstring_view(desc.adapter->get_desc().Description));

            for (auto type : magic_enum::enum_values<DescriptorHeapType>())
            {
                descriptor_sizes[type] = native_device->GetDescriptorHandleIncrementSize(::to_native(type));
            }

            D3D12_FEATURE_DATA_D3D12_OPTIONS options = {};
            D3D12_FEATURE_DATA_D3D12_OPTIONS5 options5 = {};
            D3D12_FEATURE_DATA_D3D12_OPTIONS7 options7 = {};
            D3D12_FEATURE_DATA_D3D12_OPTIONS12 options12 = {};
            D3D12_FEATURE_DATA_D3D12_OPTIONS16 options16 = {};
            D3D12_FEATURE_DATA_D3D12_OPTIONS21 options21 = {};
            D3D12_FEATURE_DATA_SHADER_MODEL supportedShaderModel = { D3D_SHADER_MODEL_6_8 };

            TEST(*this,
                 native_device->CheckFeatureSupport(D3D12_FEATURE::D3D12_FEATURE_D3D12_OPTIONS21, &options21, sizeof(
                     options21)));
            TEST(*this,
                 native_device->CheckFeatureSupport(D3D12_FEATURE::D3D12_FEATURE_D3D12_OPTIONS16, &options16, sizeof(
                     options16)));
            TEST(*this,
                 native_device->CheckFeatureSupport(D3D12_FEATURE::D3D12_FEATURE_D3D12_OPTIONS12, &options12, sizeof(
                     options12)));
            TEST(*this,
                 native_device->CheckFeatureSupport(D3D12_FEATURE::D3D12_FEATURE_D3D12_OPTIONS7, &options7, sizeof(
                     options7)));
            TEST(*this, native_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &options5, sizeof(options5)));
            TEST(*this,
                 native_device->CheckFeatureSupport(D3D12_FEATURE::D3D12_FEATURE_D3D12_OPTIONS, &options, sizeof(options
                 )));
            TEST(*this,
                 native_device->CheckFeatureSupport(D3D12_FEATURE::D3D12_FEATURE_SHADER_MODEL, &supportedShaderModel,
                     sizeof(supportedShaderModel)));

            auto& properties = THIS->properties;
            properties.rtx                    = options5.RaytracingTier != D3D12_RAYTRACING_TIER_NOT_SUPPORTED;
            properties.full_bindless          = supportedShaderModel.HighestShaderModel >= D3D_SHADER_MODEL_6_6;
            properties.mesh_shader            = options7.MeshShaderTier >= D3D12_MESH_SHADER_TIER_1;
            properties.direct_gpu_upload_heap = options16.GPUUploadHeapSupported;
            properties.work_graph             = false;//options21.WorkGraphsTier != D3D12_WORK_GRAPHS_TIER_NOT_SUPPORTED;

            if constexpr (HAL::Debug::ValidationErrors)
            {
                ComPtr<ID3D12InfoQueue> d3dInfoQueue;
                if (SUCCEEDED(native_device.As(&d3dInfoQueue)))
                {
                    D3D12_MESSAGE_ID hide[] =
                    {
                        D3D12_MESSAGE_ID::D3D12_MESSAGE_ID_HEAP_ADDRESS_RANGE_INTERSECTS_MULTIPLE_BUFFERS,
                        D3D12_MESSAGE_ID::D3D12_MESSAGE_ID_GETHEAPPROPERTIES_INVALIDRESOURCE,
                        D3D12_MESSAGE_ID::D3D12_MESSAGE_ID_CREATERESOURCE_INVALIDALIGNMENT,
                        D3D12_MESSAGE_ID::D3D12_MESSAGE_ID_NON_RETAIL_SHADER_MODEL_WONT_VALIDATE,
                        D3D12_MESSAGE_ID::D3D12_MESSAGE_ID_CREATEPIPELINESTATE_CACHEDBLOBDESCMISMATCH,
                        D3D12_MESSAGE_ID::D3D12_MESSAGE_ID_EMPTY_DISPATCH,
                        D3D12_MESSAGE_ID::D3D12_MESSAGE_ID_RESOLVE_QUERY_INVALID_QUERY_STATE,
                        D3D12_MESSAGE_ID::D3D12_MESSAGE_ID_NON_OPTIMAL_BARRIER_ONLY_EXECUTE_COMMAND_LISTS,
                        D3D12_MESSAGE_ID(1380)
                        //D3D12_MESSAGE_ID::D3D12_MESSAGE_ID_CREATERESOURCE_INVALIDALIGNMENT_SMALLRESOURCE
                    };

                    D3D12_INFO_QUEUE_FILTER filter = {};
                    filter.DenyList.NumIDs = _countof(hide);
                    filter.DenyList.pIDList = hide;
                    d3dInfoQueue->AddStorageFilterEntries(&filter);
                }
            }

            DSTORAGE_CONFIGURATION ds_config{};
            DStorageSetConfiguration(&ds_config);

            DStorageCreateCompressionCodec(
                DSTORAGE_COMPRESSION_FORMAT_GDEFLATE,
                0,
                IID_PPV_ARGS(&g_bufferCompression));
        }

        D3D::Device Device::get_native_device()
        {
            return native_device;
        }

        ResourceAllocationInfo Device::get_alloc_info(const ResourceDesc& desc)
        {
            auto it = alloc_info.find(desc);
            if (it != alloc_info.end())
            {
                return (it->second);
            }

            auto native_desc = ::to_native(desc);
            if (native_desc.Dimension != D3D12_RESOURCE_DIMENSION_BUFFER)
            {
                if ((native_desc.Flags & (D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET |
                                          D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)) == 0)
                {
                    native_desc.Alignment = D3D12::SMALL_RESOURCE_PLACEMENT_ALIGNMENT;
                }
            }
            if (native_desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)
            {
                ASSERT(native_desc.SampleDesc.Count > 0);
            }

            D3D12_RESOURCE_ALLOCATION_INFO1 info2;

            auto info = native_device->GetResourceAllocationInfo2(0, 1, &native_desc, &info2);
            if (info.SizeInBytes == std::numeric_limits<uint64_t>::max())
            {
                native_desc.Alignment = 0;
                info = native_device->GetResourceAllocationInfo2(0, 1, &native_desc, &info2);
            }
            ASSERT(info.SizeInBytes != std::numeric_limits<uint64_t>::max());

            native_desc.Alignment = info.Alignment;

            ResourceAllocationInfo result;

            result.size = info.SizeInBytes;
            result.alignment = info.Alignment;
            result.flags = HeapFlags::NONE;

            if (native_desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
            {
                result.flags |= HeapFlags::BUFFERS_ONLY;
            }
            else if (native_desc.Flags & (D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET |
                     D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL))
            {
                result.flags |= HeapFlags::RTDS_ONLY;
            }
            else
            {
                result.flags |= HeapFlags::TEXTURES_ONLY;
            }
            if constexpr (Debug::CheckErrors)
                TEST(*this, native_device->GetDeviceRemovedReason());

            alloc_info[desc] = result;
            ASSERT(result.size != std::numeric_limits<uint64_t>::max());
            return result;
        }
    }
}