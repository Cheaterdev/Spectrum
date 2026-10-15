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

SETUP_CATEGORY(MaterialX.Nodes, []() {
	if (!GraphicsSystem::create())
		Test::TestRegistry::Instance().SkipCategory("MaterialX.Nodes", "no suitable GPU device found");
});

TEARDOWN_CATEGORY(MaterialX.Nodes, []() {
	GraphicsSystem::reset();
});

SETUP_CATEGORY(MaterialX.Import, []() {
	if (!GraphicsSystem::create())
		Test::TestRegistry::Instance().SkipCategory("MaterialX.Import", "no suitable GPU device found");
});

TEARDOWN_CATEGORY(MaterialX.Import, []() {
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
		inline std::vector<float4> evaluate(const std::vector<Case>& cases)
		{
			auto& device = RenderSystem::get().device();
			const uint width = (uint)cases.size();

			std::string source = stdlib_source();
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

		inline void check(const std::vector<Case>& cases)
		{
			auto values = evaluate(cases);

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
			if (def.category == "image" || def.category == "tiledimage")
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
	TEST(MaterialX.Nodes, MathAndColor)
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
	TEST(MaterialX.Nodes, Noise)
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

	// The importer reads a .mtlx file and registers a material asset for it.
	TEST(MaterialX.Import, SampleFile)
	{
		mx_detail::ensure_library();

		auto sample = to_path(L"materialx/samples/checker_standard_surface.mtlx");
		ASSERT_EQ(size_t(1), MaterialXLibrary::get().import_file(sample));
	}
}
