#pragma once

// PROJECT includes
#include "../Core/pch.h"
#include <boost/filesystem/operations.hpp>

// CURRENT includes
#include "FileSystem/FileSystem.h"

#include <filesystem>



namespace boost {
	namespace serialization {
		template<class Archive>
		void serialize(Archive& ar, boost::filesystem::path& p,
			const unsigned int version)
		{
			std::wstring s;
			if (Archive::is_saving::value)
				s = p.wstring();
			ar & boost::serialization::make_nvp("string", s);
			if (Archive::is_loading::value)
				p = s;
		}
	}
}

#pragma comment(lib, "FileSystem")