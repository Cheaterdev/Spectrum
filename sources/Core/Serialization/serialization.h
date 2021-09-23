#pragma once


#define NVP(name) boost::serialization::make_nvp(BOOST_PP_STRINGIZE(name), name)
#define NP(name, param) boost::serialization::make_nvp(name, param)

#define SAVE_PARENT(type) boost::serialization::make_nvp("parent", boost::serialization::base_object<type>(*this))
#define SERIALIZE() friend class boost::serialization::access; template<class Archive> void serialize(Archive& ar, const unsigned int version)

