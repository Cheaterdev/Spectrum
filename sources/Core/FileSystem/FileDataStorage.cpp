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

std::shared_mutex& FileDataStorage::relocation_mutex()
{
	static std::shared_mutex m;
	return m;
}

bool FileDataStorage::append(const std::string& partition, const std::string& bytes)
{
	std::error_code ec;
	const uint64 old_size = std::filesystem::file_size(path, ec);
	if (ec || h.offsets.empty())
		return false;   // no file, or none whose table was read

	header updated = h;
	updated.offsets[partition] = old_size;
	const uint64 header_at = old_size + bytes.size();

	// Everything in one buffer, so the file is touched by a single write after nothing can throw any more.
	std::ostringstream tail(std::ios::binary);
	tail.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
	{
		serialization_oarchive oa(tail);
		oa << updated;
	}
	tail.write(reinterpret_cast<const char*>(&header_at), sizeof(header_at));
	const std::string out = tail.str();

	bool wrote = false;
	bool written = false;
	{
		std::fstream file(path, std::ios::binary | std::ios::in | std::ios::out);
		if (file)
		{
			file.seekp(0, std::ios::end);
			if (static_cast<uint64>(file.tellp()) == old_size)   // not changed since the table was read
			{
				wrote = true;
				file.write(out.data(), static_cast<std::streamsize>(out.size()));
				file.flush();
				written = file.good();
			}
		}
	}

	if (written)
	{
		h = std::move(updated);
		return true;
	}

	// A part-written tail is cut off again: the old table is still the last thing in the file.
	if (wrote)
		std::filesystem::resize_file(path, old_size, ec);
	Log::get() << Log::LEVEL_ERROR << "cannot add " << partition << " to " << path.string() << Log::endl;
	return false;
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
		// A virus scanner or the search indexer can hold a file that was just written or read for a moment, which makes
		// the rename fail with a sharing violation: retried a few times before giving up.
		for (int attempt = 0; attempt < 5; attempt++)
		{
			std::filesystem::rename(temp, path, ec);
			if (!ec)
				return true;
			std::this_thread::sleep_for(std::chrono::milliseconds(20));
		}
	}

	Log::get() << Log::LEVEL_ERROR << (written ? "cannot replace " : "writing failed, left as it was: ") << path.string()
		<< (written ? ": " + ec.message() : std::string()) << Log::endl;

	std::error_code ignored;
	std::filesystem::remove(temp, ignored);
	return false;
}
