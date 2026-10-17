module;

// FrameGraph.Base.ixx includes this in the global module, outside its export,
// so importing FrameGraph doesn't make ResourceID nameable. Including it here
// in the purview instead would attach a second, distinct ResourceID to this
// module (link errors on every API taking one).
#include "RenderSystem/FrameGraph/autogen/resource_ids.h"

export module Test.RenderSystem.FrameGraph;

#define TEST_MODULE_ID RSFG

export import Test.Framework;
export import Test.HAL.TextureUtils;

import Core;
import HAL;
import RenderSystem;
import Graphics;
import FrameGraph;

// FrameGraph mechanics, driven through the test pipelines in
// sources/Prism/defs/framegraph_tests.prism. Their passes only clear and copy,
// so what's under test is scheduling, culling, resource lifetimes/aliasing,
// cross-queue sync and the per-backend barriers the graph emits. Every result
// is copied into a passed FGT_Sink texture and checked by CPU readback against
// exact colors.

SETUP_CATEGORY(RenderSystem.FrameGraph, []() {
	if (!GraphicsSystem::create())
		Test::TestRegistry::Instance().SkipCategory("RenderSystem.FrameGraph", "no suitable GPU device found");
});

TEARDOWN_CATEGORY(RenderSystem.FrameGraph, []() {
	GraphicsSystem::reset();
});

namespace
{
	using namespace FrameGraph;
	using namespace Pipelines::FrameGraphTests;
	using TestState = Table::FrameGraphTests::FGTestState;

	constexpr int W = 64, H = 64;

	struct Color
	{
		uint8_t r, g, b;
		vec4 linear() const { return vec4(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f); }
	};

	constexpr Color kRed    { 255,   0,   0 };
	constexpr Color kGreen  {   0, 255,   0 };
	constexpr Color kBlue   {   0,   0, 255 };
	constexpr Color kYellow { 255, 255,   0 };
	constexpr Color kPalette[] = { kRed, kGreen, kBlue, kYellow, { 255, 0, 255 }, { 0, 255, 255 } };

	std::shared_ptr<HAL::TextureResource> make_sink(int w = W, int h = H, uint array = 1, uint mips = 1,
	                                                HAL::Format format = HAL::Format::R8G8B8A8_UNORM)
	{
		return std::make_shared<HAL::TextureResource>(RenderSystem::get().device(),
			HAL::ResourceDesc::Tex2D(format, { (uint)w, (uint)h }, array, mips),
			HAL::HeapType::DEFAULT);
	}

	// One full frame of `pipeline` on a persistent graph, with `sink` passed in
	// as FGT_Sink and FGTestState::size = `size`. Anything else in FGTestState
	// is set by the caller through state() before run().
	struct GraphRunner
	{
		Graph graph;
		int   frame = 0;

		// Runs every frame first, before add_passes and the pipeline's pre-setups
		// -- for scene updates, which main.cpp also does before both.
		std::function<void(Graph&)> on_frame_start;

		// Runs every frame after add_passes, before setup -- for per-frame graph
		// state such as slot generators, which reset() clears.
		std::function<void(Graph&)> on_frame;

		// Passes enabled in the most recent frame, for failure messages.
		std::string last_passes;

		// `frames` frames, each waited on, failing at the first frame after
		// which the device is lost. A lost device on NVIDIA Vulkan shows within
		// the first ~5-8 frames that run scene passes, hence the default.
		template<class Pipeline>
		void run_frames(Pipeline& pipeline, const std::shared_ptr<HAL::TextureResource>& sink,
		                ivec2 size, int frames = 16)
		{
			// SPECTRUM_TEST_FRAMES=<n> overrides the count, for soak runs.
			if (const char* env = std::getenv("SPECTRUM_TEST_FRAMES"); env && std::atoi(env) > 0)
				frames = std::atoi(env);

			for (int i = 0; i < frames; ++i)
			{
				run(pipeline, sink, size);
				if (RenderSystem::get().device().is_lost())
					throw Test::TestFailure("device lost after frame " + std::to_string(i)
						+ " (passes: " + last_passes + ")");
			}
		}

		TestState& state() { return graph.get_context<TestState>(); }

		template<class Pipeline>
		HAL::FenceWaiter run(Pipeline& pipeline, const std::shared_ptr<HAL::TextureResource>& sink,
		                     ivec2 size = ivec2(W, H), bool wait = true)
		{
			graph.start_new_frame();
			graph.get_context<TimeInfo>().frame = frame;
			state().size = size;
			if (on_frame_start)
				on_frame_start(graph);
			pipeline.add_passes(graph);
			if constexpr (requires { pipeline.run_pre_setups(graph); })
				pipeline.run_pre_setups(graph);
			if (on_frame)
				on_frame(graph);
			// After add_passes: TaskBuilder::init sizes per-resource state from
			// the pass count, so nothing may be created or passed before the
			// passes exist (ASSERT(!passes.empty())).
			graph.builder.pass_texture(ResourceID::FGT_Sink, sink, {}, ResourceFlags::Required);
			graph.setup();

			last_passes.clear();
			for (auto* pass : graph.builder.enabled_passes)
				last_passes += std::string(last_passes.empty() ? "" : ", ")
					+ std::string(pass->name.ptr, pass->name.ptr + std::wcslen(pass->name.ptr));

			graph.compile(frame++);
			graph.render();
			auto fence = graph.commit_command_lists();
			if (wait)
				fence.wait();
			graph.reset();
			return fence;
		}
	};

	// ---- render functions ------------------------------------------------------
	// `field` picks the Context member, since every test pass names its own.

	auto uav_clear(auto field, Color color, std::atomic<int>* renders = nullptr)
	{
		return [=](auto& data, FrameContext& ctx) {
			ctx.get_list()->clear_uav(field(data)->rwTexture2D, color.linear());
			if (renders) ++*renders;
		};
	}

	auto rtv_clear(auto field, Color color)
	{
		return [=](auto& data, FrameContext& ctx) {
			HAL::CompiledRT rt;
			rt.table_rtv = field(data)->renderTarget;
			ctx.get_list()->get_graphics().set_rtv(rt,
				HAL::RTOptions::Default | HAL::RTOptions::ClearColor, 0, 0, color.linear());
		};
	}

	auto copy_to_sink(auto field, ivec3 dst_offset = ivec3(0, 0, 0), ivec3 size = ivec3(W, H, 1))
	{
		return [=](auto& data, FrameContext& ctx) {
			ctx.get_list()->get_copy().copy_texture(data.FGT_Sink.info->resource, dst_offset,
				field(data).info->resource, ivec3(0, 0, 0), size);
		};
	}

	// Copies the first `count` subresources one to one; the sink has the same
	// mip/array layout as the source.
	auto copy_subresources_to_sink(auto field, uint count)
	{
		return [=](auto& data, FrameContext& ctx) {
			for (uint sub = 0; sub < count; ++sub)
				ctx.get_list()->get_copy().copy_texture(data.FGT_Sink.info->resource, sub, field(data).info->resource, sub);
		};
	}

	auto counter(std::atomic<int>& n)
	{
		return [&n](auto&, FrameContext&) { ++n; };
	}

	constexpr auto fA       = [](auto& d) -> auto& { return d.FGT_A; };
	constexpr auto fB       = [](auto& d) -> auto& { return d.FGT_B; };
	constexpr auto fANew    = [](auto& d) -> auto& { return d.FGT_ANew; };
	constexpr auto fStatic  = [](auto& d) -> auto& { return d.FGT_Static; };
	constexpr auto fHist    = [](auto& d) -> auto& { return d.FGT_Hist; };
	constexpr auto fHistPrv = [](auto& d) -> auto& { return d.FGT_HistPrev; };
	constexpr auto fUnused  = [](auto& d) -> auto& { return d.FGT_UnusedOut; };
	constexpr auto fReqOut  = [](auto& d) -> auto& { return d.FGT_RequiredOut; };
	constexpr auto fAlias0  = [](auto& d) -> auto& { return d.FGT_Alias0; };
	constexpr auto fAlias1  = [](auto& d) -> auto& { return d.FGT_Alias1; };
	constexpr auto fAlias2  = [](auto& d) -> auto& { return d.FGT_Alias2; };
	constexpr auto fMips    = [](auto& d) -> auto& { return d.FGT_Mips; };
	constexpr auto fArray   = [](auto& d) -> auto& { return d.FGT_Array; };
	constexpr auto fCube    = [](auto& d) -> auto& { return d.FGT_Cube; };
	constexpr auto fMipChain     = [](auto& d) -> auto& { return d.FGT_MipChain; };
	constexpr auto fCubeMipChain = [](auto& d) -> auto& { return d.FGT_CubeMipChain; };

	// ---- verification --------------------------------------------------------

