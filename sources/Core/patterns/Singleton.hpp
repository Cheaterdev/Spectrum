
template <typename T>
Singleton<T>::~Singleton()
{
	if (!std::is_same<T, Log>::value)
		if (!std::is_same<T, ClassLogger<singleton_system>>::value)
			ClassLogger<singleton_system>::get() << Log::LEVEL_DEBUG << "Singleton deleting " << typeid(T).name() << Log::endl;
		else
			OutputDebugStringA((string("Singleton deleting ") + typeid(T).name() + "\n").c_str());
}

template <typename T>
Singleton<T>::Singleton()
{
	if (!std::is_same<T, Log>::value)
		if (!std::is_same<T, ClassLogger<singleton_system>>::value)
			ClassLogger<singleton_system>::get() << Log::LEVEL_DEBUG << "Singleton creating " << typeid(T).name() << Log::endl;
		else
			OutputDebugStringA((string("Singleton creating  ") + typeid(T).name() + "\n").c_str());
}
