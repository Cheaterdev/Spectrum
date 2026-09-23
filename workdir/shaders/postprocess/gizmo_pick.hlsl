#include "../autogen/PickerBuffer.h"
#include "../autogen/Instance.h"

static const RWStructuredBuffer<uint> pickBuffer = GetPickerBuffer().GetViewBuffer();

// Gizmo handles are drawn without a depth buffer, so the nearest one under the
// mouse is resolved here instead: the key is the depth's bit pattern with the
// handle id in the low 4 bits. Depth is reversed-Z (nearer = larger) and
// non-negative floats order the same as their bits, so InterlockedMax keeps
// the nearest handle.
void gizmo_pick(float4 sv_pos)
{
    if (any(uint2(sv_pos.xy) != GetPickerBuffer().GetMouse_pos()))
        return;

    uint key = (asuint(sv_pos.z) & ~0xFu) | (GetInstance().GetInstanceId() & 0xFu);
    InterlockedMax(pickBuffer[0], key);
}
