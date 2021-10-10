#include "pch_render.h"

#include <assimp\Importer.hpp>
#include <assimp\importerdesc.h>
#include <assimp\postprocess.h>
#include <assimp\scene.h>
#include <assimp\DefaultLogger.hpp>
#include <assimp\LogStream.hpp>
#include <assimp\ProgressHandler.hpp>
#include <assimp\IOStream.hpp>

#include "raw4/Raw4Loader.h"

#include "BaseImporter.h"

namespace Assimp
{


	static const aiImporterDesc desc = {
		"RAW4 Loader",
		"",
		"",
		"",
		aiImporterFlags_SupportBinaryFlavour,
		0,
		0,
		0,
		0,
		"rw4"
	};
	class RAW4Importer : public BaseImporter, public Raw4SceneAdaptor
	{

		aiScene* pScene;
		aiNode* mCurNode;
	//	aiNode* lastCreated;
	//	aiNode* lastCreatedOffsetNode;

		struct node
		{
			aiMatrix4x4 local;
			aiMatrix4x4 world;
			aiMatrix4x4 offset;
		};
		std::list<node> matrix_stack;


		struct scene_material{

			std::string albedo;

		};

		struct Mesh
		{
			std::vector<int> index_data;
			std::vector<vec3> poses;
			std::vector<vec3> normals;
			std::vector<vec3> tcs;
			int mat_id;
		};
		 
		std::vector<Mesh> scene_meshes;
		std::map<int, scene_material > scene_materials;
		bool CanRead(const std::string& pFile,IOSystem* pIOHandler, bool checkSig) const override
		{
			const std::string extension = GetExtension(pFile);

			if (extension == "rw4") return true;


			return false;
		}

		virtual const aiImporterDesc* GetInfo() const override

		{
			return &desc;
		}

		virtual void InternReadFile(
			const std::string& pFile,
			aiScene* pScene,
			IOSystem* pIOHandler
		) override
		{
			this->pScene = pScene;

			if (!pScene->mRootNode)
				pScene->mRootNode = new aiNode();

			aiMatrix4x4::RotationX(-::Math::m_pi_2, pScene->mRootNode->mTransformation);
				mCurNode = pScene->mRootNode;

			//	mCurNode->mMeshes = new unsigned int[1024 * 32];
			//	mCurNode->mNumMeshes = 0;

			Raw4Loader::Load(*pIOHandler, pFile, *this);



			pScene->mMaterials = new aiMaterial*[scene_materials.size()];
			pScene->mNumMaterials = (UINT)scene_materials.size();
			for (auto it : scene_materials)
			{
				auto mat = pScene->mMaterials[it.first] = new aiMaterial();
				auto rw4_mat = it.second;
				//set the name of the material:

				aiString str = aiString(rw4_mat.albedo.c_str());

				mat->AddProperty(&str, AI_MATKEY_NAME);//MaterialName is a std::string																						  //set the first diffuse texture
				mat->AddProperty(&str, AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0));//again, Texturename is a std::string
			}



			pScene->mMeshes = new aiMesh*[scene_meshes.size()];
			pScene->mNumMeshes = (UINT)scene_meshes.size();
			for (size_t i = 0; i < scene_meshes.size(); i++)
			{
				auto mesh = pScene->mMeshes[i] = new aiMesh();
				auto rw4_mesh = scene_meshes[i];

				mesh->mName = "name";
				mesh->mVertices = new aiVector3D[rw4_mesh.poses.size()];
				mesh->mNumVertices = (UINT)rw4_mesh.poses.size();
				mesh->mMaterialIndex = rw4_mesh.mat_id;
				memcpy(mesh->mVertices, rw4_mesh.poses.data(), rw4_mesh.poses.size()*sizeof(vec3));


				if (rw4_mesh.normals.size()) {
					mesh->mNormals = new aiVector3D[rw4_mesh.normals.size()];
					memcpy(mesh->mNormals, rw4_mesh.normals.data(), rw4_mesh.normals.size() * sizeof(vec3));
				}

				if (rw4_mesh.tcs.size()) {
					mesh->mNumUVComponents[0] = 2;
					
					mesh->mTextureCoords[0] = new aiVector3D[rw4_mesh.tcs.size()];
						
				memcpy(mesh->mTextureCoords[0], rw4_mesh.tcs.data(), rw4_mesh.tcs.size() * sizeof(vec3));
				}



				mesh->mNumFaces = (UINT)(rw4_mesh.index_data.size() / 3);

				mesh->mFaces = new aiFace[mesh->mNumFaces];

				for (UINT i = 0; i < mesh->mNumFaces; i++)

				{
					auto &face = mesh->mFaces[i];

					face.mNumIndices = 3;
					face.mIndices = new  unsigned int[3];

					face.mIndices[0] = rw4_mesh.index_data[i * 3];
					face.mIndices[1] = rw4_mesh.index_data[i * 3+1]; 
					face.mIndices[2] = rw4_mesh.index_data[i * 3+2];
				}
			}
			this->pScene = nullptr;

		}



