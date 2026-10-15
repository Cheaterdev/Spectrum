export module Test.MaterialX;

#define TEST_MODULE_ID MaterialX

export import Test.Framework;
export import Test.HAL.TextureUtils;

import Core;
import HAL;
import RenderSystem;
import Graphics;

// Tests for the MaterialX node library: the registry, every node's HLSL, node
// values evaluated on the GPU, and the .mtlx importer.
//
// Values come from two places. Simple nodes have a result that follows from
// their definition, written out by hand. The noise functions have no closed
// form, so their expected values come from a separate Python port of OSL's
// noise (the algorithm MaterialX's own shaders use), written independently of
// the HLSL: a transcription slip in one shows up as a disagreement. The
// script is workdir/materialx/noise_reference.py; its output is what the noise
// tables below hold.

// One GPU category for the node and importer tests: creating and resetting the
// graphics system twice in a row (one category each) crashed intermittently
// when the second create followed the first reset.
SETUP_CATEGORY(MaterialX.GPU, []() {
	if (!GraphicsSystem::create())
		Test::TestRegistry::Instance().SkipCategory("MaterialX.GPU", "no suitable GPU device found");
});

TEARDOWN_CATEGORY(MaterialX.GPU, []() {
	GraphicsSystem::reset();
});

export namespace Test
{
	namespace mx_detail
	{
		struct Case
		{
			std::string name;
			std::string expr;      // HLSL expression convertible through mxt()
			float4      expected;
			float       tolerance = 1e-4f;
		};

		inline std::string read_text(const std::filesystem::path& path)
		{
			std::ifstream file(path, std::ios::binary);
			std::stringstream ss;
			ss << file.rdbuf();
			return ss.str();
		}

		// mx_stdlib.hlsl with its #include "mx_generated.hlsl" spliced in, so a
		// test shader needs no include handler.
		inline std::string stdlib_source()
		{
			auto dir  = to_path(L"shaders/materialx");
			auto text = read_text(dir / "mx_stdlib.hlsl");
			auto at   = text.find("#include \"mx_generated.hlsl\"");

			if (at != std::string::npos)
				text.replace(at, std::string("#include \"mx_generated.hlsl\"").size(), read_text(dir / "mx_generated.hlsl"));

			return text;
		}

		inline void ensure_library()
		{
			static bool loaded = false;

			if (!loaded)
			{
				MaterialXLibrary::get().register_nodes(to_path(L"materialx"), to_path(L"shaders/materialx/mx_stdlib.hlsl"));
				loaded = true;
			}
		}

		inline std::string errors_only(const std::string& log)
		{
			std::string res;
			std::istringstream lines(log);

			for (std::string line; std::getline(lines, line);)
				if (line.find("error") != std::string::npos)
					res += line + "\n";

			return res;
		}

		// Evaluates one case per pixel into a float target and reads it back.
		// `prelude` is extra HLSL placed after the MaterialX stdlib.
		inline std::vector<float4> evaluate(const std::vector<Case>& cases, const std::string& prelude = "")
		{
			auto& device = RenderSystem::get().device();
			const uint width = (uint)cases.size();

			std::string source = stdlib_source() + "\n" + prelude + "\n";
			source += R"hlsl(
float4 mxt(float v)  { return float4(v, 0, 0, 0); }
float4 mxt(int v)    { return float4(v, 0, 0, 0); }
float4 mxt(bool v)   { return float4(v ? 1 : 0, 0, 0, 0); }
float4 mxt(float2 v) { return float4(v, 0, 0); }
float4 mxt(float3 v) { return float4(v, 0); }
float4 mxt(float4 v) { return v; }

float4 VS(uint vid : SV_VertexID) : SV_Position
{
    float2 uv = float2((vid & 1) * 2.0, (vid >> 1) * 2.0);
    return float4(uv * 2.0 - 1.0, 0.0, 1.0);
}

float4 PS(float4 pos : SV_Position) : SV_Target
{
    switch ((int)pos.x)
    {
)hlsl";

			for (size_t i = 0; i < cases.size(); i++)
				source += "        case " + std::to_string(i) + ": return mxt(" + cases[i].expr + ");\n";

			source += "    }\n    return float4(-12345, 0, 0, 0);\n}\n";

			std::string errors;
			auto compiled = ::HAL::ShaderCompiler::get().Compile_Shader(source, {}, "ps_6_8", "PS", HAL::ShaderOptions::None, nullptr, "mx_cases", &errors);
			ASSERT_EQ(std::string(), errors_only(errors));
			ASSERT_TRUE(compiled.has_value());

			auto tex = std::make_shared<HAL::TextureResource>(device,
				HAL::ResourceDesc::Tex2D(HAL::Format::R32G32B32A32_FLOAT, { width, 1 }, 1, 1, HAL::ResFlags::RenderTarget),
				HAL::HeapType::DEFAULT);

			SimpleGraphicsPSO mpso("TestMaterialXCases");
			mpso.root_signature = Layouts::DefaultLayout;
			mpso.vertex        = { source, "VS", HAL::ShaderOptions::None, {}, true };
			mpso.pixel         = { source, "PS", HAL::ShaderOptions::None, {}, true };
			mpso.rtv_formats   = { HAL::Format::R32G32B32A32_FLOAT };
			mpso.enable_depth  = false;
			mpso.cull          = HAL::CullMode::None;
			mpso.topology      = HAL::PrimitiveTopologyType::TRIANGLE;
			auto pso = mpso.create(device);

			auto& queue = device.get_queue(HAL::CommandListType::DIRECT);
			auto  list  = queue->get_free_list();
			list->begin(L"MaterialXCases");

			HAL::Texture2DView view(tex, *list);
			HAL::CompiledRT compiled_rt;
			compiled_rt.table_rtv = view.renderTarget;

			auto& gfx = list->get_graphics();
			gfx.set_rtv(compiled_rt, HAL::RTOptions::Default | HAL::RTOptions::ClearColor, 0, 0, vec4(0, 0, 0, 0));
			gfx.set_pipeline(pso);
			gfx.set_topology(HAL::PrimitiveTopologyType::TRIANGLE);
			gfx.draw(3);
			list->execute_and_wait();

			auto data = readback_texture(tex.get());
			auto& mip = data->array[0]->mips[0];

			std::vector<float4> res(width);

			for (uint x = 0; x < width; x++)
			{
				auto px = reinterpret_cast<const float*>(mip->data.data()) + x * 4;
				res[x] = float4(px[0], px[1], px[2], px[3]);
			}

			return res;
		}

		inline void check(const std::vector<Case>& cases, const std::string& prelude = "")
		{
			auto values = evaluate(cases, prelude);

			std::string failures;

			for (size_t i = 0; i < cases.size(); i++)
			{
				auto& e = cases[i].expected;
				auto& a = values[i];

				if (std::abs(a.x - e.x) > cases[i].tolerance || std::abs(a.y - e.y) > cases[i].tolerance ||
					std::abs(a.z - e.z) > cases[i].tolerance || std::abs(a.w - e.w) > cases[i].tolerance)
				{
					std::ostringstream ss;
					ss << cases[i].name << ": expected (" << e.x << ", " << e.y << ", " << e.z << ", " << e.w
						<< ") got (" << a.x << ", " << a.y << ", " << a.z << ", " << a.w << ")\n";
					failures += ss.str();
				}
			}

			ASSERT_EQ(std::string(), failures);
		}
	}

	TEST(MaterialX.Registry, LoadsStandardLibrary)
	{
		mx_detail::ensure_library();
		auto& lib = MaterialXLibrary::get();

		ASSERT_TRUE(lib.get_all().size() >= 360);

		// ND_mix declares (fg, bg, mix): arguments are passed in that order.
		auto* mix = lib.find("ND_mix_color3");
		ASSERT_TRUE(mix != nullptr);
		ASSERT_EQ(size_t(3), mix->inputs.size());
		ASSERT_EQ(std::string("fg"), mix->inputs[0].name);
		ASSERT_EQ(std::string("bg"), mix->inputs[1].name);
		ASSERT_EQ(std::string("mix"), mix->inputs[2].name);

		// Defined by a node graph / inline expression, so only present through
		// the generated file.
		ASSERT_TRUE(lib.find("ND_contrast_color3") != nullptr);
		ASSERT_TRUE(lib.find("ND_screen_color3") != nullptr);

		// Implemented by hand, with its geometry default.
		auto* noise = lib.find("ND_noise3d_float");
		ASSERT_TRUE(noise != nullptr);
		ASSERT_EQ(std::string("Pobject"), noise->inputs[2].geomprop);

		// Image nodes sample through a texture port, not a filename.
		auto* image = lib.find("ND_image_color3");
		ASSERT_TRUE(image != nullptr);
		ASSERT_TRUE(image->inputs[0].type == ShaderParams::get().TEXTURE);

		for (auto& def : lib.get_all())
		{
			ASSERT_TRUE(!def.function.empty());
			ASSERT_TRUE(!def.outputs.empty());
		}
	}

	// Every registered node, called with its own default arguments, compiles.
	// This is the check that catches an HLSL signature drifting from its nodedef.
	TEST(MaterialX.Library, EveryNodeCompiles)
	{
		mx_detail::ensure_library();

		std::string source = mx_detail::stdlib_source();
		size_t count = 0;

		for (auto& def : MaterialXLibrary::get().get_all())
		{
			// Texture nodes emit their own sampling code, not a function call.
			if (def.function == "mx_image")
				continue;

			std::string body, args;

			for (size_t i = 0; i < def.inputs.size(); i++)
			{
				auto& in = def.inputs[i];
				body += "    " + in.type.name + " i" + std::to_string(i) + " = " + in.default_hlsl + ";\n";
				args += (i ? ", " : "") + std::string("i") + std::to_string(i);
			}

			if (def.outputs.size() == 1)
				body += "    " + def.outputs[0].type.name + " r = " + def.function + "(" + args + ");\n";
			else
			{
				for (size_t i = 0; i < def.outputs.size(); i++)
				{
					body += "    " + def.outputs[i].type.name + " r" + std::to_string(i) + ";\n";
					args += (args.empty() ? "" : ", ") + std::string("r") + std::to_string(i);
				}

				body += "    " + def.function + "(" + args + ");\n";
			}

			source += "void test_" + def.name + "()\n{\n" + body + "}\n";
			count++;
		}

		source += "float4 PS() : SV_Target { return 0; }\n";

		std::string errors;
		::HAL::ShaderCompiler::get().Compile_Shader(source, {}, "ps_6_8", "PS", HAL::ShaderOptions::None, nullptr, "mx_all_nodes", &errors);

		ASSERT_TRUE(count >= 300);
		ASSERT_EQ(std::string(), mx_detail::errors_only(errors));
	}

