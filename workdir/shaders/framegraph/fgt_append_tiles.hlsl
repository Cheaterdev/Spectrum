#include "../autogen/FGTAppendTiles.h"

// One thread per tile of grid; appends the tiles the test's CPU predicate
// selects (Test.RenderSystem.FrameGraph.ixx, VSM_TileLists_CounterLayout).
[numthreads(8, 8, 1)]
void CS(uint3 dispatchID : SV_DispatchThreadID)
{
	const uint2 grid = GetFGTAppendTiles().GetGrid();
	if (any(dispatchID.xy >= grid))
		return;
	if ((dispatchID.x * 7 + dispatchID.y * 3) % 5 == 0)
		GetFGTAppendTiles().GetTiles().Append(dispatchID.xy);
}
