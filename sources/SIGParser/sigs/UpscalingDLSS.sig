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
	# ExclusiveRead: this pass transitions these to PRESENT/COMMON for
	# Streamline, which must not be folded into the shared SRV read-window
	# every other pass uses for them.
	[Always = Read | ExclusiveRead] Texture GBuffer_Depth;
	[Always = Read | ExclusiveRead] Texture GBuffer_Speed;

	# MipCount=0: full auto mip chain, matching the original manual recreate()
	# call's omitted 4th Desc field (value-initializes to 0, not the more
	# common single-mip 1 -- see FrameGraph.cpp's mip_count==0 auto-chain path).
	[Always = Read] [Recreate = ResultTextureNew] [RecreateFlags = UnorderedAccess]
	[Size = ViewportContext::upscale_size] [Format = R16G16B16A16_FLOAT] [MipCount = 0]
	Texture ResultTexture;
}
