module Graphics:DDGI;

import RenderSystem;
import Graphics;
import HAL;
import Core;

#include "../../FrameGraph/autogen/pass_defaults.h"

using namespace FrameGraph;
using namespace HAL;

// v1 scaffold (see [[project-ddgi]] planning notes): setup() is fully
// generated (ddgi.sig's own [SetupCondition]/[Always]/[Size] annotations --
// the persistent atlas textures and Probes buffer are created and tracked by
// the FrameGraph itself via ResourceFlags::Static, same pattern as
// VSM_PageTable/sky_cubemap_filtered, no manual HAL::Texture ownership
// needed). These render bodies are intentionally empty placeholders -- probe
// selection, tracing and convolution are filled in as later plan steps land;
// this step only proves the SIG declarations compile, link, and the passes
// appear in the FrameGraph.

void PassDefault<Passes::DDGIProbeSelect>::render(
	Passes::DDGIProbeSelect::Context& data, FrameContext& context)
{
}

void PassDefault<Passes::DDGIProbeTrace>::render(
	Passes::DDGIProbeTrace::Context& data, FrameContext& context)
{
}

void PassDefault<Passes::DDGIProbeConvolve>::render(
	Passes::DDGIProbeConvolve::Context& data, FrameContext& context)
{
}
