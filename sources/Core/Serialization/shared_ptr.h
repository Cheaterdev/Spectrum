namespace boost
{
	namespace serialization
	{

		template<class Archive, class Type>
		void save(Archive& archive, const std::shared_ptr<Type>& value, const unsigned int /*version*/)
		{
			Type* data = value.get();
			archive << NVP(data);
		}

		static std::map<void*, std::weak_ptr<void>> hash;
		static std::mutex m;

		template<class Archive, class Type>
		void load(Archive& archive, std::shared_ptr<Type>& value, const unsigned int /*version*/)
		{
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

		template<class Archive, class Type>
		inline void serialize(Archive& archive, std::shared_ptr<Type>& value, const unsigned int version)
		{
			split_free(archive, value, version);
		}


		template<class Archive, class Type>
		void save(Archive& archive, const std::unique_ptr<Type>& value, const unsigned int /*version*/)
		{
			Type *data = value.get();
			archive & NVP(data);
		}



		template<class Archive, class Type>
		void load(Archive& archive, std::unique_ptr<Type>& value, const unsigned int /*version*/)
		{
			Type* data;
			archive >> NVP(data);


			value.reset(data);
		//	Type &data = *value.get();
		//	archive & NVP(data);
		}

		template<class Archive, class Type>
		inline void serialize(Archive& archive, std::unique_ptr<Type>& value, const unsigned int version)
		{
			split_free(archive, value, version);
		}

		template<class Archive, class Type>
		inline void serialize(Archive& archive, std::span<Type>& values, const unsigned int version)
		{
			for (int i = 0; i < values.size(); i++)
			{
				archive& values[i];
			}
		}


	}
}