	public:
#define ROT_ORDER_XYZ	0
#define ROT_ORDER_XZY	1
#define ROT_ORDER_YZX	2
#define ROT_ORDER_YXZ	3
#define ROT_ORDER_ZXY	4
#define ROT_ORDER_ZYX	5
#define ROT_ORDER_XYX	6
#define ROT_ORDER_YZY	7
#define ROT_ORDER_ZXZ	8

		virtual void create_object(const Raw4Context &context, const raw4_block_object &obj, const raw4_block_values* values) override
		{
		//	throw std::logic_error("The method or operation is not implemented.");
		//	auto child = new aiNode();
		//	child->mName = context.GetString(obj.name);
		//	for (int i = 0; i < 3; i++)
		//		for (int j = 0; j < 4; j++)
		//		(child->mTransformation[i][j])=obj.lparent.rows[j][i];
			aiMatrix4x4 & local= matrix_stack.back().local;
			aiMatrix4x4  & offset = matrix_stack.back().offset;

			aiMatrix4x4 mpos, rotx,roty,rotz;
		
			aiMatrix4x4::Scaling(aiVector3D(obj.scale, obj.scale, obj.scale), local);
			//child->mTransformation = mpos;



			aiMatrix4x4::RotationX(obj.rotation.x, rotx);
			aiMatrix4x4::RotationY(obj.rotation.y, roty);
			aiMatrix4x4::RotationZ(obj.rotation.z, rotz);


			if (obj.rot_order == ROT_ORDER_XYZ)
			{
				local = (rotx)*local;
				local = (roty)*local;
				local = (rotz)*local;
			}


			//
			//*roty*rotz;
			if (obj.rot_order == ROT_ORDER_XZY)
				local *= rotx*rotz*roty;

			if (obj.rot_order == ROT_ORDER_YZX)
				local *= roty*rotz*rotx;

			if (obj.rot_order == ROT_ORDER_YXZ)
				local *= roty*rotx*rotz;


			if (obj.rot_order == ROT_ORDER_ZXY)
				local *= rotz*rotx*roty;


			if (obj.rot_order == ROT_ORDER_ZYX)
				local *= rotz*roty*rotx;


			if (obj.rot_order == ROT_ORDER_XYX)
				local *= rotx*roty*rotx;


			if (obj.rot_order == ROT_ORDER_YZY)
				local *= roty*rotz*roty;

			if (obj.rot_order == ROT_ORDER_ZXZ)
				local *= rotz*rotx*rotz;


			//child->mTransformation *= mpos;



			aiMatrix4x4::Translation(aiVector3D(obj.position.x, obj.position.y, obj.position.z), mpos);
			local = mpos*local;


			//child->mTransformation.Transpose();

		//	lastCreated = child;
		//	lastCreated->mMeshes = new unsigned int[1024 * 32];
		//	lastCreated->mNumMeshes = 0;
		//	mCurNode->addChildren(1, &child);
			//lastCreatedOffsetNode = lastCreated;
			//assert(obj.scale==1);
			

			{
			//	auto child = new aiNode();
			//	child->mName = "offset";

			//	aiMatrix4x4 mpos;
				aiMatrix4x4::RotationX(obj.offset_rot.x, rotx);
				aiMatrix4x4::RotationY(obj.offset_rot.y, roty);
				aiMatrix4x4::RotationZ(obj.offset_rot.z, rotz);


				offset = (rotx);
				offset = (roty)*offset;
				offset = (rotz)*offset;
				//mpos.FromEulerAnglesXYZ(obj.offset_rot.x, obj.offset_rot.y, obj.offset_rot.z);
				//child->mTransformation = mpos;
			

				aiMatrix4x4::Translation(aiVector3D(obj.offset_pos.x, obj.offset_pos.y , obj.offset_pos.z ), mpos);
				offset = mpos*offset;

		//		child->mTransformation.Transpose();
			
			}

			aiMatrix4x4& world = matrix_stack.back().world;


			auto get_matrix = [](int tm_flags, aiMatrix4x4 parent, aiMatrix4x4 my) ->aiMatrix4x4 {
			
#define TM_FLAG_INHERIT_PX		0x00000080
#define TM_FLAG_INHERIT_PY		0x00000100
#define TM_FLAG_INHERIT_PZ		0x00000200
#define TM_FLAG_INHERIT_RX		0x00000400
#define TM_FLAG_INHERIT_RY		0x00000800
#define TM_FLAG_INHERIT_RZ		0x00001000
#define TM_FLAG_INHERIT_S		0x00002000
#define TM_FLAG_INHERIT_POS		0x00000380		// PX + PY + PZ
#define TM_FLAG_INHERIT_ROT		0x00001C00		// RX + RY + RZ
#define TM_FLAG_INHERIT_RS		0x00003C00		// RX + RY + RZ + S


#define TM_FLAG_INHERIT_ALL		0x00003F80		// PX + PY + PZ + RX + RY + RZ + S
				if ((tm_flags & TM_FLAG_INHERIT_ALL) == TM_FLAG_INHERIT_ALL) return parent;

				if (tm_flags & TM_FLAG_INHERIT_PX) my.a4 = parent.a4;
				if (tm_flags & TM_FLAG_INHERIT_PY) my.b4 = parent.b4;
				if (tm_flags & TM_FLAG_INHERIT_PZ) my.c4 = parent.c4;
				// rotation+scale
				if ((tm_flags & TM_FLAG_INHERIT_RS) == 0) return my;


				return my;

			};

			aiMatrix4x4 my;
				for (uint32_t i = 0; i < 3; i++)
					for (uint32_t j = 0; j < 4; j++)
						my[i][j]=obj.lparent.rows[j][i];

				auto prev = matrix_stack.end();
			
				aiMatrix4x4 prev_w;
				if (matrix_stack.size() > 1) {
					std::advance(prev, -2);
					prev_w = prev->world;
				}
			world = get_matrix(obj.flags, prev_w, my);

			world = world*local;


		}