	// Arithmetic, compositing and color nodes against values that follow from
	// their definitions.
	TEST(MaterialX.GPU, MathAndColor)
	{
		using mx_detail::Case;

		mx_detail::check({
			{ "add",             "mx_add(float3(1, 2, 3), float3(0.5, 0.5, 0.5))",         { 1.5f, 2.5f, 3.5f, 0 } },
			{ "subtract_fa",     "mx_subtract_fa(float3(1, 2, 3), 0.5f)",                  { 0.5f, 1.5f, 2.5f, 0 } },
			{ "divide",          "mx_divide(1.0f, 4.0f)",                                  { 0.25f, 0, 0, 0 } },
			{ "modulo negative", "mx_modulo(-0.25f, 1.0f)",                                { 0.75f, 0, 0, 0 } },
			{ "mix is fg-first", "mx_mix(float3(1, 0, 0), float3(0, 0, 1), 0.25f)",        { 0.25f, 0, 0.75f, 0 } },
			{ "smoothstep mid",  "mx_smoothstep(0.5f, 0.0f, 1.0f)",                        { 0.5f, 0, 0, 0 } },
			{ "smoothstep q",    "mx_smoothstep(0.25f, 0.0f, 1.0f)",                       { 0.15625f, 0, 0, 0 } },
			{ "remap",           "mx_remap(0.5f, 0.0f, 1.0f, 10.0f, 20.0f)",               { 15.0f, 0, 0, 0 } },
			{ "range",           "mx_range(0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 10.0f, false)",   { 5.0f, 0, 0, 0 } },
			{ "clamp",           "mx_clamp(3.0f, 0.0f, 1.0f)",                             { 1.0f, 0, 0, 0 } },
			{ "ifgreater",       "mx_ifgreater(2.0f, 1.0f, 10.0f, 20.0f)",                 { 10.0f, 0, 0, 0 } },
			{ "ifequal",         "mx_ifequal(2.0f, 1.0f, 10.0f, 20.0f)",                   { 20.0f, 0, 0, 0 } },
			{ "combine3",        "mx_combine3(1.0f, 2.0f, 3.0f)",                          { 1, 2, 3, 0 } },
			{ "luminance",       "mx_luminance(float3(1, 1, 1), float3(0.2126, 0.7152, 0.0722))", { 1.0f, 0, 0, 0 } },
			{ "hsv to rgb",      "mx_hsvtorgb(float3(0.5, 1.0, 1.0))",                     { 0, 1, 1, 0 } },
			{ "rgb to hsv",      "mx_rgbtohsv(float3(0, 1, 0))",                           { 1.0f / 3.0f, 1, 1, 0 } },
			{ "hsv roundtrip",   "mx_hsvtorgb(mx_rgbtohsv(float3(0.2, 0.6, 0.9)))",        { 0.2f, 0.6f, 0.9f, 0 } },
			{ "contrast",        "mx_contrast_color3(float3(0.25, 0.5, 1.0), float3(2, 2, 2), float3(0.5, 0.5, 0.5))", { 0, 0.5f, 1.5f, 0 } },
			{ "saturate to gray","mx_saturate_color3(float3(1, 0, 0), 0.0f, float3(0.2, 0.7, 0.1))", { 0.2f, 0.2f, 0.2f, 0 } },
			{ "screen",          "mx_screen_color3(float3(0.5, 0.5, 0.5), float3(0.5, 0.5, 0.5), 1.0f)", { 0.75f, 0.75f, 0.75f, 0 } },
			{ "plus half mix",   "mx_plus_color3(float3(0.25, 0.25, 0.25), float3(0.5, 0.5, 0.5), 0.5f)", { 0.625f, 0.625f, 0.625f, 0 } },
			{ "burn",            "mx_burn(0.5f, 0.75f, 1.0f)",                             { 0.5f, 0, 0, 0 } },
			{ "dodge",           "mx_dodge(0.5f, 0.25f, 1.0f)",                            { 0.5f, 0, 0, 0 } },
			{ "rotate2d 90",     "mx_rotate2d(float2(1, 0), 90.0f)",                       { 0, -1, 0, 0 } },
			{ "rotate3d 90",     "mx_rotate3d(float3(1, 0, 0), 90.0f, float3(0, 0, 1))",   { 0, -1, 0, 0 } },
			{ "trianglewave",    "mx_trianglewave_float(0.75f)",                           { 0.25f, 0, 0, 0 } },
			{ "reflect",         "mx_reflect_vector3(float3(1, -1, 0), float3(0, 1, 0))",  { 1, 1, 0, 0 } },
			{ "ramplr",          "mx_ramplr(0.0f, 1.0f, float2(0.25, 0.0))",               { 0.25f, 0, 0, 0 } },
			{ "checker even",    "mx_checkerboard(float3(1, 0, 0), float3(0, 0, 1), float2(2, 2), float2(0, 0), float2(0.25, 0.25))", { 1, 0, 0, 0 } },
			{ "checker odd",     "mx_checkerboard(float3(1, 0, 0), float3(0, 0, 1), float2(2, 2), float2(0, 0), float2(0.75, 0.25))", { 0, 0, 1, 0 } },
			{ "circle inside",   "mx_circle_float(float2(0.5, 0.5), float2(0.5, 0.5), 0.25f)", { 1, 0, 0, 0 } },
			{ "circle outside",  "mx_circle_float(float2(0.9, 0.5), float2(0.5, 0.5), 0.25f)", { 0, 0, 0, 0 } },
			{ "normalmap flat",  "mx_normalmap(float3(0.5, 0.5, 1.0), 1.0f, float3(0, 0, 1), float3(1, 0, 0), float3(0, 1, 0))", { 0, 0, 1, 0 } },
		});
	}

	// Noise against the independent Python port: lattice points, golden values
	// and the relations between nodes (one-octave fractal is plain noise).
	TEST(MaterialX.GPU, Noise)
	{
		using mx_detail::Case;

		mx_detail::check({
			{ "perlin2 a",   "mx_noise2d(1.0f, 0.0f, float2(0.3, 0.7))",        { 0.040744f, 0, 0, 0 } },
			{ "perlin2 b",   "mx_noise2d(1.0f, 0.0f, float2(1.5, 2.25))",       { 0.12825f, 0, 0, 0 } },
			{ "perlin2 c",   "mx_noise2d(1.0f, 0.0f, float2(-0.8, 3.1))",       { 0.314508f, 0, 0, 0 } },
			{ "perlin2 d",   "mx_noise2d(1.0f, 0.0f, float2(10.2, -4.6))",      { 0.257257f, 0, 0, 0 } },
			{ "perlin2 centre", "mx_noise2d(1.0f, 0.0f, float2(0.5, 0.5))",     { 0.0f, 0, 0, 0 } },
			{ "perlin3 a",   "mx_noise3d(1.0f, 0.0f, float3(0.3, 0.7, 0.1))",   { 0.189201f, 0, 0, 0 } },
			{ "perlin3 b",   "mx_noise3d(1.0f, 0.0f, float3(1.5, 2.25, -3.5))", { -0.008511f, 0, 0, 0 } },
			{ "perlin3 c",   "mx_noise3d(1.0f, 0.0f, float3(-0.8, 3.1, 0.9))",  { -0.035441f, 0, 0, 0 } },
			{ "perlin3 d",   "mx_noise3d(1.0f, 0.0f, float3(10.2, -4.6, 7.7))", { 0.04258f, 0, 0, 0 } },
			{ "perlin3 centre", "mx_noise3d(1.0f, 0.0f, float3(0.5, 0.5, 0.5))", { 0.12275f, 0, 0, 0 } },
			{ "perlin lattice", "mx_noise3d(1.0f, 0.0f, float3(2, 3, 4))",      { 0.0f, 0, 0, 0 } },
			{ "amplitude and pivot", "mx_noise3d(2.0f, 0.5f, float3(0.3, 0.7, 0.1))", { 2 * 0.189201f + 0.5f, 0, 0, 0 } },
			{ "cell2 a",     "mx_cellnoise2d(float2(0.3, 0.7))",                { 0.860313f, 0, 0, 0 } },
			{ "cell2 b",     "mx_cellnoise2d(float2(1.5, 2.25))",               { 0.063351f, 0, 0, 0 } },
			{ "cell2 same cell", "mx_cellnoise2d(float2(0.9, 0.1))",            { 0.860313f, 0, 0, 0 } },
			{ "cell3 a",     "mx_cellnoise3d(float3(0.3, 0.7, 0.1))",           { 0.611068f, 0, 0, 0 } },
			{ "cell3 b",     "mx_cellnoise3d(float3(1.5, 2.25, -3.5))",         { 0.603939f, 0, 0, 0 } },
			{ "worley2 a",   "mx_worleynoise2d_float(float2(0.3, 0.7), 1.0f, 0)",   { 0.600949f, 0, 0, 0 } },
			{ "worley2 b",   "mx_worleynoise2d_float(float2(1.5, 2.25), 1.0f, 0)",  { 0.34915f, 0, 0, 0 } },
			{ "worley2 c",   "mx_worleynoise2d_float(float2(-0.8, 3.1), 1.0f, 0)",  { 0.446465f, 0, 0, 0 } },
			{ "fractal2 a",  "mx_fractal2d(1.0f, 3, 2.0f, 0.5f, float2(0.3, 0.7))",   { -0.259577f, 0, 0, 0 } },
			{ "fractal2 b",  "mx_fractal2d(1.0f, 3, 2.0f, 0.5f, float2(1.5, 2.25))",  { 0.21095f, 0, 0, 0 } },
			{ "fractal3 a",  "mx_fractal3d(1.0f, 3, 2.0f, 0.5f, float3(0.3, 0.7, 0.1))",   { 0.245121f, 0, 0, 0 } },
			{ "fractal3 b",  "mx_fractal3d(1.0f, 3, 2.0f, 0.5f, float3(1.5, 2.25, -3.5))", { -0.254011f, 0, 0, 0 } },
			{ "fractal one octave", "mx_fractal3d(1.0f, 1, 2.0f, 0.5f, float3(0.3, 0.7, 0.1))", { 0.189201f, 0, 0, 0 } },
		});
	}

