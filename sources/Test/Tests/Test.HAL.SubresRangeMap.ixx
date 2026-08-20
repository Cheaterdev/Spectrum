export module Test.HAL.SubresRangeMap;

#define TEST_MODULE_ID SubresRangeMap

export import Test.Framework;

import stl.core;
import HAL;

// SubresRangeMap stores per-subresource state as RECTANGLES in
// (mip x slice x plane) space, which means every read and write goes through
// 3D rectangle subtraction. That is silent when wrong -- a dropped or
// double-counted piece only surfaces much later as a D3D12 barrier with an
// impossible before-state.
//
// So every test here checks the range map against a BRUTE-FORCE reference that
// stores one state per flat subresource. The two must agree for every
// subresource, always. The reference is obviously correct and obviously slow;
// the range map is neither.
export namespace Test
{
	using HAL::SubresRange;
	using HAL::SubresRangeMap;
	using HAL::ResourceState;

	namespace detail
	{
		// Distinct, easily-compared states. Only identity matters here.
		ResourceState st(unsigned id)
		{
			return ResourceState{ HAL::BarrierSync::ALL, HAL::BarrierAccess::SHADER_RESOURCE,
			                      static_cast<HAL::TextureLayout>(1u << (id % 8)) };
		}

		struct Reference
		{
			unsigned mips, slices, planes;
			std::vector<int> state_id;   // -1 == untouched

			Reference(unsigned m, unsigned s, unsigned p) : mips(m), slices(s), planes(p),
				state_id(size_t(m) * s * p, -1) {}

			size_t index(unsigned mip, unsigned sl, unsigned pl) const
			{
				return (size_t(pl) * slices + sl) * mips + mip;
			}

			void assign(const SubresRange& r, int id)
			{
				const unsigned m0 = r.is_all() ? 0 : r.first_mip;
				const unsigned m1 = r.is_all() ? mips : r.first_mip + r.num_mips;
				const unsigned s0 = r.is_all() ? 0 : r.first_slice;
				const unsigned s1 = r.is_all() ? slices : r.first_slice + r.num_slices;
				const unsigned p0 = r.is_all() ? 0 : r.first_plane;
				const unsigned p1 = r.is_all() ? planes : r.first_plane + r.num_planes;

				for (unsigned p = p0; p < p1; p++)
					for (unsigned s = s0; s < s1; s++)
						for (unsigned m = m0; m < m1; m++)
							state_id[index(m, s, p)] = id;
			}
		};

		// Read every subresource back out of the map and compare to the
		// reference. Also verifies each subresource is reported EXACTLY once --
		// the failure mode disjointness bugs produce.
		bool agrees(const SubresRangeMap& map, const Reference& ref,
		            const std::vector<ResourceState>& states)
		{
			std::vector<int> seen(ref.state_id.size(), -2);
			std::vector<int> hits(ref.state_id.size(), 0);

			map.visit(SubresRange::all(), [&](SubresRange piece, const ResourceState* s)
			{
				// A piece covering the whole resource is reported as the
				// whole-resource form, whose first/num fields are the sentinel
				// rather than bounds -- expand it against the resource instead.
				const unsigned p0 = piece.is_all() ? 0 : piece.first_plane;
				const unsigned p1 = piece.is_all() ? ref.planes : piece.first_plane + piece.num_planes;
				const unsigned s0 = piece.is_all() ? 0 : piece.first_slice;
				const unsigned s1 = piece.is_all() ? ref.slices : piece.first_slice + piece.num_slices;
				const unsigned m0 = piece.is_all() ? 0 : piece.first_mip;
				const unsigned m1 = piece.is_all() ? ref.mips : piece.first_mip + piece.num_mips;

				for (unsigned p = p0; p < p1; p++)
					for (unsigned sl = s0; sl < s1; sl++)
						for (unsigned m = m0; m < m1; m++)
						{
							const size_t i = ref.index(m, sl, p);
							if (i >= seen.size()) return;
							hits[i]++;

							int id = -1;
							if (s)
								for (unsigned k = 0; k < states.size(); k++)
									if (states[k] == *s) { id = int(k); break; }
							seen[i] = id;
						}
			});

			for (size_t i = 0; i < seen.size(); i++)
			{
				if (hits[i] != 1)             return false;   // missing or double-reported
				if (seen[i] != ref.state_id[i]) return false; // wrong state
			}
			return true;
		}
	}

	using namespace detail;