	// Read a sink back once and check every region from that copy:
	// pass_texture() marks it frame_graph_managed for good, which opts it out of
	// the return-to-rest after an ordinary list uses it, so a second readback
	// list would start from the wrong assumed layout (#1334).
	HAL::texture_data::ptr read_sink(HAL::TextureResource* tex)
	{
		auto data = Test::readback_texture(tex);
		if (!data)
			throw Test::TestFailure("sink readback failed");
		return data;
	}

	// Every subresource of a sink, read back in ONE list (see read_sink).
	std::vector<HAL::texture_data::ptr> read_sink_subresources(HAL::TextureResource* tex)
	{
		auto& device = RenderSystem::get().device();
		auto& desc   = tex->get_desc().as_texture();
		const uint count = desc.ArraySize * desc.MipLevels;

		std::vector<HAL::texture_data::ptr> result(count);
		std::vector<std::future<bool>> pending;

		auto list = device.get_upload_list();
		for (uint sub = 0; sub < count; ++sub)
		{
			const uint mip = sub % desc.MipLevels;
			const uint w   = std::max(1u, (uint)desc.Dimensions.x >> mip);
			const uint h   = std::max(1u, (uint)desc.Dimensions.y >> mip);
			pending.push_back(list->get_copy().read_texture(tex, sub,
				[&result, sub, w, h, fmt = desc.Format](std::span<std::byte> data, HAL::texture_layout layout) {
					result[sub] = HAL::texture_data::from_readback(w, h, fmt, data, layout);
				}));
		}
		list->execute_and_wait();
		for (auto& p : pending)
			p.wait();

		for (uint sub = 0; sub < count; ++sub)
			if (!result[sub])
				throw Test::TestFailure("readback failed for subresource " + std::to_string(sub));
		return result;
	}

	void expect_color(const HAL::texture_data& data, Color expected, const std::string& what,
	                  ivec2 origin = ivec2(0, 0), ivec2 size = ivec2(W, H))
	{
		auto& mip = data.array[0]->mips[0];
		for (int y = origin.y; y < origin.y + size.y; ++y)
			for (int x = origin.x; x < origin.x + size.x; ++x)
			{
				auto p = mip->data.data() + (size_t)y * mip->width_stride + (size_t)x * 4;
				if (std::abs(p[0] - expected.r) > 1 || std::abs(p[1] - expected.g) > 1 || std::abs(p[2] - expected.b) > 1)
				{
					throw Test::TestFailure(what + ": pixel (" + std::to_string(x) + "," + std::to_string(y)
						+ ") is (" + std::to_string(p[0]) + "," + std::to_string(p[1]) + "," + std::to_string(p[2])
						+ "), expected (" + std::to_string(expected.r) + "," + std::to_string(expected.g) + ","
						+ std::to_string(expected.b) + ")");
				}
			}
	}

	// ---- mip generation reference ---------------------------------------------

	using Image = std::vector<uint8_t>;   // tightly packed RGBA8

	Image mip_pattern(int size, int variant)
	{
		Image img((size_t)size * size * 4);
		for (int y = 0; y < size; ++y)
			for (int x = 0; x < size; ++x)
			{
				uint8_t* p = &img[((size_t)y * size + x) * 4];
				p[0] = uint8_t(x * 255 / (size - 1));
				p[1] = uint8_t(y * 255 / (size - 1));
				p[2] = uint8_t(((x ^ y) * 37 + variant * 41) & 255);
				p[3] = 255;
			}
		return img;
	}

	// The engine's generator box-filters 2x2 in linear space for non-sRGB
	// formats; this is the same filter on the CPU.
	Image box_downsample(const Image& src, int src_size)
	{
		const int dst_size = std::max(1, src_size / 2);
		Image dst((size_t)dst_size * dst_size * 4);
		for (int y = 0; y < dst_size; ++y)
			for (int x = 0; x < dst_size; ++x)
				for (int c = 0; c < 4; ++c)
				{
					auto at = [&](int sx, int sy) { return (int)src[((size_t)sy * src_size + sx) * 4 + c]; };
					const int sum = at(2 * x, 2 * y) + at(2 * x + 1, 2 * y) + at(2 * x, 2 * y + 1) + at(2 * x + 1, 2 * y + 1);
					dst[((size_t)y * dst_size + x) * 4 + c] = uint8_t((sum + 2) / 4);
				}
		return dst;
	}

	// Tolerance 2: the generator keeps intermediate mips of one dispatch in
	// full precision while the CPU chain rounds each level to 8 bits.
	void expect_image(const HAL::texture_data& data, const Image& expected, int size, const std::string& what)
	{
		auto& mip = data.array[0]->mips[0];
		for (int y = 0; y < size; ++y)
			for (int x = 0; x < size; ++x)
				for (int c = 0; c < 3; ++c)
				{
					const int actual = mip->data[(size_t)y * mip->width_stride + (size_t)x * 4 + c];
					const int want   = expected[((size_t)y * size + x) * 4 + c];
					if (std::abs(actual - want) > 2)
						throw Test::TestFailure(what + ": pixel (" + std::to_string(x) + "," + std::to_string(y) + ") channel "
							+ std::to_string(c) + " is " + std::to_string(actual) + ", box filter gives " + std::to_string(want));
				}
	}

	// ---- HDR input ---------------------------------------------------------------

	// Normal-range values only (the test ramps stay well inside half's range).
	uint16_t float_to_half(float f)
	{
		if (f == 0.0f)
			return 0;
		const uint32_t bits = std::bit_cast<uint32_t>(f);
		const uint32_t sign = (bits >> 16) & 0x8000;
		const int      exp  = int((bits >> 23) & 0xFF) - 127 + 15;
		const uint32_t mant = bits & 0x7FFFFF;
		// '+', not '|': a mantissa that rounds up to 0x400 must carry into the
		// exponent (OR halved values just below a power of two).
		return uint16_t(sign | ((uint32_t(std::clamp(exp, 1, 30)) << 10) + ((mant + 0x1000) >> 13)));
	}

	// Three horizontal bands (neutral, warm, cool), each a log-luminance ramp
	// from 2^-4 to 2^6 left to right: covers the toe, the mid tones and the
	// shoulder of a tonemap curve. RGBA16F, tightly packed.
	constexpr float kHdrMinLog = -4.0f, kHdrMaxLog = 6.0f;
	const vec3 kHdrTints[3] = { vec3(1.0f, 1.0f, 1.0f), vec3(1.0f, 0.6f, 0.3f), vec3(0.3f, 0.6f, 1.0f) };

	std::vector<uint16_t> hdr_ramp_image(int w, int h)
	{
		std::vector<uint16_t> px((size_t)w * h * 4);
		for (int y = 0; y < h; ++y)
		{
			const vec3 tint = kHdrTints[std::min(y * 3 / h, 2)];
			for (int x = 0; x < w; ++x)
			{
				const float lum = std::exp2(kHdrMinLog + (kHdrMaxLog - kHdrMinLog) * x / float(w - 1));
				uint16_t* p = &px[((size_t)y * w + x) * 4];
				p[0] = float_to_half(lum * tint.x);
				p[1] = float_to_half(lum * tint.y);
				p[2] = float_to_half(lum * tint.z);
				p[3] = float_to_half(1.0f);
			}
		}
		return px;
	}

	// Engine passes bind FrameInfo through the graph's slot setters; reset()
	// drops generators, so this goes in GraphRunner::on_frame.
	void add_frame_info(Graph& graph)
	{
		graph.add_slot_generator([](Graph& g) {
			Slots::Frame::FrameInfo fi;
			fi.GetSunDir().xyz     = g.get_context<SkyInfo>().sunDir;
			fi.GetBrdf()           = EngineAssets::brdf.get_asset()->get_texture()->texture_3d().texture3D;
			fi.GetBestFitNormals() = EngineAssets::best_fit_normals.get_asset()->get_texture()->texture_2d().texture2D;
			g.register_slot_setter(fi.compile(*g.builder.current_frame));
		});
	}

	// ---- synthetic GBuffer ----------------------------------------------------------

	// Formats as scene.prism's Scene pass creates them. Top fifth is sky (depth
	// 0); below, normals sweep the hemisphere (azimuth with x, elevation with y)
	// over four albedo bands, the last one half metallic (albedo.a, which the
	// lighting scales by 1 - a).
	struct SyntheticGBuffer
	{
		int w, h;
		std::vector<uint8_t>  albedo, normals, specular;   // RGBA8
		std::vector<uint16_t> speed;                       // RG16F, zero
		std::vector<float>    depth;                       // R32

