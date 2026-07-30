# This is the list of runtime properties to pass to the shader
# Wherever possible, it is highly recommended to have these values be compile-time constants

[Bind = DefaultLayout::Instance2]
struct DispatchParameters
{
	# Visual configuration:
	# These values will require manual tuning.
	# All shadow computation is performed in non-linear depth space (not in world space), so tuned value choices will depend on scene depth distribution (as determined by the Projection Matrix setup).

	float SurfaceThickness = 0.005;			# This is the assumed thickness of each pixel for shadow-casting, measured as a percentage of the difference in non-linear depth between the sample and FarDepthValue.
											# Recommended starting value: 0.005 (0.5%)

	float BilinearThreshold = 0.02;			# Percentage threshold for determining if the difference between two depth values represents an edge, and should not perform interpolation.
											# To tune this value, set 'DebugOutputEdgeMask' to true to visualize where edges are being detected.
											# Recommended starting value: 0.02 (2%)

	float ShadowContrast = 4;				# A contrast boost is applied to the transition in/out of shadow.
											# Recommended starting value: 2 or 4. Values >= 1 are valid.

	uint IgnoreEdgePixels = false;			# If an edge is detected, the edge pixel will not contribute to the shadow.
											# If a very flat surface is being lit and rendered at an grazing angles, the edge detect may incorrectly detect multiple 'edge' pixels along that flat surface.
											# In these cases, the grazing angle of the light may subsequently produce aliasing artefacts in the shadow where these incorrect edges were detected.
											# Setting this value to true would mean that those pixels would not cast a shadow, however it can also thin out otherwise valid shadows, especially on foliage edges.
											# Recommended starting value: false, unless typical scenes have numerous large flat surfaces, in which case true.

	uint UsePrecisionOffset = false;		# A small offset is applied to account for an imprecise depth buffer (recommend off)


	uint BilinearSamplingOffsetMode = false;# There are two modes to compute bilinear samples for shadow depth:
											# true = sampling points for pixels are offset to the wavefront shared ray, shadow depths and starting depths are the same. Can project more jagged/aliased shadow lines in some cases.
											# false = sampling points for pixels are not offset and start from pixel centers. Shadow depths are biased based on depth gradient across the current pixel bilinear sample. Has more issues in back-face / grazing areas.
											# Both modes have subtle visual differences, which may / may not exaggerate depth buffer aliasing that gets projected in to the shadow.
											# Evaluating the visual difference between each mode is recommended, then hard-coding the mode used to optimize the shader.
											# Recommended starting value: false

	# Debug views
	uint DebugOutputEdgeMask = false;		# Use this to visualize edges, for tuning the 'BilinearThreshold' value.
	uint DebugOutputThreadIndex = false;	# Debug output to visualize layout of compute threads
	uint DebugOutputWaveIndex = false;		# Debug output to visualize layout of compute wavefronts, useful to sanity check the Light Coordinate is being computed correctly.

	# Culling / Early out:
	float2 DepthBounds = float2(0,1);		# Depth Bounds (min, max) for the on-screen volume of the light. Typically (0,1) for directional lights. Only used when 'UseEarlyOut' is true.

	uint UseEarlyOut = false;				# Set to true to early-out when depth values are not within [DepthBounds] - otherwise DepthBounds is unused
											# [Optionally customize the 'EarlyOutPixel()' function to perform your own early-out logic, e.g. skipping pixels that a shadow map indicates are already fully occluded]
											# This can dramatically reduce cost when only a small portion of the pixels need a shadow term (e.g., cull out sky pixels), however it does have some overhead (~15%) in worst-case where nothing early-outs
											# Note; Early-out is most efficient when WAVE_SIZE matches the hardware wavefront size - otherwise cross wave communication is required.


	# Runtime data returned from BuildDispatchList():
	float4 LightCoordinate;					# Values stored in DispatchList::LightCoordinate_Shader by BuildDispatchList()
	int2 WaveOffset;						# Values stored in DispatchData::WaveOffset_Shader by BuildDispatchList()

	# render_size / upscale_size (<=1). Scales the cooperative-loading search
	# step in WriteScreenSpaceShadow (NOT the wavefront/tile geometry in
	# ComputeWavefrontExtents - that must stay exactly WAVE_SIZE-periodic),
	# so the shadow's world-space reach stays constant across DLSS render
	# scales instead of always meaning "N render-resolution pixels."
	float PixelStepScale = 1;

	# Renderer Specific Values:
	float FarDepthValue;					# Set to the Depth Buffer Value for the far clip plane, as determined by renderer projection matrix setup (typically 0).
	float NearDepthValue;					# Set to the Depth Buffer Value for the near clip plane, as determined by renderer projection matrix setup (typically 1).

	# Sampling data:
	float2 InvDepthTextureSize;				# Inverse of the texture dimensions for 'DepthTexture' (used to convert from pixel coordinates to UVs)
											# If 'PointBorderSampler' is an Unnormalized sampler, then this value can be hard-coded to 1.
											# The 'USE_HALF_PIXEL_OFFSET' macro might need to be defined if sampling at exact pixel coordinates isn't precise (e.g., if odd patterns appear in the shadow).

	Texture2D<float> DepthTexture;			# Depth Buffer Texture (rasterized non-linear depth)
	RWTexture2D<float4> OutputTexture;		# Output screen-space shadow buffer (typically single-channel, 8bit)

#	SamplerState PointBorderSampler;		# A point sampler, with Wrap Mode set to Clamp-To-Border-Color (D3D12_TEXTURE_ADDRESS_MODE_BORDER), and Border Color set to "FarDepthValue" (typically zero), or some other far-depth value out of DepthBounds.
											# If you have issues where invalid shadows are appearing from off-screen, it is likely that this sampler is not correctly setup
}




ComputePSO SS_Shadow
{
	root = DefaultLayout;

	[EntryPoint = CS]
	compute = SS_Shadow;
}


