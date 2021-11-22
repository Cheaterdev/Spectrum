#define SER_STR(x) #x

#define NP(name, param) cereal::make_nvp(name, param)
#define NVP(name) NP(SER_STR(name), name)

#define SAVE_PARENT(type)  NP("parent", cereal::base_class<type>(this))
#define SAVE_VPARENT(type)  NP("parent", cereal::virtual_base_class<type>(this))

#define SERIALIZE()  friend class cereal::access; template<class Archive> void serialize(Archive& ar, const unsigned int version)
#define SERIALIZE_PRETTY()  friend class cereal::access; template<PrettyArchive Archive> void serialize(Archive& ar, const unsigned int version)

