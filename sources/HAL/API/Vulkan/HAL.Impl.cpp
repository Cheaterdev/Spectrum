module;
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
module HAL:Impl;

import stl.core;
import Core;
import :Adapter;
import :Debug;

// ============================================================================
//  Static Vulkan instance — created once in HAL::init(), shared by
//  Adapters::enumerate() and API::Device::init().
// ============================================================================

namespace
{
    VkInstance                   g_instance        = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT     g_debug_messenger = VK_NULL_HANDLE;

    VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
        VkDebugUtilsMessageSeverityFlagBitsEXT      severity,
        VkDebugUtilsMessageTypeFlagsEXT             /*type*/,
        const VkDebugUtilsMessengerCallbackDataEXT* data,
        void* /*user*/)
    {
        if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
            Log::get() << Log::LEVEL_WARNING << "[Vulkan] " << data->pMessage << Log::endl;
        return VK_FALSE;
    }
}

namespace HAL
{
    VkInstance               get_vk_instance()        { return g_instance; }
    VkDebugUtilsMessengerEXT get_vk_debug_messenger() { return g_debug_messenger; }

    void EnableGPUDebug()
    {
        // Validation is configured during instance creation in init().
    }

    void EnableShaderModel()
    {
        // No-op: SPIR-V shaders have no "shader model" negotiation.
    }

    void init()
    {
        // ---- Application / instance info ------------------------------------
        VkApplicationInfo app_info{ VK_STRUCTURE_TYPE_APPLICATION_INFO };
        app_info.pApplicationName   = "Spectrum";
        app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        app_info.pEngineName        = "Spectrum";
        app_info.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
        app_info.apiVersion         = VK_API_VERSION_1_3;

        // ---- Extensions -----------------------------------------------------
        std::vector<const char*> extensions = {
            VK_KHR_SURFACE_EXTENSION_NAME,
            VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
        };

        // ---- Probe available instance layers --------------------------------
        uint32_t layer_count = 0;
        vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
        std::vector<VkLayerProperties> avail_layers(layer_count);
        vkEnumerateInstanceLayerProperties(&layer_count, avail_layers.data());

        auto layer_available = [&](const char* name) {
            for (auto& l : avail_layers)
                if (strcmp(l.layerName, name) == 0) return true;
            return false;
        };

        // ---- Probe available instance extensions ----------------------------
        uint32_t inst_ext_count = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &inst_ext_count, nullptr);
        std::vector<VkExtensionProperties> avail_inst_exts(inst_ext_count);
        vkEnumerateInstanceExtensionProperties(nullptr, &inst_ext_count, avail_inst_exts.data());

        auto inst_ext_available = [&](const char* name) {
            for (auto& e : avail_inst_exts)
                if (strcmp(e.extensionName, name) == 0) return true;
            return false;
        };

        // ---- Layers + debug messenger ---------------------------------------
        std::vector<const char*> layers;
        VkDebugUtilsMessengerCreateInfoEXT debug_info{
            VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };
        bool use_debug_utils = false;

        if constexpr (Debug::CheckErrors)
        {
            if (layer_available("VK_LAYER_KHRONOS_validation"))
                layers.push_back("VK_LAYER_KHRONOS_validation");
            else
                Log::get() << Log::LEVEL_WARNING
                    << "[Vulkan] VK_LAYER_KHRONOS_validation not found — "
                       "install the Vulkan SDK for validation support" << Log::endl;

            if (inst_ext_available(VK_EXT_DEBUG_UTILS_EXTENSION_NAME))
            {
                extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
                use_debug_utils = true;

                debug_info.messageSeverity =
                    VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                    VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
                debug_info.messageType =
                    VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT     |
                    VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT  |
                    VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
                debug_info.pfnUserCallback = debug_callback;
            }
        }

        // ---- Create instance ------------------------------------------------
        VkInstanceCreateInfo create_info{ VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
        create_info.pApplicationInfo        = &app_info;
        create_info.enabledExtensionCount   = static_cast<uint32_t>(extensions.size());
        create_info.ppEnabledExtensionNames = extensions.data();
        create_info.enabledLayerCount       = static_cast<uint32_t>(layers.size());
        create_info.ppEnabledLayerNames     = layers.data();
        if (use_debug_utils)
            create_info.pNext = &debug_info;

        VkResult result = vkCreateInstance(&create_info, nullptr, &g_instance);
        if (result != VK_SUCCESS)
        {
            Log::get().crash_error(
                std::string("vkCreateInstance failed, VkResult=") + std::to_string(static_cast<int>(result)));
            return;
        }

        // ---- Debug messenger ------------------------------------------------
        if (use_debug_utils)
        {
            auto fn = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
                vkGetInstanceProcAddr(g_instance, "vkCreateDebugUtilsMessengerEXT"));
            if (fn)
                fn(g_instance, &debug_info, nullptr, &g_debug_messenger);
        }

        // ---- Adapters singleton (uses g_instance for enumeration) -----------
        HAL::Adapters::create();
    }
}
