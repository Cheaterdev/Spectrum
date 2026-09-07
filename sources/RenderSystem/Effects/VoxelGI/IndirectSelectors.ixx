export module Graphics:IndirectSelectors;

export
{
	// Which raw signal feeds NRD REBLUR_DIFFUSE, set by main.cpp's Indirect
	// Src combobox (see [[project-nrd-integration]]). MyVCT = VoxelScreen's
	// voxel-cone-traced signal; RTXReference = IndirectRTX's raw RTX
	// reference. Purely a quality/performance comparison -- NRD is the only
	// denoiser for either.
	enum class IndirectSource { MyVCT, RTXReference };
	IndirectSource g_indirect_source = IndirectSource::RTXReference;

	// Same idea for REBLUR_SPECULAR: ScreenReflection's voxel-cone-traced
	// signal vs. ReflectionRTX's raw RTX reference.
	enum class ReflectionSource { MyReflection, RTXReference };
	ReflectionSource g_reflection_source = ReflectionSource::RTXReference;
}
