#pragma once


#ifndef APPVEYOR 
#ifndef DEBUG
//#define OCULUS_SUPPORT
#endif
#endif



#include "../DirectXFramework/pch.h"
#include "../FlowGraph/pch.h"



#include "autogen/includes.h"


#include "Context/Context.h"
#include "Helpers/MipMapGeneration.h"

#include "Occlusion/Occluder.h"
#include "Scene/SceneObject.h"
#include "Camera/Camera.h"

#include "Font/TextSystem.h"
#include "GUI/GUI.h"
#include "GUI/Skin.h"

#include "GUI/Renderer/Base.h"
#include "GUI/Elements/ColoredRect.h"
#include "GUI/Elements/Image.h"
#include "GUI/Elements/Button.h"
#include "GUI/Elements/Label.h"
#include "GUI/Elements/EditText.h"
#include "GUI/Elements/Dragger.h"
#include "GUI/Elements/Resizer.h"
#include "GUI/Elements/Resizable.h"
#include "GUI/Elements/Window.h"
#include "GUI/Elements/CheckBox.h"
#include "GUI/Elements/HorizontalLayout.h"
#include "GUI/Elements/CheckBoxText.h"
#include "GUI/Elements/OptionBox.h"
#include "GUI/Elements/ScrollBar.h"
#include "GUI/Elements/ScrollContainer.h"
#include "GUI/Elements/ListBox.h"
#include "GUI/Elements/MenuList.h"
#include "GUI/Elements/ComboBox.h"
#include "GUI/Elements/TabControl.h"
#include "GUI/Elements/StatusBar.h"
#include "GUI/Elements/DockBase.h"
#include "GUI/Elements/Container.h"
#include "GUI/Elements/Tree.h"
#include "GUI/Elements/ValueBox.h"
#include "GUI/Elements/CircleSelector.h"
#include "GUI/Elements/FlowGraph/ParameterWindow.h"
#include "GUI/Elements/FlowGraph/ComponentWindow.h"
#include "GUI/Elements/FlowGraph/Comment.h"
#include "GUI/Elements/FlowGraph/Canvas.h"
#include "GUI/Elements/FlowGraph/FlowManager.h"

#include "GUI/Debugging/OutputWindow.h"
#include "GUI/Debugging/TaskViewer.h"
#include "GUI/Debugging/TimerWatcher.h"
#include "GUI/Debugging/TimerGraph.h"


#include "Assets/Asset.h"

#include "Assets/TextureAsset.h"
#include "Assets/MaterialAsset.h"
#include "Assets/BinaryAsset.h"
#include "Assets/TiledTexture.h"

#include "Mesh/vertexes.h"
#include "Mesh/Mesh.h"
#include "Assets/MeshAsset.h"


using shader_identifier = std::array<std::byte, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES>;
static shader_identifier identify(void* data)
{
	shader_identifier result;

	memcpy(result.data(), data, result.size());

	return result;
}
#define CACHE_ALIGN(x) __declspec(align(x))

CACHE_ALIGN(64)
struct closesthit_identifier
{
	shader_identifier identifier;
	D3D12_GPU_VIRTUAL_ADDRESS mat_buffer;
};


#include "Materials/Values.h"
#include "Materials/universal_material.h"
#include "Materials/ShaderMaterial.h" 


#include "Scene/Scene.h"
#include "Renderer/Renderer.h"

#include "Assets/AssetRenderer.h"

#include "GUI/Renderer/Renderer.h"

#include "GUI/Elements/AssetExplorer.h"
#include "Lighting/PSSM.h"
/*
#include "Lighting/GBuffer.h"

#include "Lighting/Light.h"
*/


#include "Renderer/StencilRenderer.h"

#include "TiledTextures/VisibilityBuffer.h"
#include "TiledTextures/GPUTilesBuffer.h"
#include "TiledTextures/Texture3DTiled.h"
#include "TiledTextures/DynamicTileGenerator.h"


#include "Effects/Sky.h"

/*
#include "Effects/TemporalAA.h"

#include "Effects/PostProcess/Adaptation.h"
#include "Effects/PostProcess/SSR.h"
#include "Effects/PostProcess/SSGI.h"
#include "Effects/PostProcess/SMAA.h"
*/
#include "Effects/PostProcess/SMAA.h"
#include "Effects/VoxelGI/VoxelGI.h"
#include "Effects/RTX/RTX.h"




#include "Lighting/BRDF.h"
namespace EngineAssets
{
	extern EngineAsset<MeshAsset> material_tester;
	extern EngineAsset<MeshAsset> axis;
	extern EngineAsset<MeshAsset> plane;
	extern EngineAsset<BinaryAsset> material_header;
	extern EngineAsset<BinaryAsset> material_raytracing_header;


	extern EngineAsset<TextureAsset> best_fit_normals;
	extern EngineAsset<TextureAsset> missing_texture;

	extern EngineAsset<BRDF> brdf;

}
