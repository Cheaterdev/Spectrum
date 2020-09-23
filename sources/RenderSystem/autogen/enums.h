enum class Layouts: int
{
	UNKNOWN,
	FrameLayout,
	DefaultLayout,
	TOTAL
};
enum class PSO: int
{
	UNKNOWN,
	BRDF,
	GatherPipeline,
	GatherBoxes,
	InitDispatch,
	GatherMeshes,
	DownsampleDepth,
	MipMapping,
	Lighting,
	VoxelDownsample,
	FontRender,
	FontRenderClip,
	RenderBoxes,
	RenderToDS,
	QualityColor,
	QualityToStencil,
	QualityToStencilREfl,
	PSSMMask,
	PSSMApply,
	GBufferDownsample,
	Sky,
	SkyCube,
	CubemapENV,
	CubemapENVDiffuse,
	EdgeDetect,
	BlendWeight,
	Blending,
	DrawStencil,
	DrawSelected,
	DrawBox,
	DrawAxis,
	StencilerLast,
	NinePatch,
	SimpleRect,
	CanvasBack,
	CanvasLines,
	VoxelReflectionLow,
	VoxelIndirectFilter,
	VoxelReflectionHi,
	VoxelReflectionUpsample,
	VoxelIndirectHi,
	VoxelIndirectLow,
	VoxelIndirectUpsample,
	VoxelDebug,
	TOTAL
};