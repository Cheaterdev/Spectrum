#pragma once
namespace RT 
{
	#pragma pack(push, 1)
	struct _GBuffer
	{
		HLSL::RenderTarget<float4> albedo;
		HLSL::RenderTarget<float4> normals;
		HLSL::RenderTarget<float4> specular;
		HLSL::RenderTarget<float2> motion;
		HLSL::DepthStencil<float> depth;
		HLSL::RenderTarget<float4>& GetAlbedo() { return albedo; }
		HLSL::RenderTarget<float4>& GetNormals() { return normals; }
		HLSL::RenderTarget<float4>& GetSpecular() { return specular; }
		HLSL::RenderTarget<float2>& GetMotion() { return motion; }
		HLSL::DepthStencil<float>& GetDepth() { return depth; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(albedo);
			compiler.compile(normals);
			compiler.compile(specular);
			compiler.compile(motion);
			compiler.compile(depth);
		}
	};
	#pragma pack(pop)
	struct GBuffer:public RTHolder<_GBuffer>
	{
		GBuffer() = default;
	};
}
