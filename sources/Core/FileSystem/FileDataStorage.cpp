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
	failed = false;

	// Its own name per writer (two threads can save the same cache entry at once): no two live storages share an address.
	temp = path;
	temp += "." + std::to_string(reinterpret_cast<uintptr_t>(this)) + ".tmp";
	ostream_ptr = std::make_shared<std::fstream>(temp, std::ios::binary | std::ios::out);
}

bool FileDataStorage::has(std::string partition)
{
	return h.offsets.find(partition) != h.offsets.end();
}

bool FileDataStorage::save()
{
	uint64 pos = ostream_ptr->tellp();
	serialization_oarchive oa(*ostream_ptr);
	oa << h;
	(*ostream_ptr).write(reinterpret_cast<char*>(&pos), sizeof(uint64));
	const bool written = !failed && ostream_ptr->good();
	ostream_ptr->close();
	ostream_ptr = nullptr;

	// The file at `path` is replaced only by a complete new one: until here it could still be read (a save may copy
	// from it), and a save that failed part-way leaves it as it was.
	std::error_code ec;
	if (written)
	{
		std::filesystem::rename(temp, path, ec);
		if (!ec)
			return true;
	}

	Log::get() << Log::LEVEL_ERROR << (written ? "cannot replace " : "writing failed, left as it was: ") << path.string()
		<< (written ? ": " + ec.message() : std::string()) << Log::endl;

	std::error_code ignored;
	std::filesystem::remove(temp, ignored);
	return false;
}