	// The clearcoat sun highlight (common/coat.hlsl). At perfect mirror
	// alignment N = V = L with NL = NV = 1: D = 1/(pi a^2), V = 1/4, F = F0 =
	// 0.04, and the lobe is scaled by pi like the rest of the direct term, so
	// the result is 0.01 / a^2 -- exactly 100 for coat roughness 0.1 (a = 0.01).
	// The `time` and `frame` nodes read the value the shader sets per frame
	// (mx_set_time), and the triplanar helpers match MaterialX's nodegraph.
	TEST(MaterialX.GPU, TimeAndTriplanar)
	{
		using mx_detail::Case;

		mx_detail::check({
			{ "time",  "time_probe()",  { 2, 0, 0, 0 } },
			{ "frame", "frame_probe()", { 48, 0, 0, 0 } },

			// Weights: normalised |n|, sharpened by 1/blend, renormalised.
			{ "weights along x",   "mx_triplanar_weights(float3(1, 0, 0), 1.0f)", { 1, 0, 0, 0 } },
			{ "weights diagonal",  "mx_triplanar_weights(normalize(float3(1, 1, 1)), 1.0f)", { 1.0f / 3, 1.0f / 3, 1.0f / 3, 0 }, 1e-4f },
			{ "weights blend 1",   "mx_triplanar_weights(float3(0.75, 0.25, 0.0), 1.0f)", { 0.75f, 0.25f, 0, 0 }, 1e-4f },
			{ "weights sharp",     "mx_triplanar_weights(float3(0.75, 0.25, 0.0), 0.03f)", { 1, 0, 0, 0 }, 1e-3f },

			// UVs per projection axis and upaxis (NG_triplanarprojection's switches).
			{ "uv x, z up", "mx_triplanar_uv(0, float3(1, 2, 3), 2)", { 2, 3, 0, 0 } },
			{ "uv x, y up", "mx_triplanar_uv(0, float3(1, 2, 3), 1)", { 3, 2, 0, 0 } },
			{ "uv y, z up", "mx_triplanar_uv(1, float3(1, 2, 3), 2)", { 1, 3, 0, 0 } },
			{ "uv y, x up", "mx_triplanar_uv(1, float3(1, 2, 3), 0)", { 3, 1, 0, 0 } },
			{ "uv z, z up", "mx_triplanar_uv(2, float3(1, 2, 3), 2)", { 1, 2, 0, 0 } },
			{ "uv z, x up", "mx_triplanar_uv(2, float3(1, 2, 3), 0)", { -2, 1, 0, 0 } },
		}, R"hlsl(
float time_probe()  { mx_set_time(2.0); return mx_time(24.0); }
float frame_probe() { mx_set_time(2.0); return mx_frame(); }
)hlsl");
	}

