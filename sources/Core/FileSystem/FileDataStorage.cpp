module Core:FileDataStorage;

import stl.filesystem;
import :Serializer;

FileDataStorage::FileDataStorage(std::filesystem::path path) : path(path)
{
	std::fstream stream(path, std::ios::binary | std::ios::in);

	if (!stream.is_open()) return;

	stream.seekg(0, std::ios::end);
	uint64 file_size = stream.tellg();

	stream.seekg(file_size - sizeof(uint64), std::ios::beg);

	uint64 header_offset = 0;

	stream.read(reinterpret_cast<char*>(&header_offset), sizeof(uint64));

	stream.seekg(header_offset, std::ios::beg);

	serialization_iarchive oa(stream);

	oa >> h;
	stream.close();
}

void FileDataStorage::start_save()
{
	std::filesystem::create_directories(path.parent_path());
	ostream_ptr = std::make_shared<std::fstream>(path, std::ios::binary | std::ios::out);
}

bool FileDataStorage::has(std::string partition)
{
	return h.offsets.find(partition) != h.offsets.end();
}

void FileDataStorage::save()
{
	uint64 pos = ostream_ptr->tellp();
	serialization_oarchive oa(*ostream_ptr);
	oa << h;
	(*ostream_ptr).write(reinterpret_cast<char*>(&pos), sizeof(uint64));
	ostream_ptr->close();
	ostream_ptr = nullptr;
}
