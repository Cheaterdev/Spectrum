export module Test.HAL.Compute;

#define TEST_MODULE_ID Compute

export import Test.Framework;

import Core;
import HAL;
import RenderSystem;
import Graphics;

// Compute shaders against real GPU memory: the Append/counter path the VSM tile lists rely on, checked from
// the CPU. Written to find why those lists stay empty on some GPUs.

SETUP_CATEGORY(Core.Compute, []() {
	if (!GraphicsSystem::create())
		Test::TestRegistry::Instance().SkipCategory("Core.Compute", "no suitable GPU device found");
});

TEARDOWN_CATEGORY(Core.Compute, []() {
	GraphicsSystem::reset();
});

export namespace Test
{
	// A dispatch appends one item per thread to a counted buffer and writes one plain value per thread. The
	// counter is copied out the way VSM_BlockerClassify copies it into its indirect dispatch arguments.
	TEST(Core.Compute, AppendCounter)
	{
		auto& device = RenderSystem::get().device();
		constexpr uint N = 3000;

		HAL::StructuredBufferView<uint2> out(device, N, HAL::counterType::SELF,
			HAL::ResFlags::ShaderResource | HAL::ResFlags::UnorderedAccess);
		HAL::StructuredBufferView<uint> plain(device, N, HAL::counterType::NONE,
			HAL::ResFlags::ShaderResource | HAL::ResFlags::UnorderedAccess);
		HAL::StructuredBufferView<uint> atomic_buffer(device, 1, HAL::counterType::NONE,
			HAL::ResFlags::ShaderResource | HAL::ResFlags::UnorderedAccess);
		HAL::StructuredBufferView<uint> counter_copy(device, 1, HAL::counterType::NONE,
			HAL::ResFlags::ShaderResource | HAL::ResFlags::UnorderedAccess);

		auto& queue = device.get_queue(HAL::CommandListType::DIRECT);
		auto list = queue->get_free_list();
		list->begin(L"AppendCounter");

		auto& compute = list->get_compute();
		compute.set_signature(Layouts::DefaultLayout);

		// Everything starts zero: a never-written element can't pass for a real one.
		list->clear_uav(plain.rwRAW, vec4(0, 0, 0, 0));
		list->clear_uav(atomic_buffer.rwRAW, vec4(0, 0, 0, 0));
		compute.clear(out);
		compute.clear_counter(out);

		Slots::Tests::TestAppendData data;
		data.GetItems() = out.appendStructuredBuffer;
		data.GetPlain() = plain.rwStructuredBuffer;
		data.GetAtomic_counter() = atomic_buffer.rwStructuredBuffer;
		data.GetCount() = N;
		compute.set(data);

		compute.set_pipeline<PSOS::Tests::AppendCounter>();
		compute.dispatch(ivec2(N, 1), ivec2(64, 1));

		list->get_copy().copy_buffer(counter_copy.resource.get(), 0, out.get_counter_buffer().get(), out.get_counter_offset(), 4);

		std::vector<uint2> items;
		std::vector<uint> plain_values;
		uint counter_value = ~0u;
		uint counter_direct = ~0u;
		uint atomic_value = ~0u;
		std::vector<std::future<bool>> pending;
		// The elements start after the counter's room at the front of the buffer, and read() offsets from the buffer start.
		const uint prefix_elements = uint(device.counter_prefix(sizeof(uint2)) / sizeof(uint2));
		pending.push_back(list->get_copy().read<uint2>(out, 0, N + prefix_elements, [&](std::span<uint2> r) {
			items.assign(r.begin() + prefix_elements, r.end());
			counter_direct = reinterpret_cast<const uint*>(r.data())[0];   // the counter sits at the very start of the buffer
		}));
		pending.push_back(list->get_copy().read<uint>(plain, 0, N, [&](std::span<uint> r) { plain_values.assign(r.begin(), r.end()); }));
		pending.push_back(list->get_copy().read<uint>(atomic_buffer, 0, 1, [&](std::span<uint> r) { atomic_value = r[0]; }));
		pending.push_back(list->get_copy().read<uint>(counter_copy, 0, 1, [&](std::span<uint> r) { counter_value = r[0]; }));

		list->execute_and_wait();
		for (auto& p : pending)
			p.wait();

		uint plain_ok = 0;
		for (uint i = 0; i < plain_values.size(); ++i)
			plain_ok += plain_values[i] == i + 1;

		std::vector<bool> seen(N, false);
		uint valid = 0;
		for (auto& item : items)
		{
			if (item.x < N && item.y == item.x * 2 + 1 && !seen[item.x])
			{
				seen[item.x] = true;
				++valid;
			}
		}

		// All three facts in one message: which stage of the Append path fails tells where to look.
		if (plain_ok != N || atomic_value != N || counter_value != N || counter_direct != N || valid != N)
			throw Test::TestFailure("plain writes " + std::to_string(plain_ok) + "/" + std::to_string(N)
				+ ", plain atomic add " + std::to_string(atomic_value)
				+ ", Append counter " + std::to_string(counter_value) + " (read directly " + std::to_string(counter_direct) + ")"
				+ ", appended elements " + std::to_string(valid) + "/" + std::to_string(N) + " (expected " + std::to_string(N) + " each)");
	}
}
