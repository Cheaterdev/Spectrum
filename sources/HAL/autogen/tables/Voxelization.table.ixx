export module HAL:Autogen.Tables.Voxelization;
import Core;
import :SIG;
import :Types;
import :HLSL;
import :Autogen.Tables.VoxelInfo;
import :Enums;
export namespace Table 
{
	#pragma pack(push, 1)
	struct Voxelization
	{
		static constexpr SlotID ID = SlotID::Voxelization;
		HLSL::RWTexture3D<float4> albedo;
		HLSL::RWTexture3D<float4> normals;
		HLSL::RWTexture3D<uint> visibility;
		VoxelInfo info;
		HLSL::RWTexture3D<float4>& GetAlbedo() { return albedo; }
		HLSL::RWTexture3D<float4>& GetNormals() { return normals; }
		HLSL::RWTexture3D<uint>& GetVisibility() { return visibility; }
		VoxelInfo& GetInfo() { return info; }
		template<class Compiler>
		void compile(Compiler& compiler) const
		{
			compiler.compile(albedo);
			compiler.compile(normals);
			compiler.compile(visibility);
			compiler.compile(info);
		}
		struct Compiled
		{
			uint albedo; // RWTexture3D<float4>
			uint normals; // RWTexture3D<float4>
			uint visibility; // RWTexture3D<uint>
			VoxelInfo::Compiled info; // VoxelInfo
		};
	};
	#pragma pack(pop)
}