	TEST(MaterialX.GPU, ClearcoatSunSpecular)
	{
		using mx_detail::Case;

		// The lobe headers include each other; splice them in order with the
		// includes removed, so the test shader needs no include handler.
		auto dir = to_path(L"shaders/common");
		std::string lobes;

		for (const char* name : { "shading_model.hlsl", "coat.hlsl", "sheen.hlsl", "aniso.hlsl", "lobes.hlsl" })
		{
			auto text = mx_detail::read_text(dir / name);

			for (auto at = text.find("#include \""); at != std::string::npos; at = text.find("#include \""))
				text.erase(at, text.find('\n', at) - at);

			lobes += text + "\n";
		}

		const std::string up = "float3(0, 1, 0)";
		const std::string coated = "pack_material(SHADING_MODEL_CLEARCOAT, float3(1.0, 0.1, 0.0))";
		const std::string sheen = "pack_material(SHADING_MODEL_SHEEN, float3(1.0, 0.5, 0.0))";

		mx_detail::check({
			{ "mirror alignment",   "coat_sun_specular(" + coated + ", " + up + ", " + up + ", " + up + ", 1.0)", { 100, 100, 100, 0 }, 0.5f },
			{ "half weight",        "coat_sun_specular(pack_material(SHADING_MODEL_CLEARCOAT, float3(0.5, 0.1, 0.0)), " + up + ", " + up + ", " + up + ", 1.0)", { 50, 50, 50, 0 }, 0.5f },
			{ "in shadow",          "coat_sun_specular(" + coated + ", " + up + ", " + up + ", " + up + ", 0.0)", { 0, 0, 0, 0 } },
			{ "light below horizon","coat_sun_specular(" + coated + ", " + up + ", " + up + ", float3(0, -1, 0), 1.0)", { 0, 0, 0, 0 } },
			{ "standard material",  "coat_sun_specular(pack_material(SHADING_MODEL_STANDARD, 0.0), " + up + ", " + up + ", " + up + ", 1.0)", { 0, 0, 0, 0 } },
			{ "reflection head-on", "coat_reflection(" + coated + ", " + up + ", " + up + ", float3(1, 2, 3))", { 0.04f, 0.08f, 0.12f, 0 } },
			{ "reflection grazing", "coat_reflection(" + coated + ", " + up + ", float3(1, 0, 0), float3(1, 2, 3))", { 1, 2, 3, 0 } },
			{ "reflection half weight", "coat_reflection(pack_material(SHADING_MODEL_CLEARCOAT, float3(0.5, 0.1, 0.0)), " + up + ", " + up + ", float3(1, 2, 3))", { 0.02f, 0.04f, 0.06f, 0 } },
			{ "reflection standard", "coat_reflection(pack_material(SHADING_MODEL_STANDARD, 0.0), " + up + ", " + up + ", float3(1, 2, 3))", { 0, 0, 0, 0 } },
			{ "model round trip",   "float(material_model(pack_material(SHADING_MODEL_CLEARCOAT, float3(0.2, 0.3, 0.4))))", { 1, 0, 0, 0 } },
			{ "param round trip",   "material_params(pack_material(SHADING_MODEL_CLEARCOAT, float3(0.2, 0.3, 0.4)))", { 0.2f, 0.3f, 0.4f, 0 }, 1e-4f },
			{ "graph coat off",     "float(material_model(pack_material_from_graph(0.0, 0.5, 0.0, 0.3, 0.0, 0.0, float3(0, 1, 0), float3(1, 0, 0))))", { 0, 0, 0, 0 } },
			{ "graph coat on",      "float(material_model(pack_material_from_graph(0.8, 0.5, 0.0, 0.3, 0.0, 0.0, float3(0, 1, 0), float3(1, 0, 0))))", { 1, 0, 0, 0 } },
		// A coat and a sheen on one material: one shading model, the coat wins.
			{ "graph coat beats sheen", "float(material_model(pack_material_from_graph(0.8, 0.5, 0.9, 0.3, 0.0, 0.0, float3(0, 1, 0), float3(1, 0, 0))))", { 1, 0, 0, 0 } },
			{ "graph sheen",        "float(material_model(pack_material_from_graph(0.0, 0.5, 0.9, 0.3, 0.0, 0.0, float3(0, 1, 0), float3(1, 0, 0))))", { 2, 0, 0, 0 } },

			// Subsurface: lowest priority, radius = weight * scale * radius, sqrt-encoded.
			{ "graph subsurface",    "float(material_model(pack_material_from_graph(0.0, 0.5, 0.0, 0.3, 0.0, 0.0, 1.0, float3(1, 1, 1), 1.0, " + up + ", float3(1, 0, 0))))", { 4, 0, 0, 0 } },
			{ "aniso beats subsurface", "float(material_model(pack_material_from_graph(0.0, 0.5, 0.0, 0.3, 0.6, 0.0, 1.0, float3(1, 1, 1), 1.0, " + up + ", float3(1, 0, 0))))", { 3, 0, 0, 0 } },
			{ "subsurface radius",   "sss_decode_radius(material_params(pack_material_from_graph(0.0, 0.5, 0.0, 0.3, 0.0, 0.0, 1.0, float3(0.1, 0.4, 1.6), 1.0, " + up + ", float3(1, 0, 0))))", { 0.1f, 0.4f, 1.6f, 0 }, 1e-4f },
			{ "subsurface scaled",   "sss_decode_radius(material_params(pack_material_from_graph(0.0, 0.5, 0.0, 0.3, 0.0, 0.0, 0.5, float3(1, 1, 1), 2.0, " + up + ", float3(1, 0, 0))))", { 1, 1, 1, 0 }, 1e-4f },
			// An 8-bit target keeps small radii to a few percent thanks to the sqrt encoding.
			{ "subsurface 8-bit",    "sss_decode_radius(round(sss_encode_radius(float3(0.01, 0.05, 0.5)) * 255.0) / 255.0)", { 0.0104f, 0.0498f, 0.4980f, 0 }, 2e-3f },

			// Anisotropy: one shading model, ranked below coat and sheen.
			{ "graph anisotropy",    "float(material_model(pack_material_from_graph(0.0, 0.5, 0.0, 0.3, 0.6, 0.0, " + up + ", float3(1, 0, 0))))", { 3, 0, 0, 0 } },
			{ "sheen beats aniso",   "float(material_model(pack_material_from_graph(0.0, 0.5, 0.9, 0.3, 0.6, 0.0, " + up + ", float3(1, 0, 0))))", { 2, 0, 0, 0 } },
			// The tangent survives the encode/decode through the normal-only frame, up to
			// sign (a tangent and its negation are the same highlight)...
			{ "tangent round trip",  "abs(dot(aniso_decode_tangent(float3(0.3333333, 0.6666667, 0.6666667), aniso_encode_tangent(float3(0.3333333, 0.6666667, 0.6666667), normalize(cross(float3(0.3333333, 0.6666667, 0.6666667), float3(0, 0, 1))))), normalize(cross(float3(0.3333333, 0.6666667, 0.6666667), float3(0, 0, 1)))))", { 1, 0, 0, 0 }, 1e-3f },
			// ...and a quarter-turn rotation lands on the bitangent.
			{ "rotated tangent",     "abs(dot(aniso_decode_tangent(" + up + ", material_params(pack_material_from_graph(0.0, 0.0, 0.0, 0.0, 0.5, 0.25, " + up + ", float3(1, 0, 0))).y), cross(" + up + ", float3(1, 0, 0))))", { 1, 0, 0, 0 }, 1e-3f },
			// Mirror alignment (N = V = L): isotropic limit is 0.25 * F / a^2 = 4 for
			// roughness 0.5 (a = 0.25), F = 1; with anisotropy 0.5 the alphas are
			// 0.375 / 0.125, giving 0.25 / (0.375 * 0.125) = 5.3333.
			{ "aniso isotropic limit", "aniso_sun_specular(pack_material(SHADING_MODEL_ANISOTROPIC, float3(0.0, 0.0, 0.0)), " + up + ", " + up + ", " + up + ", 1.0, float3(1, 1, 1), 0.5)", { 4, 4, 4, 0 }, 1e-3f },
			{ "aniso stretched",     "aniso_sun_specular(pack_material(SHADING_MODEL_ANISOTROPIC, float3(0.5, 0.0, 0.0)), " + up + ", " + up + ", " + up + ", 1.0, float3(1, 1, 1), 0.5)", { 5.3333f, 5.3333f, 5.3333f, 0 }, 2e-3f },
			// With V up and T = +x, B = -z: a light tilted toward the tangent (wider alpha)
			// is closer to the highlight than one tilted the same angle toward the bitangent.
			{ "aniso is directional", "float(aniso_sun_specular(pack_material(SHADING_MODEL_ANISOTROPIC, float3(0.8, 0.0, 0.0)), " + up + ", " + up + ", normalize(float3(0.3, 1, 0)), 1.0, float3(1, 1, 1), 0.4).x > aniso_sun_specular(pack_material(SHADING_MODEL_ANISOTROPIC, float3(0.8, 0.0, 0.0)), " + up + ", " + up + ", normalize(float3(0, 1, -0.3)), 1.0, float3(1, 1, 1), 0.4).x)", { 1, 0, 0, 0 } },
			{ "aniso in shadow",     "aniso_sun_specular(pack_material(SHADING_MODEL_ANISOTROPIC, float3(0.5, 0.0, 0.0)), " + up + ", " + up + ", " + up + ", 0.0, float3(1, 1, 1), 0.5)", { 0, 0, 0, 0 } },
			{ "aniso on standard",   "aniso_sun_specular(pack_material(SHADING_MODEL_STANDARD, 0.0), " + up + ", " + up + ", " + up + ", 1.0, float3(1, 1, 1), 0.5)", { 0, 0, 0, 0 } },

			// The base layer sun highlight metals need (no diffuse, so otherwise no sun light).
			// Mirror alignment, F0 = 1, roughness 0.5 (a = 0.25): 0.25 * F / a^2 = 4, as the
			// anisotropic model's isotropic limit; scaled by metalness; not for anisotropic pixels.
			{ "metal sun highlight",  "base_sun_specular(pack_material(SHADING_MODEL_STANDARD, 0.0), " + up + ", " + up + ", " + up + ", 1.0, float3(1, 1, 1), 0.5, 1.0)", { 4, 4, 4, 0 }, 1e-3f },
			{ "half metal",           "base_sun_specular(pack_material(SHADING_MODEL_STANDARD, 0.0), " + up + ", " + up + ", " + up + ", 1.0, float3(1, 1, 1), 0.5, 0.5)", { 2, 2, 2, 0 }, 1e-3f },
			{ "dielectric no highlight", "base_sun_specular(pack_material(SHADING_MODEL_STANDARD, 0.0), " + up + ", " + up + ", " + up + ", 1.0, float3(0.04, 0.04, 0.04), 0.5, 0.0)", { 0, 0, 0, 0 } },
			{ "metal coloured by F0",  "base_sun_specular(pack_material(SHADING_MODEL_STANDARD, 0.0), " + up + ", " + up + ", " + up + ", 1.0, float3(1.0, 0.5, 0.25), 0.5, 1.0)", { 4, 2, 1, 0 }, 1e-3f },
			{ "metal in shadow",       "base_sun_specular(pack_material(SHADING_MODEL_STANDARD, 0.0), " + up + ", " + up + ", " + up + ", 0.0, float3(1, 1, 1), 0.5, 1.0)", { 0, 0, 0, 0 } },
			{ "metal light below horizon", "base_sun_specular(pack_material(SHADING_MODEL_STANDARD, 0.0), " + up + ", " + up + ", float3(0, -1, 0), 1.0, float3(1, 1, 1), 0.5, 1.0)", { 0, 0, 0, 0 } },
			{ "aniso keeps its own",    "base_sun_specular(pack_material(SHADING_MODEL_ANISOTROPIC, float3(0.5, 0.0, 0.0)), " + up + ", " + up + ", " + up + ", 1.0, float3(1, 1, 1), 0.5, 1.0)", { 0, 0, 0, 0 } },
			{ "lobes include the base highlight", "material_sun_lobes(pack_material(SHADING_MODEL_STANDARD, 0.0), " + up + ", " + up + ", " + up + ", 1.0, float3(1, 1, 1), 0.5, float3(1, 1, 1), 1.0)", { 4, 4, 4, 0 }, 1e-3f },

			// A sheen tinted like the base colour scales the lobe by the colour normalised to
			// its strongest channel; tint 0 leaves it white.
			{ "sheen tinted",       "sheen_sun_specular(pack_material(SHADING_MODEL_SHEEN, float3(1.0, 0.5, 1.0)), " + up + ", " + up + ", float3(0.6, 0.8, 0), 1.0, float3(1.0, 0.5, 0.25))", { 0.006f, 0.003f, 0.0015f, 0 }, 1e-5f },
			{ "sheen untinted",     "sheen_sun_specular(" + sheen + ", " + up + ", " + up + ", float3(0.6, 0.8, 0), 1.0, float3(1.0, 0.5, 0.25))", { 0.006f, 0.006f, 0.006f, 0 }, 1e-5f },
			{ "graph sheen tint",   "material_params(pack_material_from_graph(0.0, 0.5, 0.9, 0.3, 0.0, 0.0, 0.0, float3(1, 1, 1), 1.0, 0.7, " + up + ", float3(1, 0, 0))).z", { 0.7f, 0, 0, 0 }, 1e-4f },
			// Ambient sheen scales linearly with weight and with the ambient radiance, is
			// zero for other models, and stays a fraction of the incoming light (albedo < 1).
			{ "sheen ambient half weight", "sheen_indirect(pack_material(SHADING_MODEL_SHEEN, float3(0.5, 0.5, 0.0)), " + up + ", " + up + ", 1.0, 1.0) / sheen_indirect(" + sheen + ", " + up + ", " + up + ", 1.0, 1.0)", { 0.5f, 0.5f, 0.5f, 0 }, 1e-4f },
			{ "sheen ambient radiance",    "sheen_indirect(" + sheen + ", " + up + ", " + up + ", 1.0, 3.0) / sheen_indirect(" + sheen + ", " + up + ", " + up + ", 1.0, 1.0)", { 3, 3, 3, 0 }, 1e-4f },
			{ "sheen ambient other model", "sheen_indirect(" + coated + ", " + up + ", " + up + ", 1.0, 1.0)", { 0, 0, 0, 0 } },
			{ "sheen ambient bounded",     "float3(sheen_directional_albedo(1.0, 0.25) > 0 && sheen_directional_albedo(1.0, 0.25) < 1, sheen_directional_albedo(0.2, 0.25) > 0 && sheen_directional_albedo(0.2, 0.25) < 1, 0)", { 1, 1, 0, 0 } },
			// The coat takes F of the light before the base layer sees it: 1 - weight * F.
			{ "coat attenuation head-on", "coat_base_attenuation(" + coated + ", " + up + ", " + up + ")", { 0.96f, 0, 0, 0 }, 1e-4f },
			{ "coat attenuation grazing", "coat_base_attenuation(" + coated + ", " + up + ", float3(1, 0, 0))", { 0.0f, 0, 0, 0 }, 1e-4f },
			{ "coat attenuation half",    "coat_base_attenuation(pack_material(SHADING_MODEL_CLEARCOAT, float3(0.5, 0.1, 0.0)), " + up + ", " + up + ")", { 0.98f, 0, 0, 0 }, 1e-4f },
			{ "no attenuation standard",  "coat_base_attenuation(pack_material(SHADING_MODEL_STANDARD, 0.0), " + up + ", " + up + ")", { 1, 0, 0, 0 } },
			{ "no attenuation sheen",     "coat_base_attenuation(" + sheen + ", " + up + ", " + up + ")", { 1, 0, 0, 0 } },

			// Sheen, N = up. Head-on view, light at (0.6, 0.8, 0): NH^2 = 0.9 so
			// sin^2 = 0.1; a = 0.25 -> D = 6 * 0.1^2 / (2 pi), visibility 1/4,
			// NL = 0.8, scaled by pi: exactly 0.006.
			{ "sheen head-on",      "sheen_sun_specular(" + sheen + ", " + up + ", " + up + ", float3(0.6, 0.8, 0), 1.0)", { 0.006f, 0.006f, 0.006f, 0 }, 1e-5f },
			// V = (0.8, 0.6, 0), L = (0.6, 0.8, 0): NV 0.6, NL 0.8, NH^2 = 0.5.
			{ "sheen oblique",      "sheen_sun_specular(" + sheen + ", " + up + ", float3(0.8, 0.6, 0), float3(0.6, 0.8, 0), 1.0)", { 0.16304f, 0.16304f, 0.16304f, 0 }, 2e-4f },
			{ "sheen half weight",  "sheen_sun_specular(pack_material(SHADING_MODEL_SHEEN, float3(0.5, 0.5, 0.0)), " + up + ", " + up + ", float3(0.6, 0.8, 0), 1.0)", { 0.003f, 0.003f, 0.003f, 0 }, 1e-5f },
			{ "sheen in shadow",    "sheen_sun_specular(" + sheen + ", " + up + ", " + up + ", float3(0.6, 0.8, 0), 0.0)", { 0, 0, 0, 0 } },
			{ "sheen on coat",      "sheen_sun_specular(" + coated + ", " + up + ", " + up + ", float3(0.6, 0.8, 0), 1.0)", { 0, 0, 0, 0 } },
			{ "coat on sheen",      "coat_sun_specular(" + sheen + ", " + up + ", " + up + ", " + up + ", 1.0)", { 0, 0, 0, 0 } },
			{ "lobes: sheen",       "material_sun_lobes(" + sheen + ", " + up + ", " + up + ", float3(0.6, 0.8, 0), 1.0, float3(1, 1, 1), 0.5)", { 0.006f, 0.006f, 0.006f, 0 }, 1e-5f },
			{ "lobes: coat",        "material_sun_lobes(" + coated + ", " + up + ", " + up + ", " + up + ", 1.0, float3(1, 1, 1), 0.5)", { 100, 100, 100, 0 }, 0.5f },
			{ "lobes: standard",    "material_sun_lobes(pack_material(SHADING_MODEL_STANDARD, 0.0), " + up + ", " + up + ", " + up + ", 1.0, float3(1, 1, 1), 0.5)", { 0, 0, 0, 0 } },
		}, lobes);
	}

