export module assimp;
export import "assimp/Importer.hpp";
export import "assimp/importerdesc.h";
export import "assimp/postprocess.h";
export import "assimp/scene.h";
export import "assimp/DefaultLogger.hpp";
export import "assimp/LogStream.hpp";
export import "assimp/ProgressHandler.hpp";
export import "assimp/IOStream.hpp";
export import "assimp/IOSystem.hpp";
export import "assimp/types.h";


// ---- What the header unit cannot carry ---------------------------------------
// Macros do not cross a module boundary, and aiMaterial::Get<T> reaches static-inline helpers that are
// not reachable from importers. Material keys are therefore values, and the getters are defined in assimp.cpp.
export struct AiMatKey
{
	const char* key;
	unsigned    type;
	unsigned    index;
};

export namespace AiKey
{
	inline constexpr AiMatKey ColorDiffuse{ "$clr.diffuse", 0, 0 };
	inline constexpr AiMatKey ColorSpecular{ "$clr.specular", 0, 0 };
	inline constexpr AiMatKey Shininess{ "$mat.shininess", 0, 0 };
	inline constexpr AiMatKey Reflectivity{ "$mat.reflectivity", 0, 0 };
	inline constexpr AiMatKey Opacity{ "$mat.opacity", 0, 0 };
	inline constexpr AiMatKey EmissiveIntensity{ "$mat.emissiveIntensity", 0, 0 };
	inline constexpr AiMatKey GlossinessFactor{ "$mat.glossinessFactor", 0, 0 };
	inline constexpr AiMatKey Refracti{ "$mat.refracti", 0, 0 };
	inline constexpr AiMatKey TransmissionFactor{ "$mat.transmission.factor", 0, 0 };
	inline constexpr AiMatKey VolumeThicknessFactor{ "$mat.volume.thicknessFactor", 0, 0 };
	inline constexpr AiMatKey VolumeAttenuationDistance{ "$mat.volume.attenuationDistance", 0, 0 };
}

export namespace AiConfig
{
	inline constexpr const char* PP_SBP_REMOVE = "PP_SBP_REMOVE";
}

export aiReturn ai_get(const aiMaterial& material, const AiMatKey& key, float& out);
export aiReturn ai_get(const aiMaterial& material, const AiMatKey& key, aiColor3D& out);
