export module HAL:Autogen.Tables.GBuffer;
import Core;
import :SIG;
import :Types;
import :HLSL;
import :Enums;
export namespace Table 
{
	#pragma pack(push, 1)
	struct GBuffer
	{
		static constexpr SlotID ID = SlotID::GBuffer;
		HLSL::Texture2D<float4> albedo;
		HLSL::Texture2D<float4> normals;
		HLSL::Texture2D<float4> specular;
		HLSL::Texture2D<float2> motion;
		HLSL::Texture2D<float> depth;
		HLSL::Texture2D<float4>& GetAlbedo() { return albedo; }
		HLSL::Texture2D<float4>& GetNormals() { return normals; }
		HLSL::Texture2D<float4>& GetSpecular() { return specular; }
		HLSL::Texture2D<float2>& GetMotion() { return motion; }
		HLSL::Texture2D<float>& GetDepth() { return depth; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(albedo);
			compiler.compile(normals);
			compiler.compile(specular);
			compiler.compile(motion);
			compiler.compile(depth);
		}
		struct Compiled
		{
			uint albedo; // RenderTarget<float4>
			uint normals; // RenderTarget<float4>
			uint specular; // RenderTarget<float4>
			uint motion; // RenderTarget<float2>
			uint depth; // DepthStencil<float>
		};
	};
	#pragma pack(pop)
}
