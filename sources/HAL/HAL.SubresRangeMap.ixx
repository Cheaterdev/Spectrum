export module HAL:SubresRangeMap;

import Core;
import :Types;
import :ResourceStates;

using namespace HAL;

export namespace HAL
{
	// Per-subresource state, stored as RANGES rather than one entry per
	// subresource.
	//
	// The barrier system's tracking is a map "which state is each subresource
	// in". Keyed per flat index that costs one entry per subresource, which for
	// a 2048-slice atlas or a 7x256 Hi-Z pyramid is thousands of entries and
	// thousands of individual barriers. But the things that produce those states
	// are VIEWS, and a view is always a rectangle in (mip x slice x plane)
	// space -- so the natural key is that rectangle.
	//
	// Invariants:
	//   - entries are pairwise DISJOINT (assign() subtracts before inserting)
	//   - `uniform`, when set, is the state of everything no entry covers
	//   - entries are always concrete; the whole-resource form lives in
	//     `uniform`, which is why the map needs the resource's bounds
	class SubresRangeMap
	{
	public:
		struct Entry
		{
			SubresRange   range;
			ResourceState state;
		};

	private:
		std::vector<Entry> entries;

		bool          has_uniform = false;
		ResourceState uniform;

		// Bounds of the resource this map describes. Needed to materialise the
		// whole-resource form into a concrete rectangle.
		uint mip_count   = 1;
		uint slice_count = 1;
		uint plane_count = 1;

		// [lo, hi) per axis, so the arithmetic below reads as interval maths
		// rather than "first + num" everywhere.
		struct Box
		{
			uint mip_lo = 0, mip_hi = 0;
			uint sl_lo  = 0, sl_hi  = 0;
			uint pl_lo  = 0, pl_hi  = 0;

			bool valid() const { return mip_lo < mip_hi && sl_lo < sl_hi && pl_lo < pl_hi; }
		};

		Box to_box(const SubresRange& r) const
		{
			if (r.is_all())
				return Box{ 0, mip_count, 0, slice_count, 0, plane_count };

			return Box{ r.first_mip,   r.first_mip   + r.num_mips,
			            r.first_slice, r.first_slice + r.num_slices,
			            r.first_plane, r.first_plane + r.num_planes };
		}

		SubresRange to_range(const Box& b) const
		{
			// A box covering everything reports as the whole-resource form, not
			// as a range that happens to span the bounds. D3D12 has a dedicated
			// sentinel for it, and losing that would turn every whole-resource
			// barrier into an explicit range -- correct, but off the fast path
			// and noisier in every debug view.
			if (b.mip_lo == 0 && b.mip_hi == mip_count
				&& b.sl_lo == 0 && b.sl_hi == slice_count
				&& b.pl_lo == 0 && b.pl_hi == plane_count)
				return SubresRange::all();

			return SubresRange{ b.mip_lo, b.mip_hi - b.mip_lo,
			                    b.sl_lo,  b.sl_hi  - b.sl_lo,
			                    b.pl_lo,  b.pl_hi  - b.pl_lo };
		}

		static bool overlaps(const Box& a, const Box& b)
		{
			return a.mip_lo < b.mip_hi && b.mip_lo < a.mip_hi
			    && a.sl_lo  < b.sl_hi  && b.sl_lo  < a.sl_hi
			    && a.pl_lo  < b.pl_hi  && b.pl_lo  < a.pl_hi;
		}

		static Box intersect(const Box& a, const Box& b)
		{
			return Box{ std::max(a.mip_lo, b.mip_lo), std::min(a.mip_hi, b.mip_hi),
			            std::max(a.sl_lo,  b.sl_lo),  std::min(a.sl_hi,  b.sl_hi),
			            std::max(a.pl_lo,  b.pl_lo),  std::min(a.pl_hi,  b.pl_hi) };
		}

