
template<class T>
class BinaryObjects:public Singleton<BinaryObjects<T>>
{
	friend class Singleton<BinaryObjects<T>>;

	std::map<std::string, std::string> objects;
	BinaryObjects(){}

public:
	void save(std::string name, const T &obj)
	{
		objects[name] = Serializer::serialize(obj);
	}

	std::shared_ptr<T> load(std::string name)
	{
		auto it = objects.find(name);

		if (it == objects.end())
			return std::shared_ptr<T>();

		return Serializer::deserialize<T>(it.second);
	}
};