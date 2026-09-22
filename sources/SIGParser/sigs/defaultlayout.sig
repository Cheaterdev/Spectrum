layout FrameLayout { 
	slot CameraData;
	slot SceneData;
	slot PassData;

	slot DebugInfo;
	
	Sampler linearSampler = SamplerLinearWrapDesc;
	Sampler pointClampSampler = SamplerPointClampDesc;
	Sampler linearClampSampler = SamplerLinearClampDesc;
	Sampler anisoBordeSampler = SamplerAnisoBorderDesc;
	Sampler pointBorderSampler = SamplerPointBorderDesc;
	Sampler vsmShadowSampler = SamplerShadowComparisonDesc;
}

layout DefaultLayout: FrameLayout 
{
	slot Instance0;
	slot Instance1;
	slot Instance2;
	slot Instance3;
	slot Instance4;
	slot Instance5;


	slot Raytracing;

	slot MaterialData;

	slot WorkGR_ClassifyPixels_NodeEmulation;
	slot WorkGR_Shadows_NodeEmulation;

	# VSMShadowLookupData (vsm.sig). Its own slot because the reader is the
	# shared RTX hit shader, and every Instance0-5 slot is already taken
	# inside that one library.
	slot VSMShadow;
}

struct DebugStruct
{
	uint format_id;
	uint4 args;
}

layout NoneLayout {
	slot None;
}

# Printf-style shader debug logging. Call as GetDebugInfo().Log("dist=%f
# count=%u", asuint(dist), count) -- a build-time DXC preprocess pass
# (DXC.ShaderCompiler.cpp) rewrites the literal string into a stable
# uint id (hash of the string) before the real compile ever sees it, and
# registers id -> format string for CommandList::print_debug() to format
# on readback. Args are always raw bits (asuint() a float yourself); the
# %f/%u/%d in the format string is what tells the CPU side how to
# reinterpret them -- this is what keeps Log() to a handful of overloads
# instead of one per int/uint/float combination.
[Bind = FrameLayout::DebugInfo]
struct DebugInfo
{
	RWStructuredBuffer<DebugStruct> debug;
	RWStructuredBuffer<uint> logCount;

	%{
		void LogWrite(uint id, uint4 args)
		{
			uint slot;
			InterlockedAdd(GetLogCount()[0], 1, slot);

			if (slot < 64)
			{
				DebugStruct e;
				e.format_id = id;
				e.args = args;
				GetDebug()[slot] = e;
			}
		}

		void Log(uint id) { LogWrite(id, uint4(0, 0, 0, 0)); }
		void Log(uint id, uint a0) { LogWrite(id, uint4(a0, 0, 0, 0)); }
		void Log(uint id, uint a0, uint a1) { LogWrite(id, uint4(a0, a1, 0, 0)); }
		void Log(uint id, uint a0, uint a1, uint a2) { LogWrite(id, uint4(a0, a1, a2, 0)); }
		void Log(uint id, uint a0, uint a1, uint a2, uint a3) { LogWrite(id, uint4(a0, a1, a2, a3)); }
	}%
}
