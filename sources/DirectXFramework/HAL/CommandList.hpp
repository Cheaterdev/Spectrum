template<class T>
size_t DX12::Uploader::size_of(std::vector<T>& elem)
{
	return sizeof(T) * elem.size();
}

template<class T>
size_t DX12::Uploader::size_of(std::span<T>& elem)
{
	return sizeof(T) * elem.size();
}

template<class T>
size_t DX12::Uploader::size_of(std::vector<T>& elem);

template<class T>
size_t DX12::Uploader::size_of(T& elem)
{
	return sizeof(T);
}
