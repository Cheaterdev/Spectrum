#pragma once

#include <cereal/archives/binary.hpp>


using serialization_oarchive = cereal::BinaryOutputArchive;
using serialization_iarchive = cereal::BinaryInputArchive;
/*
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

using serialization_oarchive = boost::archive::binary_oarchive;
using portable_iarchive = boost::archive::binary_iarchive;

using serialization_oarchive = boost::archive::binary_oarchive;
using serialization_iarchive = boost::archive::binary_iarchive;
*/