// VulkanTest — stub definitions for every PassDefault body that MainPipeline
// references.  MainPipeline::add_passes is compiled into rendersystem.lib;
// VulkanTest never calls it, but the linker still requires every symbol it
// touches to be present.
//
// Spectrum's main.cpp provides the real implementations.  Here we provide
// no-op stubs so VulkanTest can link without the full Spectrum app code.