		SyntheticGBuffer(int w, int h) : w(w), h(h),
			albedo((size_t)w * h * 4), normals((size_t)w * h * 4), specular((size_t)w * h * 4, 0),
			speed((size_t)w * h * 2, 0), depth((size_t)w * h)
		{
			const uint8_t band_albedo[4][4] = { { 230, 60, 40, 0 }, { 60, 200, 80, 0 }, { 50, 90, 220, 0 }, { 200, 200, 200, 128 } };
			for (int y = 0; y < h; ++y)
				for (int x = 0; x < w; ++x)
				{
					const size_t i = (size_t)y * w + x;
					depth[i] = y < h / 5 ? 0.0f : 0.5f;

					const float azimuth   = Math::pi * 2.0f * x / w;
					const float elevation = Math::pi * 0.5f * y / h;
					const vec3  n(std::cos(azimuth) * std::cos(elevation), std::sin(elevation), std::sin(azimuth) * std::cos(elevation));
					normals[i * 4 + 0] = uint8_t(std::lround((n.x * 0.5f + 0.5f) * 255));
					normals[i * 4 + 1] = uint8_t(std::lround((n.y * 0.5f + 0.5f) * 255));
					normals[i * 4 + 2] = uint8_t(std::lround((n.z * 0.5f + 0.5f) * 255));
					normals[i * 4 + 3] = 255;

					std::memcpy(&albedo[i * 4], band_albedo[std::min(x * 4 / w, 3)], 4);
				}
		}

		template<class Data>
		void upload(Data& data, FrameContext& ctx) const
		{
			auto& copy = ctx.get_list()->get_copy();
			auto  put  = [&](auto& handler, const void* src, uint bytes_per_pixel) {
				copy.update_texture(handler.info->resource, ivec3(0, 0, 0), ivec3(w, h, 1), 0,
					reinterpret_cast<const char*>(src), w * bytes_per_pixel);
			};
			put(data.GBuffer_Albedo,    albedo.data(),   4);
			put(data.GBuffer_Normals,   normals.data(),  4);
			put(data.GBuffer_Specular,  specular.data(), 4);
			put(data.GBuffer_Speed,     speed.data(),    4);
			put(data.GBuffer_DepthMips, depth.data(),    4);
		}

		// vsm.hlsl's unshadowed_result(): 0 on sky, else
		// saturate(dot(N, L)) * albedo.rgb * (1 - albedo.a).
		vec3 unshadowed(int x, int y, vec3 sun) const
		{
			const size_t i = (size_t)y * w + x;
			if (depth[i] == 0.0f)
				return vec3(0, 0, 0);
			vec3 n(normals[i * 4] / 255.0f * 2 - 1, normals[i * 4 + 1] / 255.0f * 2 - 1, normals[i * 4 + 2] / 255.0f * 2 - 1);
			n = n.normalize();
			const float nl = std::clamp(n.x * sun.x + n.y * sun.y + n.z * sun.z, 0.0f, 1.0f);
			const float k  = nl * (1.0f - albedo[i * 4 + 3] / 255.0f);
			return vec3(albedo[i * 4] / 255.0f * k, albedo[i * 4 + 1] / 255.0f * k, albedo[i * 4 + 2] / 255.0f * k);
		}
	};

	float half_to_float_normal(uint16_t v)
	{
		return std::bit_cast<float>(((v & 0x8000u) << 16) | (((v >> 10) & 0x1Fu) ? ((((v >> 10) & 0x1Fu) + 112u) << 23) : 0u) | ((v & 0x3FFu) << 13));
	}

	template<class Pipeline>
	void check_async_queue(Color color, const std::string& what)
	{
		auto sink = make_sink();
		Pipeline p;
		p.fGT_ProduceUAV.render_func  = uav_clear(fA, color);
		p.fGT_CopyAToSink.render_func = copy_to_sink(fA);
		GraphRunner r;
		r.run(p, sink);
		expect_color(*read_sink(sink.get()), color, what);
	}
}

export namespace Test
{
	// Direct-queue producer (RTV clear) -> consumer copy.
	TEST(RenderSystem.FrameGraph, ProducerConsumer_Graphics)
	{
		auto sink = make_sink();
		FGT_GraphicsPipeline p;
		p.fGT_ProduceRTV.render_func  = rtv_clear(fA, kGreen);
		p.fGT_CopyAToSink.render_func = copy_to_sink(fA);
		GraphRunner r;
		r.run(p, sink);
		expect_color(*read_sink(sink.get()), kGreen, "sink");
	}

	// Async-compute producer -> direct-queue consumer, on each compute queue.
	// Exercises the cross-queue wait the graph inserts for the hand-off.
	TEST(RenderSystem.FrameGraph, AsyncCompute_ToGraphics_AllQueues)
	{
		check_async_queue<FGT_Async1Pipeline>(kRed,   "Async");
		check_async_queue<FGT_Async2Pipeline>(kGreen, "Async2");
		check_async_queue<FGT_Async3Pipeline>(kBlue,  "Async3");
	}

	// Direct -> async compute -> direct: the compute pass consumes a graphics
	// output and produces the input of a later graphics pass, so both
	// directions of cross-queue sync are needed in one frame.
	TEST(RenderSystem.FrameGraph, CrossQueue_RoundTrip)
	{
		auto sink = make_sink();
		FGT_RoundTripPipeline p;
		p.fGT_ProduceRTV.render_func      = rtv_clear(fA, kBlue);
		p.fGT_ComputeCopyAToB.render_func = [](auto& data, FrameContext& ctx) {
			ctx.get_list()->get_copy().copy_texture(data.FGT_B.info->resource, 0, data.FGT_A.info->resource, 0);
		};
		p.fGT_CopyBToSink.render_func     = copy_to_sink(fB);
		GraphRunner r;
		r.run(p, sink);
		expect_color(*read_sink(sink.get()), kBlue, "sink");
	}

	// A pass whose output nobody needs is culled; a [Required] one runs anyway.
	TEST(RenderSystem.FrameGraph, Culling_UnusedVsRequired)
	{
		std::atomic<int> unused = 0, required = 0, used = 0;
		auto sink = make_sink();
		FGT_CullingPipeline p;
		p.fGT_Unused.render_func         = uav_clear(fUnused, kRed, &unused);
		p.fGT_RequiredUnused.render_func = uav_clear(fReqOut, kRed, &required);
		p.fGT_ProduceUAV.render_func     = uav_clear(fA, kGreen, &used);
		p.fGT_CopyAToSink.render_func    = copy_to_sink(fA);
		GraphRunner r;
		r.run(p, sink);
		ASSERT_EQ(0, unused.load());
		ASSERT_EQ(1, required.load());
		ASSERT_EQ(1, used.load());
		expect_color(*read_sink(sink.get()), kGreen, "sink");
	}

	// [SetupCondition = false] -> Disabled: never renders. [RenderCondition =
	// false] -> IgnoreRender: links its resources but never renders.
	TEST(RenderSystem.FrameGraph, SetupResult_DisabledAndIgnoreRender)
	{
		std::atomic<int> disabled = 0, ignored = 0;
		auto sink = make_sink();
		FGT_SetupResultPipeline p;
		p.fGT_ProduceUAV.render_func   = uav_clear(fA, kYellow);
		p.fGT_Disabled.render_func     = counter(disabled);
		p.fGT_IgnoreRender.render_func = counter(ignored);
		p.fGT_CopyAToSink.render_func  = copy_to_sink(fA);
		GraphRunner r;
		r.run(p, sink);
		ASSERT_EQ(0, disabled.load());
		ASSERT_EQ(0, ignored.load());
		expect_color(*read_sink(sink.get()), kYellow, "sink");
	}

	// [Recreate]: a later pass derives a new version of FGT_A; consumers after
	// it must see the new version, not the one it was recreated from.
	TEST(RenderSystem.FrameGraph, Recreate_ConsumerSeesLatestVersion)
	{
		auto sink = make_sink();
		FGT_RecreatePipeline p;
		p.fGT_ProduceUAV.render_func  = uav_clear(fA, kRed);
		p.fGT_Recreate.render_func    = uav_clear(fANew, kGreen);
		p.fGT_CopyAToSink.render_func = copy_to_sink(fA);
		GraphRunner r;
		r.run(p, sink);
		expect_color(*read_sink(sink.get()), kGreen, "sink");
	}

	// Transient resources with disjoint lifetimes may share memory, one of them
	// on the async queue. Each is written and immediately copied out to its own
	// region of the sink, so a missing aliasing barrier or a wrongly overlapped
	// allocation shows up as the wrong color in some region.
	TEST(RenderSystem.FrameGraph, TransientAliasing_DisjointLifetimes)
	{
		auto sink = make_sink(W * 3, H);
		FGT_AliasingPipeline p;
		p.fGT_ProduceAlias0.render_func = uav_clear(fAlias0, kPalette[0]);
		p.fGT_CopyAlias0.render_func    = copy_to_sink(fAlias0, ivec3(0 * W, 0, 0));
		p.fGT_ProduceAlias1.render_func = uav_clear(fAlias1, kPalette[1]);
		p.fGT_CopyAlias1.render_func    = copy_to_sink(fAlias1, ivec3(1 * W, 0, 0));
		p.fGT_ProduceAlias2.render_func = uav_clear(fAlias2, kPalette[2]);
		p.fGT_CopyAlias2.render_func    = copy_to_sink(fAlias2, ivec3(2 * W, 0, 0));
		GraphRunner r;
		r.run(p, sink);
		auto result = read_sink(sink.get());
		for (int i = 0; i < 3; ++i)
			expect_color(*result, kPalette[i], "region " + std::to_string(i), { i * W, 0 });
	}

