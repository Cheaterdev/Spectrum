#define SER_STR(x) #x

#define NP(name, param) cereal::make_nvp(name, param)
#define NVP(name) NP(SER_STR(name), name)
#define NVPG(name) NP(SER_STR(name), g.name)

#define SAVE_PARENT(type)  ar&NP("parent_" SER_STR(type), cereal::base_class<type>(this))
#define SAVE_VPARENT(type)  ar&NP("parent_" SER_STR(type), cereal::virtual_base_class<type>(this))

#define SERIALIZE()  friend class cereal::access; template<class Archive> void serialize(Archive& ar, const unsigned int version)
#define SERIALIZE_PRETTY()  friend class cereal::access; template<PrettyArchive Archive> void serialize(Archive& ar, const unsigned int version)

#define REGISTER_TYPE(...) CEREAL_REGISTER_TYPE(__VA_ARGS__)