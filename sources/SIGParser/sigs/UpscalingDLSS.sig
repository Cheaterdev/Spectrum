# DLSS Super Resolution. Same slot as FSR (reads ResultTexture at frame_size,
# recreates ResultTextureNew at upscale_size) - the two are mutually exclusive
# producers, gated by nvidia::DLSS::get().available() in PassDefaults.cpp.
#
# No ComputePSO: unlike FSR's EASU/RCAS, DLSS-SR is not a shader this engine
# dispatches - it's an opaque call into the Streamline SDK operating on tagged
# resources via the command list (see PassDefault<Passes::UpscalingDLSS>::render).
[Static]
PassNode UpscalingDLSS
{
	Texture GBuffer_Depth;
	Texture GBuffer_Speed;

	[Write] [Recreate = ResultTextureNew]
	Texture ResultTexture;
}
