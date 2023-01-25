#pragma once
namespace PSOS
{
	struct Voxelization: public PSOBase
	{
		struct Keys {
			KeyValue<NoValue,Nullable> Dynamic;

			GEN_DEF_COMP(Keys);
			private:
			SERIALIZE()
			{
				ar&NVP(Dynamic);
			}
		 };
		GEN_GRAPHICS_PSO(Voxelization,Dynamic)
		GEN_KEY(Dynamic,true);

		SimplePSO init_pso(Keys & key, std::function<void(SimplePSO&, Keys&)> f)
		{
			static const ShaderDefine<&Keys::Dynamic, &SimpleGraphicsPSO::pixel> Dynamic = "VOXEL_DYNAMIC";

			SimplePSO mpso("Voxelization");
			if(f) f(mpso,key);
			mpso.root_signature = Layouts::DefaultLayout;
			mpso.mesh.file_name = "shaders/mesh_shader_voxel.hlsl";
			mpso.mesh.entry_point = "VS";
			mpso.mesh.flags = 0;
			mpso.amplification.file_name = "shaders/mesh_shader_voxel.hlsl";
			mpso.amplification.entry_point = "AS";
			mpso.amplification.flags = 0;
			Dynamic.Apply(mpso, key);

			mpso.rtv_formats = {  };
			mpso.blend = {  };
			mpso.enable_depth  = false;
			mpso.enable_stencil  = false;
			mpso.cull  = HAL::CullMode::None;
			mpso.conservative  = true;
			return mpso;
		}
		private:
		SERIALIZE()
		{
			ar&NVP(wrap(psos));
		}
	};
}
