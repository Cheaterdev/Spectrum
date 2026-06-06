export module HAL:Autogen.Tables.Camera;

import Core;
import :SIG;
import :Types;
import :HLSL;
import :Enums;
import :Autogen.Tables.Frustum;
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
		static constexpr SIG_TYPE TYPE = SIG_TYPE::Table;
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

		static std::string get_typename()
		{
			return "Tables::Camera";
		}
		private:
		SERIALIZE()
		{
			ar& NVP(view);
			ar& NVP(proj);
			ar& NVP(viewProj);
			ar& NVP(invView);
			ar& NVP(invProj);
			ar& NVP(invViewProj);
			ar& NVP(reprojectionProj);
			ar& NVP(position);
			ar& NVP(direction);
			ar& NVP(jitter);
			ar& NVP(frustum);
		}

	};
	#pragma pack(pop)
}