	// A Static resource keeps its contents across frames: written in frame 0
	// only (IgnoreRender afterwards), read every frame.
	TEST(RenderSystem.FrameGraph, StaticResource_PersistsAcrossFrames)
	{
		GraphRunner r;
		for (int frame = 0; frame < 3; ++frame)
		{
			auto sink = make_sink();
			FGT_StaticPipeline p;
			p.fGT_WriteStatic.render_func      = uav_clear(fStatic, kBlue);
			p.fGT_CopyStaticToSink.render_func = copy_to_sink(fStatic);
			r.state().write_static = frame == 0;
			r.run(p, sink);
			expect_color(*read_sink(sink.get()), kBlue, "frame " + std::to_string(frame));
		}
	}

	// [PrevFor]: the prev resource must hold the current resource's contents
	// from the previous frame.
	TEST(RenderSystem.FrameGraph, History_PrevHoldsLastFrame)
	{
		GraphRunner r;
		for (int frame = 0; frame < 4; ++frame)
		{
			auto sink = make_sink();
			FGT_HistoryPipeline p;
			p.fGT_WriteHist.render_func          = uav_clear(fHist, kPalette[frame]);
			p.fGT_CopyHistPrevToSink.render_func = copy_to_sink(fHistPrv);
			r.run(p, sink);
			// Frame 0's prev has no history yet.
			if (frame > 0)
				expect_color(*read_sink(sink.get()), kPalette[frame - 1], "frame " + std::to_string(frame));
		}
	}

	// Transient sizes change every frame, as the viewport does while the
	// window is being resized.
	TEST(RenderSystem.FrameGraph, Resize_EveryFrame)
	{
		const ivec2 sizes[] = { { 64, 64 }, { 96, 48 }, { 33, 17 }, { 128, 128 }, { 64, 64 } };
		GraphRunner r;
		for (int frame = 0; frame < (int)std::size(sizes); ++frame)
		{
			const ivec2 size  = sizes[frame];
			const Color color = kPalette[frame % std::size(kPalette)];
			auto sink = make_sink(size.x, size.y);
			FGT_Async1Pipeline p;
			p.fGT_ProduceUAV.render_func  = uav_clear(fA, color);
			p.fGT_CopyAToSink.render_func = copy_to_sink(fA, ivec3(0, 0, 0), ivec3(size, 1));
			r.run(p, sink, size);
			expect_color(*read_sink(sink.get()), color, "frame " + std::to_string(frame), {}, size);
		}
	}

	// Several frames submitted back to back without waiting, as the app does
	// with frames in flight: transient memory and command lists from frame N
	// must not be recycled while frame N is still on the GPU.
	TEST(RenderSystem.FrameGraph, FramesInFlight_NoWait)
	{
		constexpr int frames = 6;
		std::vector<std::shared_ptr<HAL::TextureResource>> sinks;
		std::vector<HAL::FenceWaiter> fences;

		GraphRunner r;
		for (int frame = 0; frame < frames; ++frame)
		{
			sinks.push_back(make_sink());
			FGT_Async1Pipeline p;
			p.fGT_ProduceUAV.render_func  = uav_clear(fA, kPalette[frame]);
			p.fGT_CopyAToSink.render_func = copy_to_sink(fA);
			fences.push_back(r.run(p, sinks.back(), ivec2(W, H), false));
		}
		for (auto& f : fences)
			f.wait();

		for (int frame = 0; frame < frames; ++frame)
			expect_color(*read_sink(sinks[frame].get()), kPalette[frame], "frame " + std::to_string(frame));
	}

	// Mip 0 written on the direct queue, mips 1..3 on async compute, each
	// through its own single-mip UAV: the graph and the HAL must track the mips
	// separately and hand the half-written texture across queues.
	TEST(RenderSystem.FrameGraph, Mips_PerMipWritesAcrossQueues)
	{
		constexpr uint mips = 4;
		auto sink = make_sink(W, H, 1, mips);
		FGT_MipsPipeline p;
		p.fGT_WriteMip0.render_func = [](auto& data, FrameContext& ctx) {
			ctx.get_list()->clear_uav(data.FGT_Mips->create_mip(0, *ctx.get_list()).rwTexture2D, kPalette[0].linear());
		};
		p.fGT_WriteUpperMips.render_func = [](auto& data, FrameContext& ctx) {
			for (uint m = 1; m < mips; ++m)
				ctx.get_list()->clear_uav(data.FGT_Mips->create_mip(m, *ctx.get_list()).rwTexture2D, kPalette[m].linear());
		};
		p.fGT_CopyMipsToSink.render_func = copy_subresources_to_sink(fMips, mips);
		GraphRunner r;
		r.run(p, sink);

		auto subs = read_sink_subresources(sink.get());
		for (uint m = 0; m < mips; ++m)
			expect_color(*subs[m], kPalette[m], "mip " + std::to_string(m), {}, { W >> m, H >> m });
	}

	// Slices 0..1 written as UAVs on async compute, slice 2 as a render target
	// on the direct queue: one resource in two different layouts at once.
	TEST(RenderSystem.FrameGraph, ArraySlices_UAVAndRTVAcrossQueues)
	{
		constexpr uint slices = 3;
		auto sink = make_sink(W, H, slices, 1);
		FGT_ArrayPipeline p;
		p.fGT_WriteSlicesUAV.render_func = [](auto& data, FrameContext& ctx) {
			for (uint s = 0; s < 2; ++s)
				ctx.get_list()->clear_uav(data.FGT_Array->create_2d_slice(s, *ctx.get_list()).rwTexture2D, kPalette[s].linear());
		};
		p.fGT_WriteSliceRTV.render_func = [](auto& data, FrameContext& ctx) {
			HAL::CompiledRT rt;
			rt.table_rtv = data.FGT_Array->create_2d_slice(2, *ctx.get_list()).renderTarget;
			ctx.get_list()->get_graphics().set_rtv(rt,
				HAL::RTOptions::Default | HAL::RTOptions::ClearColor, 0, 0, kPalette[2].linear());
		};
		p.fGT_CopyArrayToSink.render_func = copy_subresources_to_sink(fArray, slices);
		GraphRunner r;
		r.run(p, sink);

		auto subs = read_sink_subresources(sink.get());
		for (uint s = 0; s < slices; ++s)
			expect_color(*subs[s], kPalette[s], "slice " + std::to_string(s));
	}

	// Each cube face cleared through its own face UAV, copied face by face into
	// a 6-slice sink.
	TEST(RenderSystem.FrameGraph, CubeFaces_PerFaceWrites)
	{
		constexpr int size = 32;
		auto sink = make_sink(size, size, 6, 1);
		FGT_CubePipeline p;
		p.fGT_WriteCubeFaces.render_func = [](auto& data, FrameContext& ctx) {
			for (uint f = 0; f < 6; ++f)
				ctx.get_list()->clear_uav(data.FGT_Cube->faces[f].rwTexture2D, kPalette[f].linear());
		};
		p.fGT_CopyCubeToSink.render_func = copy_subresources_to_sink(fCube, 6);
		GraphRunner r;
		r.run(p, sink);

		auto subs = read_sink_subresources(sink.get());
		for (uint f = 0; f < 6; ++f)
			expect_color(*subs[f], kPalette[f], "face " + std::to_string(f), {}, { size, size });
	}

	// CPU data uploaded into a transient structured buffer on the direct queue,
	// copied into a Static one on async compute, read back after the frame.
	TEST(RenderSystem.FrameGraph, Buffer_UploadThenCopyAcrossQueues)
	{
		constexpr uint count = 256;
		std::vector<uint> expected(count);
		for (uint i = 0; i < count; ++i)
			expected[i] = i * 3 + 7;

		HAL::Resource::ptr result;
		auto sink = make_sink();   // unused by this pipeline; the runner always passes one
		FGT_BufferPipeline p;
		p.fGT_UploadBuffer.render_func = [&](auto& data, FrameContext& ctx) {
			ctx.get_list()->get_copy().update<uint>(*data.FGT_BufA, 0, std::span<uint>(expected));
		};
		p.fGT_CopyBuffer.render_func = [&](auto& data, FrameContext& ctx) {
			result = data.FGT_BufB.info->resource;
			ctx.get_list()->get_copy().copy_buffer(data.FGT_BufB.info->resource.get(), 0,
				data.FGT_BufA.info->resource.get(), 0, count * sizeof(uint));
		};
		GraphRunner r;
		r.run(p, sink);
		ASSERT_TRUE(result != nullptr);

		std::vector<uint> actual(count);
		auto list = RenderSystem::get().device().get_upload_list();
		auto done = list->get_copy().read_buffer(result.get(), 0, count * sizeof(uint), [&](std::span<std::byte> bytes) {
			std::memcpy(actual.data(), bytes.data(), std::min(bytes.size(), actual.size() * sizeof(uint)));
		});
		list->execute_and_wait();
		done.wait();

		for (uint i = 0; i < count; ++i)
			if (actual[i] != expected[i])
				throw Test::TestFailure("element " + std::to_string(i) + " is " + std::to_string(actual[i])
					+ ", expected " + std::to_string(expected[i]));
	}

