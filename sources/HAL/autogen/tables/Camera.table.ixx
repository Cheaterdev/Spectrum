export module HAL:Autogen.Tables.Camera;
import Core;
import :SIG;
import :Types;
import :HLSL;
import <HAL.h>;
import :Autogen.Tables.Frustum;
import :Enums;
export namespace Table 
{
	#pragma pack(push, 1)
	struct Camera
	{
		static constexpr SlotID ID = SlotID::Camera;
		float4x4 view;
		float4x4 proj;
		float4x4 viewProj;
		float4x4 invView;
		float4x4 invProj;
		float4x4 invViewProj;
		float4x4 reprojectionProj;
		float4 position;
		float4 direction;
		float4 jitter;
		Frustum frustum;
		float4x4& GetView() { return view; }
		float4x4& GetProj() { return proj; }
		float4x4& GetViewProj() { return viewProj; }
		float4x4& GetInvView() { return invView; }
		float4x4& GetInvProj() { return invProj; }
		float4x4& GetInvViewProj() { return invViewProj; }
		float4x4& GetReprojectionProj() { return reprojectionProj; }
		float4& GetPosition() { return position; }
		float4& GetDirection() { return direction; }
		float4& GetJitter() { return jitter; }
		Frustum& GetFrustum() { return frustum; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(view);
			compiler.compile(proj);
			compiler.compile(viewProj);
			compiler.compile(invView);
			compiler.compile(invProj);
			compiler.compile(invViewProj);
			compiler.compile(reprojectionProj);
			compiler.compile(position);
			compiler.compile(direction);
			compiler.compile(jitter);
			compiler.compile(frustum);
		}
		using Compiled = Camera;
		};
		#pragma pack(pop)
	}
