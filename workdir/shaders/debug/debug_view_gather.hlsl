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

    DebugViewGather gather = GetDebugViewGather();

    // Buckets: see DebugViewGather in debug_view.prism.
    uint bucket = 0;
    if (gather.GetSource() != DebugViewSource::All)
    {
        // Stamp = (frame << 2) | stage, stage 1 = stage 1 / direct, 2 = retest.
        uint stamp = gather.GetStamps()[id];
        uint stage = stamp & 3;
        bool rendered = stage != 0 && (stamp >> 2) == gather.GetCapture_frame();
        bool translucent = material.GetTransparency_mode() == TransparencyMode::Translucent;

        if (rendered)
            bucket = stage == 2 ? 1 : 0;
        else
            bucket = translucent ? 3 : 2;

        if (gather.GetSource() == DebugViewSource::Rendered && !rendered) return;
        // Drawn parts stay in: their culled meshlets are shown, the AS filters
        // out the drawn ones.
        if (gather.GetSource() == DebugViewSource::Culled && bucket == 3) return;
    }

    CommandData command;
    command.material_cb = material.material_cb;
    command.mesh_cb = mesh.mesh_cb;
    command.meshinstance_cb = mesh.meshinstance_cb;
    command.draw_commands = mesh.draw_commands;

    // Literal indices: bucket varies per thread, and a varying descriptor index
    // would need NonUniformResourceIndex (AMD reads the wrong buffer without it).
    if (bucket == 0)      gather.GetCommands(0).Append(command);
    else if (bucket == 1) gather.GetCommands(1).Append(command);
    else if (bucket == 2) gather.GetCommands(2).Append(command);
    else                  gather.GetCommands(3).Append(command);
}
#endif
