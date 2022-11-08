export module Core:serialization;
export import "serialization_defines.h";
export import cereal;
export import stl.core;
export import stl.threading;
export import stl.filesystem;

export import crossguid;
//export import simple_log_archive;

import :Utils;

export template<typename T> concept PrettyArchive = requires () { T::PRETTY; };

export using serialization_exception = cereal::Exception;

export
{

	namespace cereal
	{
		template<class Archive>
		void serialize(Archive& ar, Guid& g)
		{
			IF_LOAD()
			{
				std::array<unsigned char, 16> v;
				ar& NVP(v);
				g = Guid(v);

			}
			else
			{
			auto& v = g.bytes();
			ar& NVP(v);
			}
		}

		template<class Archive>
		void serialize(Archive& ar, std::filesystem::path& p)
		{
			std::wstring s;
			if (Archive::is_saving::value)
				s = p.wstring();
			ar& NP("string", s);

			//BUG_ALERT;
			if (Archive::is_loading::value)
				p = s;
		}




		template<class Archive, typename clock>
		void serialize(Archive& ar, std::chrono::time_point<clock>& tp)
		{
			if constexpr (Archive::is_loading::value)
			{
				std::chrono::milliseconds::rep millis;
				ar& NVP(millis);
				tp = std::chrono::time_point<clock>(std::chrono::milliseconds(millis));
			}
			else
			{
				std::chrono::milliseconds::rep millis = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch()).count();
				ar& NVP(millis);
			}
		}


	}
}

