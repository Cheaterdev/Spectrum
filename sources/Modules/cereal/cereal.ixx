export module cereal;

export import "cereal.h";

export using serialization_oarchive = cereal::BinaryOutputArchive;
export using serialization_iarchive = cereal::BinaryInputArchive;