	// The subsurface blur passes (sss.prism) on a synthetic G-buffer: the lit
	// target is blurred along x then y for SUBSURFACE pixels only. A uniform
	// region must come back unchanged (the kernel is normalised), a step must
	// smooth into a monotonic ramp that stays within the input range, and
	// other pixels must not be touched at all.
	TEST(MaterialX.GPU, SubsurfaceBlur)
	{
		auto& device = RenderSystem::get().device();
		constexpr uint W = 128, H = 128;
		const auto rw = HAL::ResFlags::ShaderResource | HAL::ResFlags::UnorderedAccess;

		auto make = [&](HAL::Format format, HAL::ResFlags flags)
		{
			return std::make_shared<HAL::TextureResource>(device,
				HAL::ResourceDesc::Tex2D(format, { W, H }, 1, 1, flags), HAL::HeapType::DEFAULT);
		};

		auto albedo_tex   = make(HAL::Format::R8G8B8A8_UNORM, HAL::ResFlags::ShaderResource);
		auto normals_tex  = make(HAL::Format::R8G8B8A8_UNORM, HAL::ResFlags::ShaderResource);
		auto material_tex = make(HAL::Format::R8G8B8A8_UNORM, HAL::ResFlags::ShaderResource);
		auto depth_tex    = make(HAL::Format::R32_FLOAT, HAL::ResFlags::ShaderResource);
		auto result_tex   = make(HAL::Format::R32G32B32A32_FLOAT, rw);
		auto temp_tex     = make(HAL::Format::R32G32B32A32_FLOAT, rw);

		// Rows 0-7: a step (lit on the left half). Rows 70-77: uniformly lit. Rows
		// 20-29 and 100-109: not subsurface, with a marker value that must
		// survive. The groups are further apart than the widest kernel reaches
		// (48 px), so they cannot blur into each other vertically; taps are 3 px
		// apart, so a thin separator row would simply be hopped over.
		std::vector<uint8_t> albedo(W * H * 4), normals(W * H * 4), material(W * H * 4);
		std::vector<float> depth(W * H, 0.5f), result(W * H * 4);

		for (uint y = 0; y < H; y++)
			for (uint x = 0; x < W; x++)
			{
				const size_t i = (size_t)y * W + x;
				const bool step_rows = y < 8;
				const bool sss = step_rows || (y >= 70 && y < 78);
				const float lit = step_rows ? (x < W / 2 ? 1.0f : 0.0f) : 1.0f;

				albedo[i * 4 + 0] = albedo[i * 4 + 1] = albedo[i * 4 + 2] = 128;
				albedo[i * 4 + 3] = 0;
				normals[i * 4 + 0] = normals[i * 4 + 1] = 128;
				normals[i * 4 + 2] = 255;
				normals[i * 4 + 3] = 128;

				// SUBSURFACE = 4 in alpha, radius at the encoding's maximum.
				material[i * 4 + 0] = material[i * 4 + 1] = material[i * 4 + 2] = sss ? 255 : 0;
				material[i * 4 + 3] = sss ? 4 : 0;

				const float v = sss ? lit : 7.0f;
				result[i * 4 + 0] = result[i * 4 + 1] = result[i * 4 + 2] = v;
				result[i * 4 + 3] = 1.0f;
			}

		{
			auto upload = device.get_upload_list();
			auto put = [&](HAL::TextureResource* tex, const void* src, uint bytes_per_pixel)
			{
				upload->get_copy().update_texture(tex, ivec3(0, 0, 0), ivec3(W, H, 1), 0,
					reinterpret_cast<const char*>(src), W * bytes_per_pixel);
			};
			put(albedo_tex.get(),   albedo.data(),   4);
			put(normals_tex.get(),  normals.data(),  4);
			put(material_tex.get(), material.data(), 4);
			put(depth_tex.get(),    depth.data(),    4);
			put(result_tex.get(),   result.data(),   16);
			upload->execute_and_wait();
		}

		camera cam;
		cam.set_projection_params(Math::pi / 4, float(W) / H, 0.1f, 100.0f);
		cam.update();

		auto& queue = device.get_queue(HAL::CommandListType::DIRECT);

		// The blur is dispatched indirectly over the 16x16 tiles SSSClassify lists,
		// with the list's GPU counter copied into the dispatch arguments, as the
		// frame graph passes do (sss.prism).
		HAL::StructuredBufferView<uint2> tile_list(device, (W / 16) * (H / 16), HAL::counterType::SELF,
			HAL::ResFlags::ShaderResource | HAL::ResFlags::UnorderedAccess);
		HAL::StructuredBufferView<DispatchArguments> tile_args(device, 1);
		{
			DispatchArguments init{ 0, 1, 1 };
			auto upload = device.get_upload_list();
			upload->get_copy().update(tile_args, 0, std::span{ &init, 1 });
			upload->execute_and_wait();
		}

		auto bind_gbuffer = [&](Slots::Subsurface::SSSBlurParams& params, HAL::Texture2DView& albedo_view, HAL::Texture2DView& normals_view,
			HAL::Texture2DView& material_view, HAL::Texture2DView& depth_view)
		{
			params.GetGbuffer().GetAlbedo()   = albedo_view.texture2D;
			params.GetGbuffer().GetNormals()  = normals_view.texture2D;
			params.GetGbuffer().GetMaterial() = material_view.texture2D;
			// The table has no [Auto] for these; the blur never samples them.
			params.GetGbuffer().GetSpecular() = albedo_view.texture2D;
			params.GetGbuffer().GetMotion()   = albedo_view.texture2D;
			params.GetGbuffer().GetDepth()    = depth_view.texture2D;
		};

		// read_counter delivers its value from another thread, possibly after
		// execute_and_wait returns, so it is waited for explicitly.
		std::atomic<int> tile_count{ -1 };

		{
			auto list = queue->get_free_list();
			list->begin(L"SubsurfaceClassify");

			HAL::Texture2DView albedo_view(albedo_tex, *list);
			HAL::Texture2DView normals_view(normals_tex, *list);
			HAL::Texture2DView material_view(material_tex, *list);
			HAL::Texture2DView depth_view(depth_tex, *list);

			auto& compute = list->get_compute();
			compute.set_signature(Layouts::DefaultLayout);

			Slots::Subsurface::SSSClassifyParams params;
			params.GetGbuffer().GetAlbedo()   = albedo_view.texture2D;
			params.GetGbuffer().GetNormals()  = normals_view.texture2D;
			params.GetGbuffer().GetMaterial() = material_view.texture2D;
			params.GetGbuffer().GetSpecular() = albedo_view.texture2D;
			params.GetGbuffer().GetMotion()   = albedo_view.texture2D;
			params.GetGbuffer().GetDepth()    = depth_view.texture2D;
			compute.set(params);

			Slots::Subsurface::SSSTilesAppend tiles;
			tiles.GetTiles() = tile_list.appendStructuredBuffer;
			compute.set(tiles);

			compute.clear_counter(tile_list);
			compute.set_pipeline<PSOS::Subsurface::SSSClassify>();
			compute.dispatch(ivec2{ int(W), int(H) }, ivec2{ 16, 16 });
			list->get_copy().copy_buffer(tile_args.resource.get(), 0, tile_list.get_counter_buffer().get(), tile_list.get_counter_offset(), 4);
			list->get_copy().read_counter(tile_list, [&](uint count) { tile_count = int(count); });
			list->execute_and_wait();
		}

		// Only tiles holding subsurface pixels are listed: rows 0-7 touch tile row
		// 0 and rows 70-77 touch tile row 4, eight tile columns each.
		for (int waited = 0; tile_count.load() < 0 && waited < 500; waited++)
			std::this_thread::sleep_for(std::chrono::milliseconds(10));

		ASSERT_EQ(16, tile_count.load());

		// One command list per pass: the second reads what the first wrote.
		auto run = [&](bool vertical, std::shared_ptr<HAL::TextureResource> source_tex, std::shared_ptr<HAL::TextureResource> target_tex)
		{
			auto list = queue->get_free_list();
			list->begin(L"SubsurfaceBlur");

			HAL::Texture2DView albedo_view(albedo_tex, *list);
			HAL::Texture2DView normals_view(normals_tex, *list);
			HAL::Texture2DView material_view(material_tex, *list);
			HAL::Texture2DView depth_view(depth_tex, *list);
			HAL::Texture2DView source_view(source_tex, *list);
			HAL::Texture2DView target_view(target_tex, *list);

			auto& compute = list->get_compute();
			compute.set_signature(Layouts::DefaultLayout);

			{
				Slots::Frame::FrameInfo frameInfo;
				frameInfo.GetCamera() = cam.camera_cb.current;
				frameInfo.GetBrdf()           = EngineAssets::brdf.get_asset()->get_texture()->texture_3d().texture3D;
				frameInfo.GetBestFitNormals() = EngineAssets::best_fit_normals.get_asset()->get_texture()->texture_2d().texture2D;
				compute.set(frameInfo);
			}

			if (vertical)
				compute.set_pipeline<PSOS::Subsurface::SSSBlurV>();
			else
				compute.set_pipeline<PSOS::Subsurface::SSSBlurH>();

			Slots::Subsurface::SSSBlurParams params;
			bind_gbuffer(params, albedo_view, normals_view, material_view, depth_view);
			params.GetSource() = source_view.texture2D;
			params.GetTarget() = target_view.rwTexture2D;
			compute.set(params);

			Slots::Subsurface::SSSTilesRead tiles;
			tiles.GetTiles() = tile_list.structuredBuffer;
			compute.set(tiles);

			compute.exec_indirect(tile_args, 1);
			list->execute_and_wait();
		};

		run(false, result_tex, temp_tex);
		run(true, temp_tex, result_tex);

		auto data = readback_texture(result_tex.get());
		auto& mip = data->array[0]->mips[0];
		auto at = [&](uint x, uint y) { return reinterpret_cast<const float*>(mip->data.data() + (size_t)y * mip->width_stride)[x * 4]; };

		// Uniform rows come back as they went in.
		std::string failures;
		for (uint x = 0; x < W; x++)
			if (std::abs(at(x, 72) - 1.0f) > 1e-3f)
				failures += "uniform row x=" + std::to_string(x) + " is " + std::to_string(at(x, 72)) + "\n";

		// The step: bounded, monotonic, genuinely smoothed, and symmetric about the edge.
		for (uint x = 0; x < W; x++)
		{
			const float v = at(x, 4);

			if (v < -1e-3f || v > 1.0f + 1e-3f)
				failures += "step out of range at x=" + std::to_string(x) + ": " + std::to_string(v) + "\n";

			// Taps sit 3 px apart, so a hard edge leaves ~1% ripple; more than that is a bug.
			if (x > 0 && v > at(x - 1, 4) + 0.02f)
				failures += "step not monotonic at x=" + std::to_string(x) + "\n";
		}

		if (!(at(W / 2 - 1, 4) < 0.99f && at(W / 2, 4) > 0.01f))
			failures += "step was not blurred: " + std::to_string(at(W / 2 - 1, 4)) + " / " + std::to_string(at(W / 2, 4)) + "\n";

		if (std::abs(at(W / 2 - 1, 4) + at(W / 2, 4) - 1.0f) > 0.08f)
			failures += "step not symmetric about the edge: " + std::to_string(at(W / 2 - 1, 4)) + " + " + std::to_string(at(W / 2, 4)) + "\n";

		// Non-subsurface pixels are left alone.
		for (uint y : { 20u, 25u, 29u, 100u, 105u, 109u })
			for (uint x = 0; x < W; x++)
				if (at(x, y) != 7.0f)
					failures += "non-subsurface pixel changed at " + std::to_string(x) + "," + std::to_string(y) + "\n";

		ASSERT_EQ(std::string(), failures);
	}

