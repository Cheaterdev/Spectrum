[Bind = DefaultLayout::Instance0]
struct BlueNoise
{
	uint frame_index;
	Buffer<uint> sobol_buffer;
	Buffer<uint> ranking_tile_buffer;
	Buffer<uint> scrambling_tile_buffer;

	RWTexture2D<float2> blue_noise_texture;
}


ComputePSO BlueNoise
{
	root = DefaultLayout;

	[EntryPoint = CS]
	[Enable16bits]
	compute = postprocess/blue_noise;
}

[Compute]
[RunAlways]
PassNode BlueNoise
{
	[Always = UnorderedAccess] [Size = 128] [Format = R8G8_UNORM] Texture BlueNoise;
}