		// a MINUS b, as up to six disjoint boxes.
		//
		// Split one axis at a time: the slabs of `a` outside `b` along the mip
		// axis come out whole, then the same for the slice axis within the
		// overlapping mip band, then the plane axis within both. Peeling in a
		// fixed order is what keeps the pieces disjoint.
		static void subtract(const Box& a, const Box& b, std::vector<Box>& out)
		{
			if (!overlaps(a, b)) { out.push_back(a); return; }

			if (a.mip_lo < b.mip_lo) out.push_back(Box{ a.mip_lo, b.mip_lo, a.sl_lo, a.sl_hi, a.pl_lo, a.pl_hi });
			if (b.mip_hi < a.mip_hi) out.push_back(Box{ b.mip_hi, a.mip_hi, a.sl_lo, a.sl_hi, a.pl_lo, a.pl_hi });

			const uint m_lo = std::max(a.mip_lo, b.mip_lo);
			const uint m_hi = std::min(a.mip_hi, b.mip_hi);

			if (a.sl_lo < b.sl_lo) out.push_back(Box{ m_lo, m_hi, a.sl_lo, b.sl_lo, a.pl_lo, a.pl_hi });
			if (b.sl_hi < a.sl_hi) out.push_back(Box{ m_lo, m_hi, b.sl_hi, a.sl_hi, a.pl_lo, a.pl_hi });

			const uint s_lo = std::max(a.sl_lo, b.sl_lo);
			const uint s_hi = std::min(a.sl_hi, b.sl_hi);

			if (a.pl_lo < b.pl_lo) out.push_back(Box{ m_lo, m_hi, s_lo, s_hi, a.pl_lo, b.pl_lo });
			if (b.pl_hi < a.pl_hi) out.push_back(Box{ m_lo, m_hi, s_lo, s_hi, b.pl_hi, a.pl_hi });
		}

	public:
		void reset(uint mips, uint slices, uint planes)
		{
			entries.clear();
			has_uniform = false;
			mip_count   = std::max(1u, mips);
			slice_count = std::max(1u, slices);
			plane_count = std::max(1u, planes);
		}

		void clear() { entries.clear(); has_uniform = false; }

		bool   empty() const { return entries.empty() && !has_uniform; }
		size_t size()  const { return entries.size() + (has_uniform ? 1 : 0); }

		bool                 uniform_set()   const { return has_uniform; }
		const ResourceState& uniform_state() const { return uniform; }

		const std::vector<Entry>& get_entries() const { return entries; }

		// True when the whole resource is in one state -- the common case, and
		// the one worth keeping cheap: a resource used as a whole never leaves
		// this shape.
		bool is_uniform() const { return has_uniform && entries.empty(); }

		void assign(const SubresRange& range, const ResourceState& state)
		{
			if (range.is_all())
			{
				entries.clear();
				has_uniform = true;
				uniform     = state;
				return;
			}

			const Box box = to_box(range);
			if (!box.valid()) return;

			// Carve the assigned box out of everything already stored, so the
			// entries stay disjoint and the newest write wins.
			std::vector<Entry> kept;
			kept.reserve(entries.size() + 4);

			std::vector<Box> pieces;
			for (auto& e : entries)
			{
				pieces.clear();
				subtract(to_box(e.range), box, pieces);
				for (auto& p : pieces)
					if (p.valid()) kept.push_back(Entry{ to_range(p), e.state });
			}

			kept.push_back(Entry{ to_range(box), state });
			entries.swap(kept);
		}

		// Report the state of every part of `query`.
		//
		// f(SubresRange piece, const ResourceState* state) -- state is null for
		// parts nothing has touched and no uniform covers, which is what the
		// barrier system treats as "assume where it rests".
		template<class F>
		void visit(const SubresRange& query, F&& f) const
		{
			const Box q = to_box(query);
			if (!q.valid()) return;

			// Leftover starts as the whole query and shrinks as entries claim
			// parts of it.
			std::vector<Box> leftover{ q };
			std::vector<Box> next;

			for (auto& e : entries)
			{
				const Box eb = to_box(e.range);
				if (!overlaps(q, eb)) continue;

				bool any = false;
				next.clear();
				for (auto& l : leftover)
				{
					if (!overlaps(l, eb)) { next.push_back(l); continue; }

					const Box hit = intersect(l, eb);
					if (hit.valid()) { f(to_range(hit), &e.state); any = true; }

					subtract(l, eb, next);
				}
				if (any) leftover.swap(next);
				else     leftover.swap(next);

				if (leftover.empty()) return;
			}

			for (auto& l : leftover)
				if (l.valid()) f(to_range(l), has_uniform ? &uniform : nullptr);
		}

		// Dev-only invariant: entries must never overlap. Every bug in a
		// structure like this shows up as double-counting a subresource, which
		// is silent until D3D12 rejects a barrier much later.
		bool check_disjoint() const
		{
			for (size_t i = 0; i < entries.size(); i++)
				for (size_t j = i + 1; j < entries.size(); j++)
					if (overlaps(to_box(entries[i].range), to_box(entries[j].range)))
						return false;
			return true;
		}
	};
}