	// MipMapGenerator::generate on async compute over a pattern uploaded into
	// mip 0. Every mip is checked against a CPU box filter, and against a
	// golden image (D3D12 reference, for comparing other backends).
	TEST(RenderSystem.FrameGraph, MipGeneration_2D)
	{
		constexpr int size = W;
		constexpr uint mips = 7;   // 64 -> 1
		const Image level0 = mip_pattern(size, 0);

		auto sink = make_sink(size, size, 1, mips);
		FGT_MipGenPipeline p;
		p.fGT_UploadMipPattern.render_func = [&](auto& data, FrameContext& ctx) {
			ctx.get_list()->get_copy().update_texture(data.FGT_MipChain.info->resource, ivec3(0, 0, 0),
				ivec3(size, size, 1), 0, reinterpret_cast<const char*>(level0.data()), size * 4);
		};
		p.fGT_GenerateMips.render_func = [](auto& data, FrameContext& ctx) {
			MipMapGenerator::get().generate(ctx.get_list()->get_compute(), *data.FGT_MipChain);
		};
		p.fGT_CopyMipChainToSink.render_func = copy_subresources_to_sink(fMipChain, mips);
		GraphRunner r;
		r.run(p, sink, ivec2(size, size));

		auto subs = read_sink_subresources(sink.get());
		Image expected = level0;
		for (uint m = 0; m < mips; ++m)
		{
			const int mip_size = size >> m;
			if (m > 0)
				expected = box_downsample(expected, mip_size * 2);
			expect_image(*subs[m], expected, mip_size, "mip " + std::to_string(m));
			Test::check_texture_data_reference(subs[m], "fg_mipgen_2d_mip" + std::to_string(m));
		}
	}

	// MipMapGenerator::generate_cube: one dispatch per batch covers all six
	// faces (face in Z), a separate code path from the 2D generator. Each face
	// gets a different pattern so a face mix-up shows.
	TEST(RenderSystem.FrameGraph, MipGeneration_Cube)
	{
		constexpr int size = 32;
		constexpr uint mips = 6;   // 32 -> 1
		std::vector<Image> level0;
		for (int f = 0; f < 6; ++f)
			level0.push_back(mip_pattern(size, f));

		auto sink = make_sink(size, size, 6, mips);
		FGT_CubeMipGenPipeline p;
		p.fGT_UploadCubeMipPattern.render_func = [&](auto& data, FrameContext& ctx) {
			for (uint f = 0; f < 6; ++f)
				ctx.get_list()->get_copy().update_texture(data.FGT_CubeMipChain.info->resource, ivec3(0, 0, 0),
					ivec3(size, size, 1), f * mips, reinterpret_cast<const char*>(level0[f].data()), size * 4);
		};
		p.fGT_GenerateCubeMips.render_func = [](auto& data, FrameContext& ctx) {
			MipMapGenerator::get().generate_cube(ctx.get_list()->get_compute(), *data.FGT_CubeMipChain);
		};
		p.fGT_CopyCubeMipChainToSink.render_func = copy_subresources_to_sink(fCubeMipChain, 6 * mips);
		GraphRunner r;
		r.run(p, sink);

		auto subs = read_sink_subresources(sink.get());
		for (uint f = 0; f < 6; ++f)
		{
			Image expected = level0[f];
			for (uint m = 0; m < mips; ++m)
			{
				const int  mip_size = size >> m;
				const uint sub      = m + f * mips;
				const auto what     = "face " + std::to_string(f) + " mip " + std::to_string(m);
				if (m > 0)
					expected = box_downsample(expected, mip_size * 2);
				expect_image(*subs[sub], expected, mip_size, what);
				Test::check_texture_data_reference(subs[sub],
					"fg_mipgen_cube_f" + std::to_string(f) + "_mip" + std::to_string(m));
			}
		}
	}

	// The real sky bake: CubeSky renders the atmosphere into sky_cubemap
	// (SkyRender's render function, PreSetup sun-direction diff),
	// CubeMapDownsample builds its mips, CubeMapEnviromentProcessor filters the
	// specular and diffuse IBL cubes. Mip 0 of every face of both is compared
	// against D3D12 goldens, tonemapped to 8 bits.
	TEST(RenderSystem.FrameGraph, Effect_SkyBake)
	{
		constexpr int size = 64;   // sky_cubemap_filtered / _diffuse, sky.prism
		auto sink = make_sink(size, size, 12, 1, HAL::Format::R11G11B10_FLOAT);

		FGT_SkyPipeline p;
		SkyRender sky(p);
		p.fGT_CopySkyToSink.render_func = [](auto& data, FrameContext& ctx) {
			auto& copy = ctx.get_list()->get_copy();
			const auto& filtered = data.sky_cubemap_filtered.info->resource;
			const auto& diffuse  = data.sky_cubemap_filtered_diffuse.info->resource;
			const uint filtered_mips = filtered->get_desc().as_texture().MipLevels;
			const uint diffuse_mips  = diffuse->get_desc().as_texture().MipLevels;
			for (uint f = 0; f < 6; ++f)
			{
				copy.copy_texture(data.FGT_Sink.info->resource, f,     filtered, f * filtered_mips);
				copy.copy_texture(data.FGT_Sink.info->resource, 6 + f, diffuse,  f * diffuse_mips);
			}
		};

		GraphRunner r;
		r.graph.get_context<SkyInfo>().sunDir = float3(0.3f, 0.6f, 0.4f).normalize();
		r.on_frame = add_frame_info;
		r.run(p, sink, ivec2(size, size));

		auto subs = read_sink_subresources(sink.get());
		const Test::TextureCompare hdr{ .tolerance = 4, .float_encoding = Test::FloatEncoding::Tonemapped, .exposure = 1.0f };
		for (uint f = 0; f < 6; ++f)
		{
			Test::check_texture_data_reference(subs[f],     "fg_sky_specular_f" + std::to_string(f), hdr);
			Test::check_texture_data_reference(subs[6 + f], "fg_sky_diffuse_f" + std::to_string(f), hdr);
		}
	}

	// The real Tonemap pass (histogram build, auto-exposure adapt, AgX apply) in
	// place on a synthetic HDR ResultTexture. Bloom is off, so Tonemap creates
	// ExposureState itself; auto exposure snaps on first use (Tonemap.cpp's
	// g_reset), so one frame is deterministic. Checked for what any display
	// tonemap must guarantee -- output in [0, 1], monotonic along each luminance
	// ramp -- and against a D3D12 golden.
	TEST(RenderSystem.FrameGraph, Effect_Tonemap)
	{
		constexpr int w = kReferenceWidth, h = kReferenceHeight;
		const auto input = hdr_ramp_image(w, h);
		auto sink = make_sink(w, h, 1, 1, HAL::Format::R16G16B16A16_FLOAT);

		FGT_TonemapPipeline p;
		p.fGT_UploadHDR.render_func = [&](auto& data, FrameContext& ctx) {
			ctx.get_list()->get_copy().update_texture(data.ResultTexture.info->resource, ivec3(0, 0, 0),
				ivec3(w, h, 1), 0, reinterpret_cast<const char*>(input.data()), w * 4 * sizeof(uint16_t));
		};
		p.fGT_CopyResultToSink.render_func = [](auto& data, FrameContext& ctx) {
			ctx.get_list()->get_copy().copy_texture(data.FGT_Sink.info->resource, 0, data.ResultTexture.info->resource, 0);
		};

		GraphRunner r;
		r.graph.get_context<Table::Post::BloomSelectors>().enabled = false;
		r.graph.get_context<ViewportInfo>().frame_size   = { w, h };
		r.graph.get_context<ViewportInfo>().upscale_size = { w, h };
		r.run(p, sink, ivec2(w, h));

		auto result = read_sink(sink.get());

		// Linear readback of the FP16 output for the range/monotonic checks.
		auto& mip = result->array[0]->mips[0];
		auto  lum = [&](int x, int y) {
			auto px = reinterpret_cast<const uint16_t*>(mip->data.data() + (size_t)y * mip->width_stride) + x * 4;
			auto half = [](uint16_t v) { return std::bit_cast<float>(((v & 0x8000u) << 16) | (((v >> 10) & 0x1Fu) ? ((((v >> 10) & 0x1Fu) + 112u) << 23) : 0u) | ((v & 0x3FFu) << 13)); };
			return 0.2126f * half(px[0]) + 0.7152f * half(px[1]) + 0.0722f * half(px[2]);
		};
		for (int band = 0; band < 3; ++band)
		{
			const int y = band * h / 3 + h / 6;
			float prev = -1.0f;
			for (int x = 0; x < w; ++x)
			{
				const float l = lum(x, y);
				if (l < 0.0f || l > 1.001f)
					throw Test::TestFailure("band " + std::to_string(band) + " x " + std::to_string(x)
						+ ": tonemapped luminance " + std::to_string(l) + " outside [0, 1]");
				if (l + 1e-3f < prev)
					throw Test::TestFailure("band " + std::to_string(band) + " x " + std::to_string(x)
						+ ": tonemapped luminance drops from " + std::to_string(prev) + " to " + std::to_string(l));
				prev = l;
			}
		}

		Test::check_texture_data_reference(result, "fg_tonemap_agx");
	}