	// The importer reads a .mtlx file and registers a material asset for it.
	TEST(MaterialX.GPU, SampleFile)
	{
		mx_detail::ensure_library();

		auto sample = to_path(L"materialx/samples/checker_standard_surface.mtlx");
		ASSERT_EQ(size_t(1), MaterialXLibrary::get().import_file(sample));
	}

	namespace mx_import
	{
		using Graphs = std::vector<std::pair<std::string, MaterialGraph::ptr>>;

		// Writes the document to a scratch file (textures are named by absolute
		// path, so the location doesn't matter) and builds its graphs.
		inline Graphs build(const std::string& body)
		{
			mx_detail::ensure_library();

			static int counter = 0;
			auto dir = std::filesystem::temp_directory_path() / "spectrum_mx_tests";
			std::filesystem::create_directories(dir);
			auto file = dir / ("case_" + std::to_string(counter++) + ".mtlx");

			{
				std::ofstream out(file, std::ios::binary);
				out << "<?xml version=\"1.0\"?>\n<materialx version=\"1.39\">\n" << body << "\n</materialx>\n";
			}

			return MaterialXLibrary::get().build_graphs(file);
		}

		// A surface shader node plus the material that uses it.
		inline std::string surface(const std::string& category, const std::string& inputs, const std::string& extra_nodes = "")
		{
			return extra_nodes +
				"<" + category + " name=\"shader\" type=\"surfaceshader\">\n" + inputs + "\n</" + category + ">\n"
				"<surfacematerial name=\"mat\" type=\"material\"><input name=\"surfaceshader\" type=\"surfaceshader\" nodename=\"shader\" /></surfacematerial>";
		}

		inline ::FlowGraph::Node* source(const ::FlowGraph::parameter::ptr& port)
		{
			if (!port || port->input_connections.empty())
				return nullptr;

			return (*port->input_connections.begin())->from->owner;
		}

		template<class T>
		T* source_as(const ::FlowGraph::parameter::ptr& port)
		{
			return dynamic_cast<T*>(source(port));
		}

		inline std::string close(const std::vector<float>& got, const std::vector<float>& want)
		{
			if (got.size() != want.size())
				return "size " + std::to_string(got.size()) + " != " + std::to_string(want.size());

			for (size_t i = 0; i < got.size(); i++)
				if (std::abs(got[i] - want[i]) > 1e-5f)
					return "component " + std::to_string(i) + ": " + std::to_string(got[i]) + " != " + std::to_string(want[i]);

			return {};
		}

		inline std::vector<float> value_of(VectorNode* node) { auto v = node->get_value(); return { v.x, v.y, v.z, v.w }; }

		// Constant-only inputs: the graph outputs come out as the folded values.
		inline void check_constants(const Graphs& graphs, std::vector<float> base_color, float metallic, float roughness)
		{
			ASSERT_EQ(size_t(1), graphs.size());
			auto& g = graphs[0].second;

			auto* base = source_as<VectorNode>(g->get_base_color());
			auto* met  = source_as<ScalarNode>(g->get_mettalic());
			auto* rough = source_as<ScalarNode>(g->get_roughness());
			ASSERT_TRUE(base && met && rough);

			ASSERT_EQ(std::string(), close(value_of(base), base_color));
			ASSERT_EQ(std::string(), close({ met->get_value() }, { metallic }));
			ASSERT_EQ(std::string(), close({ rough->get_value() }, { roughness }));
		}
	}

