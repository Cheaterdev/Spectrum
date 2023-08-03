export module HAL:Autogen.Tables.FrameInfo;
import Core;
import :SIG;
import :Types;
import :HLSL;
import <HAL.h>;
import :Autogen.Tables.Camera;
import :Enums;
export namespace Table 
{
	#pragma pack(push, 1)
	struct FrameInfo
	{
		static constexpr SlotID ID = SlotID::FrameInfo;
		float4 time;
		float4 sunDir;
		HLSL::Texture2D<float4> bestFitNormals;
		HLSL::Texture3D<float4> brdf;
		HLSL::TextureCube<float4> sky;
		Camera camera;
		Camera prevCamera;
		float4& GetTime() { return time; }
		HLSL::Texture2D<float4>& GetBestFitNormals() { return bestFitNormals; }
		HLSL::Texture3D<float4>& GetBrdf() { return brdf; }
		HLSL::TextureCube<float4>& GetSky() { return sky; }
		float4& GetSunDir() { return sunDir; }
		Camera& GetCamera() { return camera; }
		Camera& GetPrevCamera() { return prevCamera; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(time);
			compiler.compile(sunDir);
			compiler.compile(bestFitNormals);
			compiler.compile(brdf);
			compiler.compile(sky);
			compiler.compile(camera);
			compiler.compile(prevCamera);
		}
		struct Compiled
		{
			float4 time; // float4
			float4 sunDir; // float4
			uint bestFitNormals; // Texture2D<float4>
			uint brdf; // Texture3D<float4>
			uint sky; // TextureCube<float4>
			Camera::Compiled camera; // Camera
			Camera::Compiled prevCamera; // Camera
		};
	};
	#pragma pack(pop)
}