	// The real bloom chain (BloomBuild: threshold downsample + temporal resolve,
	// BloomProcess: down/up chain + lens flare, BloomComposite: add onto
	// ResultTexture) on a dim scene with three small, very bright squares. One
	// frame: the temporal step resets on its first frame, and ExposureState is
	// zeroed when created, so the threshold is deterministic. Checked for a halo
	// around each bright square (energy where the input was dim) and against a
	// D3D12 golden of the HDR result, tonemapped.
	TEST(RenderSystem.FrameGraph, Effect_Bloom)
	{
		constexpr int w = kReferenceWidth, h = kReferenceHeight;
		constexpr float background = 0.05f;
		// Positions/sizes as fractions of the image: bloom's radius scales with
		// resolution (it works on upscale_size / 4 and below), so this keeps the
		// scene the same at any reference size.
		struct Spot { int x, y; vec3 color; };
		// Rounded in double: int(h * 0.70f) truncates 755.99998 on MSVC but 756.0 on Clang, which
		// moved the spot by a pixel between platforms.
		const auto at = [](int n, float fraction) { return int(std::lround(double(n) * fraction)); };
		const Spot spots[] = {
			{ at(w, 0.19f), at(h, 0.31f), vec3(60.0f,  8.0f,  4.0f) },
			{ at(w, 0.50f), at(h, 0.50f), vec3(40.0f, 40.0f, 40.0f) },
			{ at(w, 0.78f), at(h, 0.70f), vec3( 4.0f, 10.0f, 60.0f) },
		};
		const int spot_half = std::max(3, h / 40);
		const int halo_gap  = std::max(8, h / 16);

		std::vector<uint16_t> input((size_t)w * h * 4);
		for (int y = 0; y < h; ++y)
			for (int x = 0; x < w; ++x)
			{
				vec3 c(background, background, background);
				for (auto& s : spots)
					if (std::abs(x - s.x) <= spot_half && std::abs(y - s.y) <= spot_half)
						c = s.color;
				uint16_t* p = &input[((size_t)y * w + x) * 4];
				p[0] = float_to_half(c.x); p[1] = float_to_half(c.y); p[2] = float_to_half(c.z); p[3] = float_to_half(1.0f);
			}

		auto sink = make_sink(w, h, 1, 1, HAL::Format::R16G16B16A16_FLOAT);

		FGT_BloomPipeline p;
		p.fGT_UploadHDR.render_func = [&](auto& data, FrameContext& ctx) {
			ctx.get_list()->get_copy().update_texture(data.ResultTexture.info->resource, ivec3(0, 0, 0),
				ivec3(w, h, 1), 0, reinterpret_cast<const char*>(input.data()), w * 4 * sizeof(uint16_t));
		};
		p.fGT_ProduceBloomInputs.render_func = [](auto& data, FrameContext& ctx) {
			HAL::CompiledRT rt;
			rt.table_rtv = data.GBuffer_Speed->renderTarget;
			rt.table_dsv = data.GBuffer_Depth->depthStencil;
			ctx.get_list()->get_graphics().set_rtv(rt,
				HAL::RTOptions::Default | HAL::RTOptions::ClearColor | HAL::RTOptions::ClearDepth, 0, 0, vec4(0, 0, 0, 0));
		};
		p.fGT_CopyResultToSink.render_func = [](auto& data, FrameContext& ctx) {
			ctx.get_list()->get_copy().copy_texture(data.FGT_Sink.info->resource, 0, data.ResultTexture.info->resource, 0);
		};

		GraphRunner r;
		r.graph.get_context<ViewportInfo>().frame_size   = { w, h };
		r.graph.get_context<ViewportInfo>().upscale_size = { w, h };
		r.on_frame = add_frame_info;
		r.run(p, sink, ivec2(w, h));

		auto result = read_sink(sink.get());

		auto& mip = result->array[0]->mips[0];
		auto  lum = [&](int x, int y) {
			auto px = reinterpret_cast<const uint16_t*>(mip->data.data() + (size_t)y * mip->width_stride) + x * 4;
			auto half = [](uint16_t v) { return std::bit_cast<float>(((v & 0x8000u) << 16) | (((v >> 10) & 0x1Fu) ? ((((v >> 10) & 0x1Fu) + 112u) << 23) : 0u) | ((v & 0x3FFu) << 13)); };
			return 0.2126f * half(px[0]) + 0.7152f * half(px[1]) + 0.0722f * half(px[2]);
		};
		for (int i = 0; i < 3; ++i)
		{
			// Just outside the square the input is pure background; bloom must
			// have spread energy there.
			const float halo = lum(spots[i].x + spot_half + halo_gap, spots[i].y);
			if (!(halo > background * 1.5f))
				throw Test::TestFailure("no bloom halo next to spot " + std::to_string(i) + ": luminance "
					+ std::to_string(halo) + " vs background " + std::to_string(background));
		}

		Test::check_texture_data_reference(result, "fg_bloom",
			Test::TextureCompare{ .float_encoding = Test::FloatEncoding::Tonemapped, .exposure = 1.0f });
	}

	// VSM_Unshadowed (the path without sparse depth: VSMSelectors::supported
	// false) on async compute over a synthetic GBuffer, 16 frames. Faulted the
	// device on NVIDIA Vulkan with sparse depth off (VK_ERROR_DEVICE_LOST,
	// WRITE_INVALID) within the first frames that run scene passes. Checked:
	// no device loss, every pixel against the shader's formula on the CPU, and
	// a D3D12 golden.
	TEST(RenderSystem.FrameGraph, Effect_VSM_Unshadowed_MultiFrame)
	{
		constexpr int w = kReferenceWidth, h = kReferenceHeight;
		const vec3 sun = vec3(0.3f, 0.8f, 0.5f).normalize();
		const SyntheticGBuffer gbuffer(w, h);

		auto sink = make_sink(w, h, 1, 1, HAL::Format::R16G16B16A16_FLOAT);

		FGT_VSMUnshadowedPipeline p;
		VSM vsm(p);
		p.fGT_ProduceGBuffer.render_func   = [&](auto& data, FrameContext& ctx) { gbuffer.upload(data, ctx); };
		p.fGT_CopyResultToSink.render_func = [](auto& data, FrameContext& ctx) {
			ctx.get_list()->get_copy().copy_texture(data.FGT_Sink.info->resource, 0, data.ResultTexture.info->resource, 0);
		};

		GraphRunner r;
		r.graph.get_context<Table::Shadows::VSM::VSMSelectors>().supported = false;
		r.graph.get_context<ViewportInfo>().frame_size   = { w, h };
		r.graph.get_context<ViewportInfo>().upscale_size = { w, h };
		r.graph.get_context<SkyInfo>().sunDir = sun;
		r.on_frame = add_frame_info;
		r.run_frames(p, sink, ivec2(w, h));

		auto result = read_sink(sink.get());
		auto& mip = result->array[0]->mips[0];
		for (int y = 0; y < h; ++y)
			for (int x = 0; x < w; ++x)
			{
				auto px = reinterpret_cast<const uint16_t*>(mip->data.data() + (size_t)y * mip->width_stride) + x * 4;
				const vec3 want = gbuffer.unshadowed(x, y, sun);
				const float got[3] = { half_to_float_normal(px[0]), half_to_float_normal(px[1]), half_to_float_normal(px[2]) };
				const float exp[3] = { want.x, want.y, want.z };
				for (int c = 0; c < 3; ++c)
					if (std::abs(got[c] - exp[c]) > 2e-3f + 2e-3f * exp[c])
						throw Test::TestFailure("pixel (" + std::to_string(x) + "," + std::to_string(y) + ") channel "
							+ std::to_string(c) + " is " + std::to_string(got[c]) + ", expected " + std::to_string(exp[c]));
			}

		Test::check_texture_data_reference(result, "fg_vsm_unshadowed");
	}