	TEST(MaterialX.GPU, StandardSurfaceConstants)
	{
		using namespace mx_import;

		auto graphs = build(surface("standard_surface",
			R"(<input name="base" type="float" value="0.5" />
			   <input name="base_color" type="color3" value="0.2, 0.4, 0.6" />
			   <input name="metalness" type="float" value="0.7" />
			   <input name="specular_roughness" type="float" value="0.35" />)"));

		// base * base_color is folded into one constant, alpha 1.
		check_constants(graphs, { 0.1f, 0.2f, 0.3f, 1.0f }, 0.7f, 0.35f);
		ASSERT_EQ(std::string("mat"), graphs[0].first);

		// A default-valued emission, opacity and normal leave their outputs unwired.
		auto& g = graphs[0].second;
		ASSERT_TRUE(source(g->get_glow()) == nullptr);
		ASSERT_TRUE(source(g->get_opacity()) == nullptr);
		ASSERT_TRUE(source(g->get_normals()) == nullptr);
	}

	// Parameters the file leaves out take the shader's own defaults, which
	// standard_surface inherits from a versioned parent nodedef.
	TEST(MaterialX.GPU, ShaderDefaults)
	{
		using namespace mx_import;

		check_constants(build(surface("standard_surface", "")), { 0.8f, 0.8f, 0.8f, 1.0f }, 0.0f, 0.2f);
	}

	TEST(MaterialX.GPU, EmissionAndOpacity)
	{
		using namespace mx_import;

		auto graphs = build(surface("standard_surface",
			R"(<input name="emission" type="float" value="2.0" />
			   <input name="emission_color" type="color3" value="1.0, 0.5, 0.0" />
			   <input name="opacity" type="color3" value="0.5, 0.5, 0.5" />)"));

		ASSERT_EQ(size_t(1), graphs.size());
		auto& g = graphs[0].second;

		auto* glow = source_as<VectorNode>(g->get_glow());
		auto* opacity = source_as<ScalarNode>(g->get_opacity());
		ASSERT_TRUE(glow && opacity);

		ASSERT_EQ(std::string(), close(value_of(glow), { 2.0f, 1.0f, 0.0f, 1.0f }));
		ASSERT_EQ(std::string(), close({ opacity->get_value() }, { 0.5f }));
	}

	// Coat and sheen map to the graph's lobe outputs; a material gets one shading
	// model, so the coat wins when both are set.
	TEST(MaterialX.GPU, CoatAndSheen)
	{
		using namespace mx_import;

		auto coated = build(surface("standard_surface",
			R"(<input name="coat" type="float" value="0.7" />
			   <input name="coat_roughness" type="float" value="0.15" />)"));
		ASSERT_EQ(size_t(1), coated.size());
		auto& c = coated[0].second;
		ASSERT_EQ(std::string(), close({ source_as<ScalarNode>(c->get_coat())->get_value() }, { 0.7f }));
		ASSERT_EQ(std::string(), close({ source_as<ScalarNode>(c->get_coat_roughness())->get_value() }, { 0.15f }));
		ASSERT_TRUE(source(c->get_sheen()) == nullptr);

		auto cloth = build(surface("standard_surface",
			R"(<input name="sheen" type="float" value="0.9" />
			   <input name="sheen_roughness" type="float" value="0.35" />)"));
		ASSERT_EQ(size_t(1), cloth.size());
		auto& s = cloth[0].second;
		ASSERT_EQ(std::string(), close({ source_as<ScalarNode>(s->get_sheen())->get_value() }, { 0.9f }));
		ASSERT_EQ(std::string(), close({ source_as<ScalarNode>(s->get_sheen_roughness())->get_value() }, { 0.35f }));
		ASSERT_TRUE(source(s->get_coat()) == nullptr);

		// gltf_pbr's sheen is a colour: its strongest component becomes the weight.
		auto gltf = build(surface("gltf_pbr",
			R"(<input name="sheen_color" type="color3" value="0.2, 0.5, 0.1" />
			   <input name="sheen_roughness" type="float" value="0.6" />)"));
		ASSERT_EQ(size_t(1), gltf.size());
		ASSERT_EQ(std::string(), close({ source_as<ScalarNode>(gltf[0].second->get_sheen())->get_value() }, { 0.5f }));

		// The sheen colour's saturation becomes the tint: white is untinted,
		// a saturated colour follows the base colour. (0.5 - 0.1) / 0.5 = 0.8.
		ASSERT_EQ(std::string(), close({ source_as<ScalarNode>(gltf[0].second->get_sheen_tint())->get_value() }, { 0.8f }));

		auto white_sheen = build(surface("standard_surface",
			R"(<input name="sheen" type="float" value="1.0" />
			   <input name="sheen_color" type="color3" value="0.9, 0.9, 0.9" />)"));
		ASSERT_EQ(size_t(1), white_sheen.size());
		ASSERT_EQ(std::string(), close({ source_as<ScalarNode>(white_sheen[0].second->get_sheen_tint())->get_value() }, { 0.0f }));

		auto both = build(surface("standard_surface",
			R"(<input name="coat" type="float" value="1.0" />
			   <input name="sheen" type="float" value="1.0" />)"));
		ASSERT_EQ(size_t(1), both.size());
		ASSERT_TRUE(source(both[0].second->get_coat()) != nullptr);
		ASSERT_TRUE(source(both[0].second->get_sheen()) == nullptr);
	}

	TEST(MaterialX.GPU, Subsurface)
	{
		using namespace mx_import;

		auto skin = build(surface("standard_surface",
			R"(<input name="subsurface" type="float" value="1.0" />
			   <input name="subsurface_radius" type="color3" value="1.0, 0.35, 0.2" />
			   <input name="subsurface_scale" type="float" value="0.05" />)"));
		ASSERT_EQ(size_t(1), skin.size());
		auto& g = skin[0].second;
		ASSERT_EQ(std::string(), close({ source_as<ScalarNode>(g->get_subsurface_weight())->get_value() }, { 1.0f }));
		ASSERT_EQ(std::string(), close({ source_as<ScalarNode>(g->get_subsurface_scale())->get_value() }, { 0.05f }));
		auto v = source_as<VectorNode>(g->get_subsurface_radius())->get_value();
		ASSERT_EQ(std::string(), close({ v.x, v.y, v.z }, { 1.0f, 0.35f, 0.2f }));

		// Lowest priority of the shading models.
		auto anisotropic = build(surface("standard_surface",
			R"(<input name="specular_anisotropy" type="float" value="0.7" />
			   <input name="subsurface" type="float" value="1.0" />)"));
		ASSERT_EQ(size_t(1), anisotropic.size());
		ASSERT_TRUE(source(anisotropic[0].second->get_anisotropy()) != nullptr);
		ASSERT_TRUE(source(anisotropic[0].second->get_subsurface_weight()) == nullptr);

		// No subsurface weight, nothing wired.
		auto plain = build(surface("standard_surface", ""));
		ASSERT_EQ(size_t(1), plain.size());
		ASSERT_TRUE(source(plain[0].second->get_subsurface_weight()) == nullptr);
	}

	// GBufferDownsampler's roughness verdict decides which tiles trace their own
	// full-res reflection ray instead of copying the (isotropic) half-res one. A tile
	// holding an anisotropic pixel has to trace regardless of roughness and
	// metalness; the existing "glossy and metallic" rule still applies.
	TEST(MaterialX.GPU, TileClassifyAnisotropic)
	{
		auto& device = RenderSystem::get().device();
		constexpr uint W = 64, H = 64, T = W / 8;
		const auto rw = HAL::ResFlags::ShaderResource | HAL::ResFlags::UnorderedAccess;

		auto make = [&](HAL::Format format, uint w, uint h, HAL::ResFlags flags)
		{
			return std::make_shared<HAL::TextureResource>(device,
				HAL::ResourceDesc::Tex2D(format, { w, h }, 1, 1, flags), HAL::HeapType::DEFAULT);
		};

		auto albedo_tex   = make(HAL::Format::R8G8B8A8_UNORM, W, H, HAL::ResFlags::ShaderResource);
		auto normals_tex  = make(HAL::Format::R8G8B8A8_UNORM, W, H, HAL::ResFlags::ShaderResource);
		auto material_tex = make(HAL::Format::R8G8B8A8_UNORM, W, H, HAL::ResFlags::ShaderResource);
		auto depth_tex    = make(HAL::Format::R32_FLOAT, W, H, HAL::ResFlags::ShaderResource);
		auto half_depth   = make(HAL::Format::R32_FLOAT, W / 2, H / 2, rw);
		auto half_normals = make(HAL::Format::R8G8B8A8_UNORM, W / 2, H / 2, rw);
		auto tile_mask    = make(HAL::Format::R8_UINT, W, H, rw);
		auto tile_flags   = make(HAL::Format::R8_UINT, T, T, rw);
		auto rough_flags  = make(HAL::Format::R8_UINT, T, T, rw);

		// A flat, rough, dielectric scene: nothing qualifies by itself.
		std::vector<uint8_t> albedo(W * H * 4, 0), normals(W * H * 4, 0), material(W * H * 4, 0);
		std::vector<float> depth(W * H, 0.5f);

		for (uint i = 0; i < W * H; i++)
		{
			normals[i * 4 + 0] = normals[i * 4 + 1] = 128;
			normals[i * 4 + 2] = 255;
			normals[i * 4 + 3] = 230;   // roughness 0.9: above the 0.5 threshold
		}

		auto pixel = [&](uint x, uint y) { return (size_t)(y * W + x) * 4; };

		// Tile (2, 3): one anisotropic pixel, nothing else special.
		material[pixel(20, 25) + 3] = 3;
		// Tile (5, 5): glossy and metallic -- the pre-existing rule, as a control.
		for (uint y = 40; y < 48; y++)
			for (uint x = 40; x < 48; x++)
			{
				albedo[pixel(x, y) + 3]  = 255;
				normals[pixel(x, y) + 3] = 25;   // roughness 0.1
			}
		// Tile (6, 1): metallic but rough, and tile (1, 6): glossy but dielectric --
		// each satisfies only half of the rule, so neither qualifies.
		for (uint y = 8; y < 16; y++)
			for (uint x = 48; x < 56; x++)
				albedo[pixel(x, y) + 3] = 255;
		for (uint y = 48; y < 56; y++)
			for (uint x = 8; x < 16; x++)
				normals[pixel(x, y) + 3] = 25;

		{
			auto upload = device.get_upload_list();
			auto put = [&](HAL::TextureResource* tex, const void* src, uint w, uint h, uint bytes_per_pixel)
			{
				upload->get_copy().update_texture(tex, ivec3(0, 0, 0), ivec3(w, h, 1), 0,
					reinterpret_cast<const char*>(src), w * bytes_per_pixel);
			};
			put(albedo_tex.get(),   albedo.data(),   W, H, 4);
			put(normals_tex.get(),  normals.data(),  W, H, 4);
			put(material_tex.get(), material.data(), W, H, 4);
			put(depth_tex.get(),    depth.data(),    W, H, 4);
			upload->execute_and_wait();
		}

		const auto counted = HAL::ResFlags::ShaderResource | HAL::ResFlags::UnorderedAccess;
		const uint tiles = T * T;
		HAL::StructuredBufferView<uint2> hi(device, tiles, HAL::counterType::SELF, counted);
		HAL::StructuredBufferView<uint2> low(device, tiles, HAL::counterType::SELF, counted);
		HAL::StructuredBufferView<uint2> rough_hi(device, tiles, HAL::counterType::SELF, counted);
		HAL::StructuredBufferView<uint2> rough_low(device, tiles, HAL::counterType::SELF, counted);

		{
			auto& queue = device.get_queue(HAL::CommandListType::DIRECT);
			auto  list  = queue->get_free_list();
			list->begin(L"TileClassify");

			HAL::Texture2DView albedo_view(albedo_tex, *list);
			HAL::Texture2DView normals_view(normals_tex, *list);
			HAL::Texture2DView material_view(material_tex, *list);
			HAL::Texture2DView depth_view(depth_tex, *list);
			HAL::Texture2DView half_depth_view(half_depth, *list);
			HAL::Texture2DView half_normals_view(half_normals, *list);
			HAL::Texture2DView mask_view(tile_mask, *list);
			HAL::Texture2DView flags_view(tile_flags, *list);
			HAL::Texture2DView rough_flags_view(rough_flags, *list);

			auto& compute = list->get_compute();
			compute.set_signature(Layouts::DefaultLayout);

			Slots::Meshes::TileClassifyData params;
			params.GetGbuffer().GetAlbedo()   = albedo_view.texture2D;
			params.GetGbuffer().GetNormals()  = normals_view.texture2D;
			params.GetGbuffer().GetMaterial() = material_view.texture2D;
			params.GetGbuffer().GetSpecular() = albedo_view.texture2D;
			params.GetGbuffer().GetMotion()   = albedo_view.texture2D;
			params.GetGbuffer().GetDepth()    = depth_view.texture2D;
			params.GetHalf_depth()   = half_depth_view.rwTexture2D;
			params.GetHalf_normals() = half_normals_view.rwTexture2D;
			params.GetTile_hi()      = hi.appendStructuredBuffer;
			params.GetTile_low()     = low.appendStructuredBuffer;
			params.GetTile_mask()    = mask_view.rwTexture2D;
			params.GetTile_flags()   = flags_view.rwTexture2D;
			params.GetTile_roughness_hi()    = rough_hi.appendStructuredBuffer;
			params.GetTile_roughness_low()   = rough_low.appendStructuredBuffer;
			params.GetTile_roughness_flags() = rough_flags_view.rwTexture2D;
			params.GetRoughness_threshold()  = 0.5f;
			params.GetMetallic_threshold()   = 0.05f;
			compute.set(params);

			compute.clear_counter(hi);
			compute.clear_counter(low);
			compute.clear_counter(rough_hi);
			compute.clear_counter(rough_low);
			compute.set_pipeline<PSOS::Meshes::GBufferDownsample>();
			compute.dispatch(ivec2{ int(W), int(H) }, ivec2{ 8, 8 });
			list->execute_and_wait();
		}

		auto data = readback_texture(rough_flags.get());
		auto& mip = data->array[0]->mips[0];
		auto flag = [&](uint x, uint y) { return int(mip->data[(size_t)y * mip->width_stride + x]); };

		std::string failures;

		for (uint y = 0; y < T; y++)
			for (uint x = 0; x < T; x++)
			{
				const int expected = (x == 2 && y == 3) || (x == 5 && y == 5) ? 1 : 0;

				if (flag(x, y) != expected)
					failures += "tile (" + std::to_string(x) + "," + std::to_string(y) + ") is " + std::to_string(flag(x, y))
						+ ", expected " + std::to_string(expected) + "\n";
			}

		ASSERT_EQ(std::string(), failures);
	}

	// subsurface_color is the diffuse colour of the scattering part: the base
	// colour blends towards it by the subsurface weight.
	TEST(MaterialX.GPU, SubsurfaceColor)
	{
		using namespace mx_import;

		auto graphs = build(surface("standard_surface",
			R"(<input name="base_color" type="color3" value="0.8, 0.5, 0.4" />
			   <input name="subsurface" type="float" value="0.5" />
			   <input name="subsurface_color" type="color3" value="0.2, 0.1, 0.0" />)"));
		ASSERT_EQ(size_t(1), graphs.size());
		ASSERT_EQ(std::string(), close(value_of(source_as<VectorNode>(graphs[0].second->get_base_color())), { 0.5f, 0.3f, 0.2f, 1.0f }));

		// Left alone when another shading model took the slot (subsurface is dropped).
		auto coated = build(surface("standard_surface",
			R"(<input name="base_color" type="color3" value="0.8, 0.5, 0.4" />
			   <input name="coat" type="float" value="1.0" />
			   <input name="subsurface" type="float" value="1.0" />
			   <input name="subsurface_color" type="color3" value="0.2, 0.1, 0.0" />)"));
		ASSERT_EQ(size_t(1), coated.size());
		ASSERT_EQ(std::string(), close(value_of(source_as<VectorNode>(coated[0].second->get_base_color())), { 0.8f, 0.5f, 0.4f, 1.0f }));
	}

	// triplanarprojection: three image files become three texture inputs, and its
	// upaxis is an editable int constant.
	TEST(MaterialX.GPU, TriplanarImport)
	{
		using namespace mx_import;

		std::string icon = std::filesystem::absolute(to_path(L"resources/icon.jpg")).generic_string();
		auto file = [&](const char* name) { return std::string("<input name=\"") + name + "\" type=\"filename\" value=\"" + icon + "\" />"; };

		auto graphs = build(surface("standard_surface",
			R"(<input name="base_color" type="color3" nodename="tri" />)",
			"<triplanarprojection name=\"tri\" type=\"color3\">" + file("filex") + file("filey") + file("filez") +
			"<input name=\"upaxis\" type=\"integer\" value=\"1\" /><input name=\"blend\" type=\"float\" value=\"0.5\" /></triplanarprojection>"));

		ASSERT_EQ(size_t(1), graphs.size());
		auto* tri = source_as<MaterialXNode>(graphs[0].second->get_base_color());
		ASSERT_TRUE(tri != nullptr);
		ASSERT_EQ(std::string("ND_triplanarprojection_color3"), tri->get_nodedef());

		for (const char* name : { "filex", "filey", "filez" })
			ASSERT_TRUE(source_as<TextureAssetNode>(tri->get_input_by_name(name)) != nullptr);

		ASSERT_EQ(std::string(), close(tri->get_constant("upaxis"), { 1.0f }));
		ASSERT_EQ(std::string(), close(tri->get_constant("blend"), { 0.5f }));
	}

	TEST(MaterialX.GPU, Anisotropy)
	{
		using namespace mx_import;

		auto brushed = build(surface("standard_surface",
			R"(<input name="specular_anisotropy" type="float" value="0.8" />
			   <input name="specular_rotation" type="float" value="0.25" />)"));
		ASSERT_EQ(size_t(1), brushed.size());
		auto& g = brushed[0].second;
		ASSERT_EQ(std::string(), close({ source_as<ScalarNode>(g->get_anisotropy())->get_value() }, { 0.8f }));
		ASSERT_EQ(std::string(), close({ source_as<ScalarNode>(g->get_anisotropy_rotation())->get_value() }, { 0.25f }));

		// Lowest priority: a coat or sheen takes the one shading-model slot.
		auto coated = build(surface("standard_surface",
			R"(<input name="coat" type="float" value="1.0" />
			   <input name="specular_anisotropy" type="float" value="0.8" />)"));
		ASSERT_EQ(size_t(1), coated.size());
		ASSERT_TRUE(source(coated[0].second->get_coat()) != nullptr);
		ASSERT_TRUE(source(coated[0].second->get_anisotropy()) == nullptr);

		auto open = build(surface("open_pbr_surface",
			R"(<input name="specular_roughness_anisotropy" type="float" value="0.6" />)"));
		ASSERT_EQ(size_t(1), open.size());
		ASSERT_EQ(std::string(), close({ source_as<ScalarNode>(open[0].second->get_anisotropy())->get_value() }, { 0.6f }));
	}

	// Each supported surface shader maps its own parameter names.
	TEST(MaterialX.GPU, OtherSurfaceShaders)
	{
		using namespace mx_import;

		check_constants(build(surface("gltf_pbr",
			R"(<input name="base_color" type="color3" value="0.2, 0.4, 0.6" />
			   <input name="metallic" type="float" value="0.3" />
			   <input name="roughness" type="float" value="0.8" />)")),
			{ 0.2f, 0.4f, 0.6f, 1.0f }, 0.3f, 0.8f);

		check_constants(build(surface("UsdPreviewSurface",
			R"(<input name="diffuseColor" type="color3" value="0.2, 0.4, 0.6" />
			   <input name="metallic" type="float" value="0.25" />
			   <input name="roughness" type="float" value="0.75" />)")),
			{ 0.2f, 0.4f, 0.6f, 1.0f }, 0.25f, 0.75f);

		check_constants(build(surface("open_pbr_surface",
			R"(<input name="base_weight" type="float" value="0.5" />
			   <input name="base_color" type="color3" value="0.2, 0.4, 0.6" />
			   <input name="base_metalness" type="float" value="0.1" />
			   <input name="specular_roughness" type="float" value="0.45" />)")),
			{ 0.1f, 0.2f, 0.3f, 1.0f }, 0.1f, 0.45f);
	}

	// Connected inputs become nodes: their constants carry over, their
	// connections are rebuilt, and unconnected UV inputs still read the
	// graph's texcoord.
	TEST(MaterialX.GPU, ConnectedNodes)
	{
		using namespace mx_import;

		auto graphs = build(surface("standard_surface",
			R"(<input name="base_color" type="color3" nodename="checker" />
			   <input name="specular_roughness" type="float" nodename="rough" />)",
			R"(<checkerboard name="checker" type="color3">
			     <input name="color1" type="color3" value="0.9, 0.2, 0.1" />
			     <input name="uvtiling" type="vector2" value="8, 8" />
			   </checkerboard>
			   <noise2d name="rough" type="float">
			     <input name="amplitude" type="float" value="0.25" />
			     <input name="pivot" type="float" value="0.45" />
			   </noise2d>)"));

		ASSERT_EQ(size_t(1), graphs.size());
		auto& g = graphs[0].second;

		auto* checker = source_as<MaterialXNode>(g->get_base_color());
		auto* noise = source_as<MaterialXNode>(g->get_roughness());
		ASSERT_TRUE(checker && noise);

		ASSERT_EQ(std::string("ND_checkerboard_color3"), checker->get_nodedef());
		ASSERT_EQ(std::string("ND_noise2d_float"), noise->get_nodedef());

		ASSERT_EQ(std::string(), close(checker->get_constant("color1"), { 0.9f, 0.2f, 0.1f }));
		ASSERT_EQ(std::string(), close(checker->get_constant("uvtiling"), { 8.0f, 8.0f }));
		ASSERT_EQ(std::string(), close(noise->get_constant("amplitude"), { 0.25f }));
		ASSERT_EQ(std::string(), close(noise->get_constant("pivot"), { 0.45f }));

		// color2 was not in the file: still the nodedef default.
		ASSERT_EQ(std::string(), close(checker->get_constant("color2"), { 0.0f, 0.0f, 0.0f }));

		auto* graph_node = dynamic_cast<::FlowGraph::Node*>(g.get());
		ASSERT_TRUE(source(noise->get_input_by_name("texcoord")) == graph_node);
		ASSERT_TRUE(source(checker->get_input_by_name("texcoord")) == graph_node);
	}

	// Integer and boolean inputs are float uniforms like the rest, so the
	// importer sets them as constants on the node and they stay editable.
	TEST(MaterialX.GPU, IntegerInput)
	{
		using namespace mx_import;

		auto graphs = build(surface("standard_surface",
			R"(<input name="specular_roughness" type="float" nodename="fractal" />)",
			R"(<fractal2d name="fractal" type="float">
			     <input name="octaves" type="integer" value="5" />
			   </fractal2d>)"));

		ASSERT_EQ(size_t(1), graphs.size());
		auto* fractal = source_as<MaterialXNode>(graphs[0].second->get_roughness());
		ASSERT_TRUE(fractal != nullptr);

		ASSERT_TRUE(source(fractal->get_input_by_name("octaves")) == nullptr);
		ASSERT_EQ(std::string(), close(fractal->get_constant("octaves"), { 5.0f }));

		// An input the file leaves out keeps the nodedef default (3 octaves).
		auto defaulted = build(surface("standard_surface",
			R"(<input name="specular_roughness" type="float" nodename="fractal" />)",
			R"(<fractal2d name="fractal" type="float" />)"));
		ASSERT_EQ(size_t(1), defaulted.size());
		auto* plain = source_as<MaterialXNode>(defaulted[0].second->get_roughness());
		ASSERT_TRUE(plain != nullptr);
		ASSERT_EQ(std::string(), close(plain->get_constant("octaves"), { 3.0f }));

		// A boolean: range's doclamp defaults to false.
		auto clamped = build(surface("standard_surface",
			R"(<input name="specular_roughness" type="float" nodename="r" />)",
			R"(<range name="r" type="float"><input name="doclamp" type="boolean" value="true" /></range>)"));
		ASSERT_EQ(size_t(1), clamped.size());
		auto* range = source_as<MaterialXNode>(clamped[0].second->get_roughness());
		ASSERT_TRUE(range != nullptr);
		ASSERT_EQ(std::string(), close(range->get_constant("doclamp"), { 1.0f }));
	}

	// Image files become texture assets feeding the image node's file port; a
	// normalmap node is bypassed so the texel reaches the graph's normal output
	// as stored.
	TEST(MaterialX.GPU, TexturesAndNormalMap)
	{
		using namespace mx_import;

		std::string icon = std::filesystem::absolute(to_path(L"resources/icon.jpg")).generic_string();

		auto graphs = build(surface("standard_surface",
			R"(<input name="base_color" type="color3" nodename="albedo" />
			   <input name="normal" type="vector3" nodename="nmap" />)",
			"<image name=\"albedo\" type=\"color3\"><input name=\"file\" type=\"filename\" value=\"" + icon + "\" /></image>\n"
			"<image name=\"nimg\" type=\"vector3\"><input name=\"file\" type=\"filename\" value=\"" + icon + "\" /></image>\n"
			"<normalmap name=\"nmap\" type=\"vector3\"><input name=\"in\" type=\"vector3\" nodename=\"nimg\" /></normalmap>"));

		ASSERT_EQ(size_t(1), graphs.size());
		auto& g = graphs[0].second;

		auto* albedo = source_as<MaterialXNode>(g->get_base_color());
		ASSERT_TRUE(albedo != nullptr);
		ASSERT_EQ(std::string("ND_image_color3"), albedo->get_nodedef());
		ASSERT_TRUE(source_as<TextureAssetNode>(albedo->get_input_by_name("file")) != nullptr);

		auto* normal = source_as<MaterialXNode>(g->get_normals());
		ASSERT_TRUE(normal != nullptr);
		ASSERT_EQ(std::string("ND_image_vector3"), normal->get_nodedef());
		ASSERT_TRUE(source_as<TextureAssetNode>(normal->get_input_by_name("file")) != nullptr);
	}

	TEST(MaterialX.GPU, MultipleMaterialsAndRejects)
	{
		using namespace mx_import;

		auto two = build(
			"<standard_surface name=\"a\" type=\"surfaceshader\"><input name=\"metalness\" type=\"float\" value=\"0.1\" /></standard_surface>\n"
			"<standard_surface name=\"b\" type=\"surfaceshader\"><input name=\"metalness\" type=\"float\" value=\"0.9\" /></standard_surface>\n"
			"<surfacematerial name=\"first\" type=\"material\"><input name=\"surfaceshader\" type=\"surfaceshader\" nodename=\"a\" /></surfacematerial>\n"
			"<surfacematerial name=\"second\" type=\"material\"><input name=\"surfaceshader\" type=\"surfaceshader\" nodename=\"b\" /></surfacematerial>");

		ASSERT_EQ(size_t(2), two.size());
		ASSERT_EQ(std::string("first"), two[0].first);
		ASSERT_EQ(std::string("second"), two[1].first);
		ASSERT_EQ(std::string(), close({ source_as<ScalarNode>(two[0].second->get_mettalic())->get_value() }, { 0.1f }));
		ASSERT_EQ(std::string(), close({ source_as<ScalarNode>(two[1].second->get_mettalic())->get_value() }, { 0.9f }));

		// A shader the importer has no mapping for, and a file with no surface at all.
		ASSERT_EQ(size_t(0), build(surface("disney_principled", "")).size());
		ASSERT_EQ(size_t(0), build("<constant name=\"c\" type=\"float\"><input name=\"value\" type=\"float\" value=\"1\" /></constant>").size());
	}

	// A node the library can't build leaves the input at the shader's default
	// instead of failing the import.
	TEST(MaterialX.GPU, UnsupportedNodeFallsBack)
	{
		using namespace mx_import;

		auto graphs = build(surface("standard_surface",
			R"(<input name="specular_roughness" type="float" nodename="vertex_gray" />)",
			// geomcolor: the mesh format has no vertex colours, so there is nothing to read.
			R"(<geomcolor name="vertex_gray" type="float" />)"));

		ASSERT_EQ(size_t(1), graphs.size());
		auto* rough = source_as<ScalarNode>(graphs[0].second->get_roughness());
		ASSERT_TRUE(rough != nullptr);
		ASSERT_EQ(std::string(), close({ rough->get_value() }, { 0.2f }));
	}
}
