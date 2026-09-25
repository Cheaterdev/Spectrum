export module Graphics:DebugDraw;

import Core;
import HAL;

// Immediate-mode schematic shapes (lines, boxes, frustums, ...) for debug views.
// Callable from any thread; everything submitted is drawn once, by the next
// debug_view render, then dropped -- resubmit every frame to keep a shape up.
// Matrices follow the CPU convention (row vectors: p * m).
export class DebugDraw : public Singleton<DebugDraw>
{
	friend class Singleton<DebugDraw>;

public:
	using Segment = Table::Dev::DebugLineSegment;

	static constexpr float default_thickness = 1.5f;

	void line(vec3 a, vec3 b, float4 color, float thickness = default_thickness);

	void box(vec3 min, vec3 max, float4 color, float thickness = default_thickness);
	void box(vec3 min, vec3 max, const mat4x4& transform, float4 color, float thickness = default_thickness);

	// Corners in camera::get_points() order: near plane 0..3, far plane 4..7,
	// each as (-x-y, +x-y, -x+y, +x+y).
	void frustum(const vec3 (&corners)[8], float4 color, float thickness = default_thickness);
	// The frustum an inverse view-projection maps NDC [-1,1]x[-1,1]x[0,1] from.
	void frustum(const mat4x4& inv_view_proj, float4 color, float thickness = default_thickness);

	void circle(vec3 center, vec3 normal, float radius, float4 color, float thickness = default_thickness, int segments = 48);
	void sphere(vec3 center, float radius, float4 color, float thickness = default_thickness, int segments = 48);
	void arrow(vec3 from, vec3 to, float4 color, float thickness = default_thickness);
	void axes(const mat4x4& transform, float size, float thickness = default_thickness);
	void grid(vec3 center, float cell_size, int half_count, float4 color, float thickness = 1.0f);

	// Everything submitted since the previous take().
	std::vector<Segment> take();

private:
	std::mutex m;
	std::vector<Segment> pending;
};
