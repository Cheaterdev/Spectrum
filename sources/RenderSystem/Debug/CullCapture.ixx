export module Graphics:CullCapture;

import Core;
import HAL;

// What the main view's GBuffer drew, recorded by the draws themselves (not a
// second cull pass): its GatherPipeline stamps every mesh it sends to a draw
// with (frame << 2) | stage. The debug view reads the stamps back to show
// rendered/culled geometry. Stamps from other frames read as "not drawn", so
// the buffer never needs clearing between frames.
//
// The buffer isn't a FrameGraph resource: readers are ordered after the writer
// only by their place in the pipeline listing, on the same queue.
export class CullCapture
{
public:
	using ptr = std::shared_ptr<CullCapture>;

	static constexpr uint max_meshes = 1024 * 1024;

	enum Stage : uint
	{
		// The occlusion culler's first stage, or the direct path when GPU
		// occlusion is off.
		First = 1,
		// Rescued by the occlusion culler's stage-2 retest.
		Retest = 2,
	};

	// Set by the owner (debug_view) before graph setup, read by the writer.
	bool capturing = false;
	// Stamp of the most recent captured frame; readers compare against it.
	// Stays put while capturing is off, which is what freezing means.
	uint frame = 0;

	HAL::virtual_gpu_buffer<uint>::ptr stamps;

	CullCapture();

	void next_frame();
	uint stamp(Stage stage) const { return (frame << 2) | stage; }

	// Maps the buffer's tiles and, the first time, zeroes them: freshly mapped
	// tiles hold whatever the heap last contained, which could alias a real
	// stamp. Call on the list about to write or read the stamps.
	void prepare(HAL::CommandList& list);

private:
	bool cleared = false;
};
