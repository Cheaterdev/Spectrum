#include "../autogen/GatherPipelineGlobal.h"
#include "../autogen/SceneData.h"
#include "../autogen/DebugViewGather.h"

#ifdef BUILD_FUNC_CS
[numthreads(64, 1, 1)]
void CS(uint3 dispatchID : SV_DispatchThreadID)
{
    GatherPipelineGlobal pip = GetGatherPipelineGlobal();
    if (dispatchID.x >= pip.GetMeshes_count()[0]) return;

    uint id = pip.GetCommands().Load(dispatchID.x);
    MeshCommandData mesh = GetSceneData().GetMeshes()[id];
    MaterialCommandData material = GetSceneData().GetMaterials()[mesh.material_id];

    CommandData command;
    command.material_cb = material.material_cb;
    command.mesh_cb = mesh.mesh_cb;
    command.meshinstance_cb = mesh.meshinstance_cb;
    command.draw_commands = mesh.draw_commands;

    GetDebugViewGather().GetCommands().Append(command);
}
#endif
