enum class Layouts: int
{
	UNKNOWN,
	FrameLayout,
	DefaultLayout,
	TOTAL
	, GENERATE_OPS
};
enum class PSO: int
{
	UNKNOWN,
	BRDF,
	FSR,
	RCAS,
	GatherPipeline,
	GatherBoxes,
	InitDispatch,
	GatherMeshes,
	DownsampleDepth,
	MipMapping,
	Lighting,
	VoxelDownsample,
	VoxelCopy,
	VoxelZero,
	VoxelVisibility,
	VoxelIndirectFilter,
	VoxelIndirectLow,
	DenoiserHistoryFix,
	FrameClassification,
	FrameClassificationInitDispatch,
	FontRender,
	RenderBoxes,
	RenderToDS,
	QualityColor,
	QualityToStencil,
	QualityToStencilREfl,
	CopyTexture,
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
	VoxelReflectionHi,
	VoxelReflectionUpsample,
	VoxelIndirectHi,
	VoxelIndirectUpsample,
	VoxelDebug,
	DenoiserDownsample,
	TOTAL
	, GENERATE_OPS
};
enum class SlotID: unsigned int
{
	TextureRenderer = "TextureRenderer"_crc32 ,
	BRDF = "BRDF"_crc32 ,
	DebugStruct = "DebugStruct"_crc32 ,
	DebugInfo = "DebugInfo"_crc32 ,
	FontRendering = "FontRendering"_crc32 ,
	FontRenderingConstants = "FontRenderingConstants"_crc32 ,
	Glyph = "Glyph"_crc32 ,
	FontRenderingGlyphs = "FontRenderingGlyphs"_crc32 ,
	Frustum = "Frustum"_crc32 ,
	Camera = "Camera"_crc32 ,
	FrameInfo = "FrameInfo"_crc32 ,
	FSRConstants = "FSRConstants"_crc32 ,
	FSR = "FSR"_crc32 ,
	MaterialInfo = "MaterialInfo"_crc32 ,
	mesh_vertex_input = "mesh_vertex_input"_crc32 ,
	AABB = "AABB"_crc32 ,
	node_data = "node_data"_crc32 ,
	MeshInfo = "MeshInfo"_crc32 ,
	Meshlet = "Meshlet"_crc32 ,
	DrawIndexedArguments = "DrawIndexedArguments"_crc32 ,
	DispatchArguments = "DispatchArguments"_crc32 ,
	GPUAddress = "GPUAddress"_crc32 ,
	MeshInstance = "MeshInstance"_crc32 ,
	CommandData = "CommandData"_crc32 ,
	MeshCommandData = "MeshCommandData"_crc32 ,
	MaterialCommandData = "MaterialCommandData"_crc32 ,
	GatherPipelineGlobal = "GatherPipelineGlobal"_crc32 ,
	GatherPipeline = "GatherPipeline"_crc32 ,
	BoxInfo = "BoxInfo"_crc32 ,
	GatherBoxes = "GatherBoxes"_crc32 ,
	DrawBoxes = "DrawBoxes"_crc32 ,
	InitDispatch = "InitDispatch"_crc32 ,
	GatherMeshesBoxes = "GatherMeshesBoxes"_crc32 ,
	MipMapping = "MipMapping"_crc32 ,
	CopyTexture = "CopyTexture"_crc32 ,
	DownsampleDepth = "DownsampleDepth"_crc32 ,
	GBuffer = "GBuffer"_crc32 ,
	GBufferDownsample = "GBufferDownsample"_crc32 ,
	GBufferQuality = "GBufferQuality"_crc32 ,
	PSSMConstants = "PSSMConstants"_crc32 ,
	PSSMData = "PSSMData"_crc32 ,
	PSSMDataGlobal = "PSSMDataGlobal"_crc32 ,
	PSSMLighting = "PSSMLighting"_crc32 ,
	RaytracingRays = "RaytracingRays"_crc32 ,
	RayCone = "RayCone"_crc32 ,
	RayPayload = "RayPayload"_crc32 ,
	ShadowPayload = "ShadowPayload"_crc32 ,
	Triangle = "Triangle"_crc32 ,
	Raytracing = "Raytracing"_crc32 ,
	SceneData = "SceneData"_crc32 ,
	SkyData = "SkyData"_crc32 ,
	SkyFace = "SkyFace"_crc32 ,
	EnvFilter = "EnvFilter"_crc32 ,
	EnvSource = "EnvSource"_crc32 ,
	SMAA_Global = "SMAA_Global"_crc32 ,
	SMAA_Weights = "SMAA_Weights"_crc32 ,
	SMAA_Blend = "SMAA_Blend"_crc32 ,
	Countour = "Countour"_crc32 ,
	DrawStencil = "DrawStencil"_crc32 ,
	PickerBuffer = "PickerBuffer"_crc32 ,
	Instance = "Instance"_crc32 ,
	Color = "Color"_crc32 ,
	Test = "Test"_crc32 ,
	vertex_input = "vertex_input"_crc32 ,
	NinePatch = "NinePatch"_crc32 ,
	ColorRect = "ColorRect"_crc32 ,
	FlowGraph = "FlowGraph"_crc32 ,
	VSLine = "VSLine"_crc32 ,
	LineRender = "LineRender"_crc32 ,
	VoxelTilingParams = "VoxelTilingParams"_crc32 ,
	VoxelInfo = "VoxelInfo"_crc32 ,
	Voxelization = "Voxelization"_crc32 ,
	VoxelScreen = "VoxelScreen"_crc32 ,
	VoxelOutput = "VoxelOutput"_crc32 ,
	VoxelBlur = "VoxelBlur"_crc32 ,
	VoxelUpscale = "VoxelUpscale"_crc32 ,
	VoxelVisibility = "VoxelVisibility"_crc32 ,
	VoxelMipMap = "VoxelMipMap"_crc32 ,
	VoxelCopy = "VoxelCopy"_crc32 ,
	VoxelZero = "VoxelZero"_crc32 ,
	VoxelLighting = "VoxelLighting"_crc32 ,
	VoxelDebug = "VoxelDebug"_crc32 ,
	DenoiserDownsample = "DenoiserDownsample"_crc32 ,
	TilingParams = "TilingParams"_crc32 ,
	DenoiserHistoryFix = "DenoiserHistoryFix"_crc32 ,
	TilingPostprocess = "TilingPostprocess"_crc32 ,
	FrameClassification = "FrameClassification"_crc32 ,
	FrameClassificationInitDispatch = "FrameClassificationInitDispatch"_crc32 ,
	GENERATE_OPS
};