		virtual void create_model(const Raw4Context &context, const raw4_block_model &model, const raw4_block_values* values)
		{
			std::string name = context.GetString(model.name);

			create_object(context, model, values);


			bool visible = true;
			if (name.find("d_") == 0)
				visible = false;
			if (name.find("b_") == 0)
				visible = false;
				auto child = new aiNode();
				child->mName = name;
				child->mMeshes = new unsigned int[1];
				child->mNumMeshes = 0;
				child->mTransformation =/* matrix_stack.back().offset* */matrix_stack.back().world*matrix_stack.back().offset;
				mCurNode->addChildren(1, &child);
			auto lod = model.lods[0];
			auto mesh = context.GetMesh(lod.mesh);
			auto material = context.GetMaterial(lod.material);
			auto mat_it = scene_materials.find(lod.material);
		
			//auto &mat = materials[lod.material];
			if (mat_it == scene_materials.end())
			{
				auto &mat = scene_materials[lod.material];

				mat.albedo = context.GetString(context.GetTexture(material->diffuse_texture)->name);
			}
			
			{
				raw4_vertex_array* arrays = (raw4_vertex_array*)((uint8_t*)mesh + mesh->arrays_offset);



				//	assert(mesh->render_type== RAW4_RENDER_TRIANGLES);
				raw4_buffer* ib = context.GetBuffer(mesh->indices);
				uint16_t* ics = (uint16_t*)((uint8_t*)ib + ib->offset);

				raw4_buffer* vb_pos = context.GetBuffer(arrays[0].vertices);
				uint8_t* vpos = ((uint8_t*)vb_pos + vb_pos->offset);



				scene_meshes.emplace_back();

				Mesh& new_mesh = scene_meshes.back();

				child->mMeshes[child->mNumMeshes++] = (UINT)(scene_meshes.size() - 1);

				new_mesh.mat_id = lod.material;


				bool fix = false;

				if (arrays->attrs[0].offset != 0) fix = true;
				uint32_t stride = arrays->attrs[0].stride;

				for (uint32_t i = 0; i < arrays->attributes_count; i++)
				{
					auto &a = arrays->attrs[i];
					if (a.stride != stride || a.stride == 0)
					{
						fix = true;
						break;
					}
				}


				if (fix)
				{
				
				stride = 0;// (arrays->attrs[1].format == RAW4_FORMAT_F32) ? 32 : 24;
				for (uint32_t i = 0; i < arrays->attributes_count; i++)
					stride += arrays->attrs[i].count*RAW4_FORMAT_SIZE[arrays->attrs[i].format];
				}

				//stride /= 4;
				
				uint32_t verts_count = vb_pos->size /stride;
				
				if (mesh->index_format == RAW4_FORMAT_U8)
					for (uint32_t i = 0; i < mesh->index_count; i++)
						new_mesh.index_data.emplace_back(((char*)ics)[i]);

				if (mesh->index_format == RAW4_FORMAT_U16&&mesh->render_type == RAW4_RENDER_TRIANGLES)
				{
					bool clocked = (model.render_opts&RAW4_RENDER_OPTS_FRONT_CW)==0;
					if(clocked)
					for (uint32_t i = 0; i < mesh->index_count; i++)
						new_mesh.index_data.emplace_back(ics[i]);
					else
						for (uint32_t i = 0; i < mesh->index_count; i += 3)
						{
							new_mesh.index_data.emplace_back(ics[i]);
							new_mesh.index_data.emplace_back(ics[i+2]);
							new_mesh.index_data.emplace_back(ics[i+1]);
						}
				}
					

				if (mesh->index_format == RAW4_FORMAT_U16&&mesh->render_type == RAW4_RENDER_TRIANGLE_STRIP)
				{
					
					bool clocked = model.render_opts&RAW4_RENDER_OPTS_FRONT_CW;

					for (uint32_t i = 0; i < mesh->index_count; i++)
					{
						if (clocked)
						{
							if (i > 0)
								new_mesh.index_data.emplace_back(ics[i - 1]);
							if (i > 1)
								new_mesh.index_data.emplace_back(ics[i - 2]);
						}
						else

						{
							if (i > 1)
								new_mesh.index_data.emplace_back(ics[i - 2]);

							if (i > 0)
								new_mesh.index_data.emplace_back(ics[i - 1]);

						}
						clocked = !clocked;
						new_mesh.index_data.emplace_back(ics[i]);

					}

				}

				if (mesh->index_format == RAW4_FORMAT_U32)
					for (uint32_t i = 0; i < mesh->index_count; i++)
						new_mesh.index_data.emplace_back(((uint32_t*)ics)[i]);

				bool has_normal = false;
				bool has_tc = false;

				for (uint32_t i = 0; i < arrays->attributes_count; i++)
				{
					if (arrays->attrs[i].type == RAW4_ATTR_TYPE_POS)
					{
				
						for (uint32_t j = 0; j < verts_count; j++)
						{
							vec3 *pos = (vec3*)(vpos + j*stride + arrays->attrs[i].offset);
							new_mesh.poses.emplace_back(*pos);
						}
					}

					if (arrays->attrs[i].type == RAW4_ATTR_TYPE_NORMAL)
					{
						has_normal = true;

						typedef Vector<vec3_t<char>> norm8;


						if(arrays->attrs[i].format == RAW4_FORMAT_S8)
						for (uint32_t j = 0; j < verts_count; j++)
						{
							norm8 *pos = (norm8*)(vpos + j*stride + arrays->attrs[i].offset);

							float3 normal = (float3(*pos));
							normal.normalize();
							new_mesh.normals.emplace_back(normal);
						}
						else
							for (uint32_t j = 0; j < verts_count; j++)
							{
								float3 *pos = (float3*)(vpos + j*stride + arrays->attrs[i].offset);

								float3 normal = (float3(*pos));
								new_mesh.normals.emplace_back(normal);
							}
					}

					if (arrays->attrs[i].type == RAW4_ATTR_TYPE_UV)
					{

						for (uint32_t j = 0; j < verts_count; j++)
						{
							vec2 *pos = (vec2*)(vpos + j*stride + arrays->attrs[i].offset);
							new_mesh.tcs.emplace_back(pos->x,1-pos->y,0);
						}
					}


				}


				if (!has_normal|| !visible)
				{
					scene_meshes.resize(scene_meshes.size() - 1);
					child->mNumMeshes--;
				}
			}


		



		}

		virtual void go_down() {
		
			matrix_stack.emplace_back();
		}
		virtual void go_up() {
			auto prev = matrix_stack.end();

			std::advance(prev, -1);

			matrix_stack.erase(prev);
		}
	};

}	

void get_additional_assimp_importers(std::vector<  Assimp::BaseImporter* >& out)
{
	out.emplace_back(new Assimp::RAW4Importer());
}