	TEST(SubresRangeMap, UniformStaysOneEntry)
	{
		SubresRangeMap map;
		map.reset(7, 256, 1);

		map.assign(SubresRange::all(), st(0));

		ASSERT_TRUE(map.is_uniform());
		ASSERT_EQ(map.size(), size_t(1));
	}

	TEST(SubresRangeMap, WholeMipAcrossSlicesIsOneEntry)
	{
		SubresRangeMap map;
		map.reset(7, 256, 1);

		// The shape this whole structure exists for: one mip, every slice.
		map.assign(SubresRange{ 3, 1, 0, 256, 0, 1 }, st(1));

		ASSERT_EQ(map.get_entries().size(), size_t(1));
		ASSERT_TRUE(map.check_disjoint());
	}

	TEST(SubresRangeMap, OverlappingAssignsStayDisjoint)
	{
		SubresRangeMap map;
		map.reset(4, 8, 1);

		map.assign(SubresRange::all(),               st(0));
		map.assign(SubresRange{ 1, 2, 2, 4, 0, 1 },  st(1));
		map.assign(SubresRange{ 0, 3, 3, 2, 0, 1 },  st(2));   // straddles the previous
		map.assign(SubresRange{ 2, 1, 0, 8, 0, 1 },  st(3));

		ASSERT_TRUE(map.check_disjoint());
	}

	TEST(SubresRangeMap, MatchesReferenceOnOverlappingWrites)
	{
		const unsigned MIPS = 4, SLICES = 8, PLANES = 1;

		SubresRangeMap map;
		map.reset(MIPS, SLICES, PLANES);
		Reference ref(MIPS, SLICES, PLANES);

		std::vector<ResourceState> states;
		for (unsigned i = 0; i < 6; i++) states.push_back(st(i));

		const SubresRange writes[] = {
			SubresRange::all(),
			SubresRange{ 1, 2, 2, 4, 0, 1 },
			SubresRange{ 0, 1, 0, 8, 0, 1 },   // whole mip 0
			SubresRange{ 3, 1, 7, 1, 0, 1 },   // single subresource
			SubresRange{ 0, 4, 3, 1, 0, 1 },   // whole slice 3
		};

		for (unsigned i = 0; i < std::size(writes); i++)
		{
			map.assign(writes[i], states[i]);
			ref.assign(writes[i], int(i));

			ASSERT_TRUE(map.check_disjoint());
			ASSERT_TRUE(agrees(map, ref, states));
		}
	}

	TEST(SubresRangeMap, MatchesReferenceUnderRandomWrites)
	{
		const unsigned MIPS = 5, SLICES = 6, PLANES = 2;

		SubresRangeMap map;
		map.reset(MIPS, SLICES, PLANES);
		Reference ref(MIPS, SLICES, PLANES);

		std::vector<ResourceState> states;
		for (unsigned i = 0; i < 8; i++) states.push_back(st(i));

		// Deterministic pseudo-random: a fixed sequence so a failure is
		// reproducible rather than a once-seen flake.
		unsigned seed = 12345;
		auto rnd = [&](unsigned n) { seed = seed * 1664525u + 1013904223u; return (seed >> 16) % n; };

		for (unsigned iter = 0; iter < 200; iter++)
		{
			const unsigned m0 = rnd(MIPS),   m1 = m0 + 1 + rnd(MIPS - m0);
			const unsigned s0 = rnd(SLICES), s1 = s0 + 1 + rnd(SLICES - s0);
			const unsigned p0 = rnd(PLANES), p1 = p0 + 1 + rnd(PLANES - p0);

			const SubresRange r{ m0, m1 - m0, s0, s1 - s0, p0, p1 - p0 };
			const unsigned id = rnd(unsigned(states.size()));

			map.assign(r, states[id]);
			ref.assign(r, int(id));

			ASSERT_TRUE(map.check_disjoint());
			ASSERT_TRUE(agrees(map, ref, states));
		}
	}

	TEST(SubresRangeMap, UntouchedReportsNoState)
	{
		SubresRangeMap map;
		map.reset(2, 2, 1);

		map.assign(SubresRange{ 0, 1, 0, 1, 0, 1 }, st(1));

		unsigned with_state = 0, without_state = 0;
		map.visit(SubresRange::all(), [&](SubresRange piece, const HAL::ResourceState* s)
		{
			const unsigned n = piece.is_all() ? 2u * 2u * 1u
			                                 : piece.num_mips * piece.num_slices * piece.num_planes;
			(s ? with_state : without_state) += n;
		});

		// One subresource written, three never touched and no uniform set.
		ASSERT_EQ(with_state,    unsigned(1));
		ASSERT_EQ(without_state, unsigned(3));
	}
}
