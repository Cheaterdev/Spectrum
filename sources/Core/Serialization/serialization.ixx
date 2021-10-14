module;
#include "serialization_defines.h"
export module serialization;
export import boost.serialization;
export import stl.core;
export import stl.threading;
export import stl.filesystem;

export import crossguid;

export template<typename T> concept PrettyArchive = requires () { T::PRETTY; };


namespace boost
{
	namespace serialization
	{

		export template<class Archive, class Type>
		void save(Archive& archive, const std::shared_ptr<Type>& value, const unsigned int)
		{
			Type* data = value.get();
			archive << NVP(data);
		}

		export template<class Archive, class Type>
		void load(Archive& archive, std::shared_ptr<Type>& value, const unsigned int)
		{
			static std::map<void*, std::weak_ptr<void>> hash;
			static std::mutex m;

			Type* data;
			archive >> NVP(data);
			std::lock_guard<std::mutex> g(m);
			auto&& info = hash[data];

			if (info.expired())
			{
				value = std::shared_ptr<Type>(data);
				info = value;
			}

			else value = static_pointer_cast<Type>(info.lock());
		}

		export template<class Archive, class Type>
		inline void serialize(Archive& archive, std::shared_ptr<Type>& value, const unsigned int version)
		{
			split_free(archive, value, version);
		}


		export template<class Archive, class Type>
		void save(Archive& archive, const std::unique_ptr<Type>& value, const unsigned int)
		{
			Type* data = value.get();
			archive& NVP(data);
		}



		export template<class Archive, class Type>
		void load(Archive& archive, std::unique_ptr<Type>& value, const unsigned int )
		{
			Type* data;
			archive >> NVP(data);


			value.reset(data);
			//	Type &data = *value.get();
			//	archive & NVP(data);
		}

		export template<class Archive, class Type>
		inline void serialize(Archive& archive, std::unique_ptr<Type>& value, const unsigned int version)
		{
			split_free(archive, value, version);
		}

		export template<class Archive, class Type>
		inline void serialize(Archive& archive, std::span<Type>& values, const unsigned int version)
		{
			for (int i = 0; i < values.size(); i++)
			{
				archive& values[i];
			}
		}




		namespace chrn = std::chrono;

		export template<class Archive, typename clock>
		void load(Archive& ar, chrn::time_point<clock>& tp, unsigned)
		{
			chrn::milliseconds::rep millis;
			ar& NVP(millis);
			tp = chrn::time_point<clock>(chrn::milliseconds(millis));
		}

		export template<class Archive, typename clock>
		void save(Archive& ar, chrn::time_point<clock> const& tp, unsigned)
		{
			chrn::milliseconds::rep millis = chrn::duration_cast<chrn::milliseconds>(tp.time_since_epoch()).count();
			ar& NVP(millis);
		}

		export template<class Archive, typename clock>
		inline void serialize(Archive& ar, std::chrono::time_point<clock>& tp, unsigned version)
		{
			boost::serialization::split_free(ar, tp, version);
		}


		export template<class Archive>
		inline void serialize(Archive& ar, Guid& g, unsigned version)
		{
			if constexpr (Archive::is_loading::value)
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
			//boost::serialization::split_free(ar, tp, version);
		}


		export template<class Archive>
		void serialize(Archive& ar, std::filesystem::path& p,
			const unsigned int version)
		{
			std::wstring s;
			if (Archive::is_saving::value)
				s = p.wstring();
			ar& boost::serialization::make_nvp("string", s);
			if (Archive::is_loading::value)
				p = s;
		}

	}
}

