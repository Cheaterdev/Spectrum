// Android stand-in for HAL:Streamline / HAL:DLSS / HAL:DLSSRR / HAL:NRD. Those wrap NVIDIA's
// Windows-only SDKs; here the same names exist so the engine's callers compile, and every
// query answers "not available". The real partitions are excluded by the Android build.
export module HAL:NvidiaStubs;

import Core;
import :ResourceViews;
import :Device;
import :CommandList;

export namespace nvidia
{
	enum class DLSSMode
	{
		Off,
		MaxPerformance,
		Balanced,
		MaxQuality,
		UltraQuality,
		UltraPerformance,
		DLAA,
	};

	struct DLSSOptimalSettings
	{
		ivec2 render_size;
		ivec2 render_size_min;
		ivec2 render_size_max;
	};

	struct FrameConstants {};

	class Streamline : public Singleton<Streamline>
	{
		friend class Singleton<Streamline>;
		Streamline() = default;
	public:
		bool available() const { return false; }
		void begin_frame() const {}
	};

	class DLSS : public Singleton<DLSS>
	{
		friend class Singleton<DLSS>;
		DLSS() = default;
	public:
		bool available() const { return false; }
		bool get_optimal_settings(DLSSMode, ivec2, bool, DLSSOptimalSettings&) const { return false; }
	};

	class DLSSRR : public Singleton<DLSSRR>
	{
		friend class Singleton<DLSSRR>;
		DLSSRR() = default;
	public:
		bool available() const { return false; }
	};

	struct NRDFrameInputs
	{
		HAL::Texture2DView view_z;
		HAL::Texture2DView normal_roughness;
		HAL::Texture2DView mv;
		HAL::Texture2DView diff_noisy;
		HAL::Texture2DView diff_denoised;
		HAL::Texture2DView spec_noisy;
		HAL::Texture2DView spec_denoised;

		HAL::Texture2DView penumbra_noisy;
		HAL::Texture2DView shadow_denoised;

		float world_to_view[16];
		float world_to_view_prev[16];
		float view_to_clip[16];
		float view_to_clip_prev[16];
		float2 jitter;
		float2 jitter_prev;

		float3 sun_direction;

		uint64 frame = 0;
	};

	class NRD : public Singleton<NRD>
	{
		friend class Singleton<NRD>;
		NRD() = default;
	public:
		template<class Size>
		void ensure_pools(HAL::Device&, const Size&) {}
		void execute_reblur(HAL::CommandList&, const NRDFrameInputs&) {}
		void execute_shadow(HAL::CommandList&, const NRDFrameInputs&) {}
	};
}