	// VSM_DepthAnalysis on async compute over a synthetic depth buffer (a far
	// ramp plus a nearer box inside the analysed central region), 16 frames.
	// Faulted the device on NVIDIA Vulkan with sparse depth on. Checked: no
	// device loss, and the measured minimum world-space texel size equals the
	// shader's formula evaluated on the CPU (vsm_depth_analysis.hlsl).
	TEST(RenderSystem.FrameGraph, Effect_VSM_DepthAnalysis_MultiFrame)
	{
		constexpr int w = kReferenceWidth, h = kReferenceHeight;
		SyntheticGBuffer gbuffer(w, h);
		for (int y = 0; y < h; ++y)
			for (int x = 0; x < w; ++x)
			{
				const bool box = x > w * 0.45f && x < w * 0.60f && y > h * 0.40f && y < h * 0.65f;
				gbuffer.depth[(size_t)y * w + x] = box ? 0.05f : 0.002f + 0.018f * y / h;   // reversed-Z: larger = nearer
			}

		camera cam;
		cam.position = vec3(0.0f, 5.0f, -10.0f);
		cam.target   = vec3(0.0f, 0.0f, 0.0f);
		cam.set_projection_params(Math::pi / 4, kReferenceAspect, 0.1f, 1000.0f);
		cam.update();

		// The shader's formula on the CPU: min over the central half of
		// 2 * |wpos - camera| * tan(fov/2) / height, wpos reconstructed from depth.
		float expected = std::numeric_limits<float>::max();
		const float tan_half_fov = std::tan(Math::pi / 8);
		for (int y = h / 4; y < h / 4 + h / 2; ++y)
			for (int x = w / 4; x < w / 4 + w / 2; ++x)
			{
				const float d = gbuffer.depth[(size_t)y * w + x];
				const vec2  tc((x + 0.5f) / w, (y + 0.5f) / h);
				vec4 p = vec4(tc.x * 2 - 1, -(tc.y * 2 - 1), d, 1) * cam.get_inv_view_proj();
				const vec3 wpos = vec3(p.x / p.w, p.y / p.w, p.z / p.w);
				expected = std::min(expected, 2.0f * (wpos - cam.position).length() * tan_half_fov / float(h));
			}

		auto sink = make_sink();   // unused: VSM_DepthAnalysis is [Required]
		FGT_VSMDepthAnalysisPipeline p;
		VSM vsm(p);
		p.fGT_ProduceGBuffer.render_func = [&](auto& data, FrameContext& ctx) { gbuffer.upload(data, ctx); };

		GraphRunner r;
		r.graph.get_context<Table::Shadows::VSM::VSMSelectors>().supported = true;
		r.graph.get_context<ViewportInfo>().frame_size   = { w, h };
		r.graph.get_context<ViewportInfo>().upscale_size = { w, h };
		r.on_frame = [&](Graph& g) {
			g.add_slot_generator([&](Graph& g) {
				Slots::Frame::FrameInfo fi;
				fi.GetCamera()         = cam.camera_cb.current;
				fi.GetPrevCamera()     = cam.camera_cb.current;
				fi.GetBrdf()           = EngineAssets::brdf.get_asset()->get_texture()->texture_3d().texture3D;
				fi.GetBestFitNormals() = EngineAssets::best_fit_normals.get_asset()->get_texture()->texture_2d().texture2D;
				g.register_slot_setter(fi.compile(*g.builder.current_frame));
			});
		};
		r.run_frames(p, sink, ivec2(w, h));

		const float measured = vsm.measured_texel_size();
		if (!(std::abs(measured - expected) <= 1e-3f * expected))
			throw Test::TestFailure("measured texel size " + std::to_string(measured)
				+ ", expected " + std::to_string(expected));
	}

}

namespace
{
	// Ground (plane.obj: 20x20 at y = 0) and the same plane scaled into a square
	// occluder above it. The GBuffer is ray cast on the CPU against the same two
	// squares, so it matches what VSM_RenderPages draws into the atlas.
	struct VSMShadowScene
	{
		static constexpr float ground_half = 10.0f, occluder_half = 1.5f, occluder_y = 2.0f;
		// Ground pixels this close (world units, on the occluder's plane) to the
		// shadow edge are penumbra and go unchecked.
		static constexpr float penumbra_margin = 0.6f;

		enum class Expect : uint8_t { None, Lit, Shadowed };

		std::shared_ptr<Scene> scene = std::make_shared<Scene>();
		std::vector<Expect>    expect;

		VSMShadowScene()
		{
			auto ground   = std::make_shared<MeshAssetInstance>(EngineAssets::plane.get_asset());
			auto occluder = std::make_shared<MeshAssetInstance>(EngineAssets::plane.get_asset());
			// Rotated pi about X, facing down: VSMDepthDraw culls front faces, so
			// the light has to see the occluder's back faces.
			mat4x4 m;
			m.identity();
			m.a11 = occluder_half / ground_half;
			m.a22 = -1.0f;
			m.a33 = -occluder_half / ground_half;
			m.a42 = occluder_y;
			occluder->set_local_transform(m);
			scene->add_child(ground);
			scene->add_child(occluder);
			scene->update_transforms();
		}

		// Flat-shaded GBuffer of the ground only, seen from `cam`: normals up, one
		// albedo; and for every pixel whether the sun is clearly visible or clearly
		// blocked by the occluder. The occluder still casts its shadow -- the
		// atlas is drawn from the scene meshes -- but stays out of the GBuffer:
		// its own surface self-shadows from its atlas depth, a noisy biased
		// region that differs across vendors and frame timing.
		void fill(SyntheticGBuffer& g, const camera& cam, vec3 sun)
		{
			expect.assign((size_t)g.w * g.h, Expect::None);
			const vec3 o = cam.position;
			for (int y = 0; y < g.h; ++y)
				for (int x = 0; x < g.w; ++x)
				{
					const size_t i = (size_t)y * g.w + x;
					vec4 p = vec4((x + 0.5f) / g.w * 2 - 1, -((y + 0.5f) / g.h * 2 - 1), 0.5f, 1) * cam.get_inv_view_proj();
					const vec3 d = (vec3(p.x / p.w, p.y / p.w, p.z / p.w) - o).normalize();

					static constexpr uint8_t albedo[4] = { 200, 180, 160, 0 };
					std::memcpy(&g.albedo[i * 4], albedo, 4);
					g.normals[i * 4 + 0] = 128; g.normals[i * 4 + 1] = 255; g.normals[i * 4 + 2] = 128; g.normals[i * 4 + 3] = 255;

					const float t = std::abs(d.y) < 1e-6f ? -1.0f : -o.y / d.y;
					const vec3  q = o + d * t;
					if (t <= 0 || std::abs(q.x) > ground_half || std::abs(q.z) > ground_half)
					{
						g.depth[i] = 0.0f;
						continue;
					}
					vec4 c = vec4(q.x, q.y, q.z, 1) * cam.get_view_proj();
					g.depth[i] = c.z / c.w;

					const vec3  r    = q + sun * ((occluder_y - q.y) / sun.y);
					const float edge = std::max(std::abs(r.x), std::abs(r.z)) - occluder_half;
					expect[i] = edge < -penumbra_margin ? Expect::Shadowed
					          : edge >  penumbra_margin ? Expect::Lit : Expect::None;
				}
		}
	};

