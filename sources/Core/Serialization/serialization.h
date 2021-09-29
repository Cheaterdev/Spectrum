#pragma once

import serialization;

#define NVP(name) serialization::make_nvp(BOOST_PP_STRINGIZE(name), name)
#define NP(name, param)serialization::make_nvp(name, param)

#define SAVE_PARENT(type) serialization::make_nvp("parent", serialization::base_object<type>(*this))
#define SERIALIZE() friend class serialization::access; template<class Archive> void serialize(Archive& ar, const unsigned int version)
#define SERIALIZE_PRETTY() friend class serialization::access; template<PrettyArchive Archive> void serialize(Archive& ar, const unsigned int version)

