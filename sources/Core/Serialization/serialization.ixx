module;

export module serialization;
export import "serialization_defines.h";
export import cereal;
export import stl.core;
export import stl.threading;
export import stl.filesystem;

export import crossguid;

export template<typename T> concept PrettyArchive = requires () { T::PRETTY; };

export using serialization_exception = cereal::Exception;

	/*	export template<class Archive, class Type>
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
		}*/




	
export
{

	namespace cereal
	{
		template<class Archive>
			void serialize(Archive& ar, Guid& g)
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
		}
	
	template<class Archive>
	void serialize(Archive& ar, std::filesystem::path& p)
	{
		std::wstring s;
		if (Archive::is_saving::value)
			s = p.wstring();
		ar& NP("string", s);
		if (Archive::is_loading::value)
			p = s;
	}




	template<class Archive, typename clock>
		void serialize(Archive& ar, std::chrono::time_point<clock>& tp)
	{
		/*std::chrono::milliseconds::rep millis;
		ar& NVP(millis);
		tp = std::chrono::time_point<clock>(std::chrono::milliseconds(millis));*/
	}
/*
	template<class Archive, typename clock>
		void save(Archive& ar, std::chrono::time_point<clock> const& tp)
	{
		std::chrono::milliseconds::rep millis = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch()).count();
		ar& NVP(millis);
	}
		*/
		}

}

	
		/*

*/