	// The whole VSM chain over VSMShadowScene for 16 frames (sparse atlas page
	// rendering, Hi-Z, depth analysis, blocker classify/search, resolve),
	// failing on device loss. Then the last frame's result: clearly lit ground
	// is the unshadowed N.L * albedo, clearly shadowed ground is black.
	void run_vsm_full_chain(bool move_camera, const char* golden)
	{
		if (!RenderSystem::get().device().is_tiled_depth_supported())
			throw Test::TestSkipped("no sparse depth textures for the VSM atlas");

		constexpr int w = Test::kReferenceWidth, h = Test::kReferenceHeight;
		const vec3 sun = vec3(0.35f, 1.0f, 0.25f).normalize();
		SyntheticGBuffer gbuffer(w, h);
		VSMShadowScene world;

		camera cam;
		auto place_camera = [&](int frame) {
			const float a = move_camera ? 0.05f * frame : 0.0f;
			cam.position = vec3(-9.0f * std::sin(a), 7.0f, -9.0f * std::cos(a));
			cam.target   = vec3(0.0f, 0.0f, 0.0f);
			cam.set_projection_params(Math::pi / 4, Test::kReferenceAspect, 0.1f, 100.0f);
			cam.update();
		};

		auto sink = make_sink(w, h, 1, 1, HAL::Format::R16G16B16A16_FLOAT);
		FGT_VSMFullChainPipeline p;
		VSM        vsm(p);
		BlueNoise  blue_noise(p);
		vsm.attach_scene(world.scene);
		vsm.set_position(sun);
		p.fGT_ProduceGBuffer.render_func   = [&](auto& data, FrameContext& ctx) { gbuffer.upload(data, ctx); };
		p.fGT_CopyResultToSink.render_func = [](auto& data, FrameContext& ctx) {
			ctx.get_list()->get_copy().copy_texture(data.FGT_Sink.info->resource, 0, data.ResultTexture.info->resource, 0);
		};

		GraphRunner r;
		r.graph.get_context<ViewportInfo>().frame_size   = { w, h };
		r.graph.get_context<ViewportInfo>().upscale_size = { w, h };
		r.graph.get_context<SkyInfo>().sunDir            = sun;
		r.graph.get_context<SceneInfo>().scene           = world.scene;
		r.graph.get_context<CameraInfo>().cam            = &cam;
		int frame = 0;
		r.on_frame_start = [&](Graph& g) {
			place_camera(frame++);
			world.fill(gbuffer, cam, sun);
			world.scene->update(*g.builder.current_frame);
			if (RenderSystem::get().device().is_rtx_supported())
				RTX::get().update();
		};
		r.on_frame = [&](Graph& g) {
			vsm.update_frame(g);
			vsm.pass_data(g.builder);
			g.add_slot_generator([&](Graph& g) { vsm.plan_frame(g); });
			g.add_slot_generator([&](Graph& g) {
				Slots::Frame::FrameInfo fi;
				fi.GetSunDir().xyz     = sun;
				fi.GetCamera()         = cam.camera_cb.current;
				fi.GetPrevCamera()     = cam.camera_cb.prev;
				fi.GetBrdf()           = EngineAssets::brdf.get_asset()->get_texture()->texture_3d().texture3D;
				fi.GetBestFitNormals() = EngineAssets::best_fit_normals.get_asset()->get_texture()->texture_2d().texture2D;
				g.register_slot_setter(fi.compile(*g.builder.current_frame));
			});
			g.add_slot_generator([&](Graph& g) { g.register_slot_setter(world.scene->compiledScene); });
		};
		r.run_frames(p, sink, ivec2(w, h));

		auto result = read_sink(sink.get());
		auto& mip = result->array[0]->mips[0];
		size_t lit = 0, shadowed = 0, wrong_lit = 0, wrong_shadowed = 0;
		std::string first_wrong;
		for (int y = 0; y < h; ++y)
			for (int x = 0; x < w; ++x)
			{
				const auto e = world.expect[(size_t)y * w + x];
				if (e == VSMShadowScene::Expect::None) continue;
				auto px = reinterpret_cast<const uint16_t*>(mip->data.data() + (size_t)y * mip->width_stride) + x * 4;
				const float got[3] = { half_to_float_normal(px[0]), half_to_float_normal(px[1]), half_to_float_normal(px[2]) };
				const vec3  lit_value = gbuffer.unshadowed(x, y, sun);
				const float want[3] = { lit_value.x, lit_value.y, lit_value.z };
				bool ok = true;
				for (int c = 0; c < 3; ++c)
				{
					const float exp = e == VSMShadowScene::Expect::Lit ? want[c] : 0.0f;
					ok &= std::abs(got[c] - exp) <= 2e-3f + 2e-3f * exp;
				}
				(e == VSMShadowScene::Expect::Lit ? lit : shadowed)++;
				if (!ok)
				{
					(e == VSMShadowScene::Expect::Lit ? wrong_lit : wrong_shadowed)++;
					if (first_wrong.empty())
						first_wrong = "(" + std::to_string(x) + "," + std::to_string(y) + ") expected "
							+ (e == VSMShadowScene::Expect::Lit ? "lit" : "shadowed") + ", got "
							+ std::to_string(got[0]) + "," + std::to_string(got[1]) + "," + std::to_string(got[2]);
				}
			}

		if (lit == 0 || shadowed == 0)
			throw Test::TestFailure("scene has " + std::to_string(lit) + " lit and " + std::to_string(shadowed)
				+ " shadowed ground pixels to check; the camera misses the shadow");
		// A handful of pixels at the PCSS search footprint's edge may disagree.
		if (wrong_lit > lit / 1000 || wrong_shadowed > shadowed / 1000)
			throw Test::TestFailure(std::to_string(wrong_lit) + "/" + std::to_string(lit) + " lit and "
				+ std::to_string(wrong_shadowed) + "/" + std::to_string(shadowed) + " shadowed pixels wrong, first "
				+ first_wrong);

		if (golden)
			// The PCSS penumbra edge is noise-sampled; a sliver of its pixels lands
			// differently across vendors and as the depth-analysis feedback settles.
			Test::check_texture_data_reference(result, golden, { .tolerance = 6, .max_mismatch_fraction = 0.002 });
	}
}

export namespace Test
{
	// The VSM device loss on NVIDIA Vulkan showed within 5-8 frames of scene
	// passes: VSM_RenderPages/VSM_DepthAnalysis with sparse depth on.
	TEST(RenderSystem.FrameGraph, Effect_VSM_FullChain_MultiFrame)
	{
		run_vsm_full_chain(false, "fg_vsm_full_chain");
	}

	// Same, with the camera orbiting: pages are invalidated and re-rendered
	// every frame instead of staying cached.
	TEST(RenderSystem.FrameGraph, Effect_VSM_FullChain_MovingCamera)
	{
		run_vsm_full_chain(true, nullptr);
	}

	// VSM's tile lists are [Counted] buffers: the append counter sits at the
	// start, the elements at Device::counter_prefix(stride). A shader appends a
	// known tile set into a list declared the same way; the raw buffer is
	// copied out and checked: the counter equals the appended count, and the
	// elements after the prefix are exactly that set.
	TEST(RenderSystem.FrameGraph, VSM_TileLists_CounterLayout)
	{
		constexpr uint gx = 37, gy = 23;
		std::set<std::pair<uint, uint>> expected;
		for (uint y = 0; y < gy; ++y)
			for (uint x = 0; x < gx; ++x)
				if ((x * 7 + y * 3) % 5 == 0)   // fgt_append_tiles.hlsl
					expected.insert({ x, y });

		const uint64 prefix = RenderSystem::get().device().counter_prefix(sizeof(uint2));
		HAL::Resource::ptr result;
		uint64 bytes = 0;

		auto sink = make_sink();   // unused by this pipeline
		FGT_TileListPipeline p;
		p.fGT_AppendTiles.render_func = [&](auto& data, FrameContext& ctx) {
			auto& compute = ctx.get_list()->get_compute();
			compute.clear_counter(*data.FGT_TileList);
			Slots::FrameGraphTests::FGTAppendTiles tiles;
			tiles.GetGrid()  = uint2(gx, gy);
			tiles.GetTiles() = data.FGT_TileList->appendStructuredBuffer;
			compute.set(tiles);
			compute.set_pipeline<PSOS::FrameGraphTests::FGTAppendTiles>();
			compute.dispatch(ivec2(gx, gy), ivec2{ 8, 8 });
		};
		p.fGT_CopyTileList.render_func = [&](auto& data, FrameContext& ctx) {
			result = data.FGT_TileListCopy.info->resource;
			auto* list = data.FGT_TileList.info->resource.get();
			bytes = list->get_desc().as_buffer().SizeInBytes;
			ctx.get_list()->get_copy().copy_buffer(result.get(), 0, list, 0, bytes);
		};
		GraphRunner r;
		r.run(p, sink);
		ASSERT_TRUE(result != nullptr);
		ASSERT_TRUE(bytes >= prefix + expected.size() * sizeof(uint2));

		std::vector<std::byte> raw(bytes);
		auto list = RenderSystem::get().device().get_upload_list();
		auto done = list->get_copy().read_buffer(result.get(), 0, bytes, [&](std::span<std::byte> data) {
			std::memcpy(raw.data(), data.data(), std::min(data.size(), raw.size()));
		});
		list->execute_and_wait();
		done.wait();

		uint counter = 0;
		std::memcpy(&counter, raw.data(), sizeof(counter));
		ASSERT_EQ(uint(expected.size()), counter);

		std::set<std::pair<uint, uint>> appended;
		for (uint i = 0; i < counter; ++i)
		{
			uint tile[2];
			std::memcpy(tile, raw.data() + prefix + i * sizeof(uint2), sizeof(tile));
			if (!expected.contains({ tile[0], tile[1] }) || !appended.insert({ tile[0], tile[1] }).second)
				throw Test::TestFailure("element " + std::to_string(i) + " at byte " + std::to_string(prefix + i * sizeof(uint2))
					+ " is (" + std::to_string(tile[0]) + "," + std::to_string(tile[1]) + "), not an expected tile or a duplicate");
		}
	}
}
