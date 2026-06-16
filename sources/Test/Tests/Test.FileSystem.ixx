export module Test.FileSystem;

#define TEST_MODULE_ID FileSystem

export import Test.Framework;

import Core;

namespace {
	struct FileSystemSetup {
		FileSystemSetup() {
			auto provider = std::make_shared<native_file_provider>();
			FileSystem::get().register_provider(provider);
		}
	};

	static FileSystemSetup setup;
}

export namespace Test
{
	// File I/O operations tests
	TEST(Core.FileSystem, SaveAndLoadString)
	{
		std::filesystem::path test_file = std::filesystem::temp_directory_path() / "test_string.txt";

		std::string original = "Hello, FileSystem!";
		bool saved = FileSystem::get().save_data(test_file, original);
		ASSERT_TRUE(saved);

		auto file = FileSystem::get().get_file(test_file);
		ASSERT_TRUE(file != nullptr);

		std::string loaded = file->load_all();
		ASSERT_EQ(loaded, original);

		std::filesystem::remove(test_file);
	}

	TEST(Core.FileSystem, SaveAndLoadMultilineText)
	{
		std::filesystem::path test_file = std::filesystem::temp_directory_path() / "test_multiline.txt";

		std::string original = "Line 1\nLine 2\nLine 3\n";
		FileSystem::get().save_data(test_file, original);

		auto file = FileSystem::get().get_file(test_file);
		std::string loaded = file->load_all();

		ASSERT_EQ(loaded, original);

		std::filesystem::remove(test_file);
	}


	TEST(Core.FileSystem, SaveLargeFile)
	{
		std::filesystem::path test_file = std::filesystem::temp_directory_path() / "test_large.txt";

		std::string original;
		for (int i = 0; i < 10000; ++i)
		{
			original += "This is line " + std::to_string(i) + "\n";
		}

		FileSystem::get().save_data(test_file, original);

		auto file = FileSystem::get().get_file(test_file);
		std::string loaded = file->load_all();

		ASSERT_EQ(loaded.length(), original.length());

		std::filesystem::remove(test_file);
	}

	TEST(Core.FileSystem, FileExistence)
	{
		std::filesystem::path test_file = std::filesystem::temp_directory_path() / "test_exists.txt";

		FileSystem::get().save_data(test_file, "test data");
		ASSERT_TRUE(std::filesystem::exists(test_file));

		auto file = FileSystem::get().get_file(test_file);
		ASSERT_TRUE(file != nullptr);

		std::filesystem::remove(test_file);
		ASSERT_FALSE(std::filesystem::exists(test_file));
	}

	// Path handling and manipulation tests
	TEST(Core.FileSystem, PathExtension)
	{
		std::filesystem::path file_path = "document.txt";
		ASSERT_EQ(file_path.extension().string(), std::string(".txt"));
	}

	TEST(Core.FileSystem, PathFilename)
	{
		std::filesystem::path file_path = "/home/user/documents/file.txt";
		ASSERT_EQ(file_path.filename().string(), std::string("file.txt"));
	}

	TEST(Core.FileSystem, PathParent)
	{
		std::filesystem::path file_path = "/home/user/documents/file.txt";
		std::filesystem::path parent = file_path.parent_path();
		ASSERT_TRUE(parent.string().find("documents") != std::string::npos);
	}

	TEST(Core.FileSystem, PathCombine)
	{
		std::filesystem::path dir = std::filesystem::temp_directory_path();
		std::filesystem::path filename = "test.txt";
		std::filesystem::path full_path = dir / filename;

		ASSERT_TRUE(full_path.string().find("test.txt") != std::string::npos);
	}

	TEST(Core.FileSystem, PathAbsolute)
	{
		std::filesystem::path relative = "file.txt";
		std::filesystem::path absolute = std::filesystem::absolute(relative);

		ASSERT_TRUE(absolute.is_absolute());
	}

	TEST(Core.FileSystem, PathRelative)
	{
		std::filesystem::path base = std::filesystem::temp_directory_path();
		std::filesystem::path full = base / "test.txt";
		std::filesystem::path relative = std::filesystem::relative(full, base);

		ASSERT_EQ(relative.string(), std::string("test.txt"));
	}

	TEST(Core.FileSystem, PathComparison)
	{
		std::filesystem::path path1 = "/home/user/file.txt";
		std::filesystem::path path2 = "/home/user/file.txt";
		std::filesystem::path path3 = "/home/user/other.txt";

		ASSERT_EQ(path1, path2);
		ASSERT_NE(path1, path3);
	}

	// Directory operations tests
	TEST(Core.FileSystem, CreateDirectory)
	{
		std::filesystem::path test_dir = std::filesystem::temp_directory_path() / "test_dir_123";

		if (std::filesystem::exists(test_dir))
			std::filesystem::remove_all(test_dir);

		std::filesystem::create_directory(test_dir);
		ASSERT_TRUE(std::filesystem::exists(test_dir));
		ASSERT_TRUE(std::filesystem::is_directory(test_dir));

		std::filesystem::remove(test_dir);
	}

	TEST(Core.FileSystem, CreateNestedDirectories)
	{
		std::filesystem::path test_dir = std::filesystem::temp_directory_path() / "test_nested" / "sub" / "deep";

		if (std::filesystem::exists(std::filesystem::temp_directory_path() / "test_nested"))
			std::filesystem::remove_all(std::filesystem::temp_directory_path() / "test_nested");

		std::filesystem::create_directories(test_dir);
		ASSERT_TRUE(std::filesystem::exists(test_dir));

		std::filesystem::remove_all(std::filesystem::temp_directory_path() / "test_nested");
	}

	TEST(Core.FileSystem, ListDirectoryFiles)
	{
		std::filesystem::path test_dir = std::filesystem::temp_directory_path() / "test_list_dir";

		if (std::filesystem::exists(test_dir))
			std::filesystem::remove_all(test_dir);

		std::filesystem::create_directory(test_dir);

		FileSystem::get().save_data(test_dir / "file1.txt", "content1");
		FileSystem::get().save_data(test_dir / "file2.txt", "content2");
		FileSystem::get().save_data(test_dir / "file3.txt", "content3");

		std::atomic<int> file_count(0);
		FileSystem::get().iterate(test_dir, [&file_count](auto) {
			file_count.fetch_add(1, std::memory_order_relaxed);
		}, false);

		ASSERT_EQ(file_count.load(std::memory_order_relaxed), 3);

		std::filesystem::remove_all(test_dir);
	}

	TEST(Core.FileSystem, ListDirectories)
	{
		std::filesystem::path test_dir = std::filesystem::temp_directory_path() / "test_list_dirs";

		if (std::filesystem::exists(test_dir))
			std::filesystem::remove_all(test_dir);

		std::filesystem::create_directory(test_dir);
		std::filesystem::create_directory(test_dir / "subdir1");
		std::filesystem::create_directory(test_dir / "subdir2");
		std::filesystem::create_directory(test_dir / "subdir3");

		std::atomic<int> dir_count(0);
		FileSystem::get().iterate_dirs(test_dir, [&dir_count](auto) {
			dir_count.fetch_add(1, std::memory_order_relaxed);
		}, false);

		ASSERT_EQ(dir_count.load(std::memory_order_relaxed), 3);

		std::filesystem::remove_all(test_dir);
	}

	TEST(Core.FileSystem, RecursiveDirectoryIteration)
	{
		std::filesystem::path test_dir = std::filesystem::temp_directory_path() / "test_recursive";

		if (std::filesystem::exists(test_dir))
			std::filesystem::remove_all(test_dir);

		std::filesystem::create_directory(test_dir);
		std::filesystem::create_directory(test_dir / "level1");
		std::filesystem::create_directory(test_dir / "level1" / "level2");

		FileSystem::get().save_data(test_dir / "file1.txt", "content");
		FileSystem::get().save_data(test_dir / "level1" / "file2.txt", "content");
		FileSystem::get().save_data(test_dir / "level1" / "level2" / "file3.txt", "content");

		std::atomic<int> file_count(0);
		FileSystem::get().iterate(test_dir, [&file_count](auto) {
			file_count.fetch_add(1, std::memory_order_relaxed);
		}, true);

		ASSERT_EQ(file_count.load(std::memory_order_relaxed), 3);

		std::filesystem::remove_all(test_dir);
	}

	TEST(Core.FileSystem, DirectorySize)
	{
		std::filesystem::path test_dir = std::filesystem::temp_directory_path() / "test_size";

		if (std::filesystem::exists(test_dir))
			std::filesystem::remove_all(test_dir);

		std::filesystem::create_directory(test_dir);

		FileSystem::get().save_data(test_dir / "file1.txt", "12345");
		FileSystem::get().save_data(test_dir / "file2.txt", "67890");

		std::atomic<size_t> total_size(0);
		FileSystem::get().iterate(test_dir, [&total_size](auto file) {
			std::string content = file->load_all();
			total_size.fetch_add(content.length(), std::memory_order_relaxed);
		}, false);

		ASSERT_EQ(total_size.load(std::memory_order_relaxed), static_cast<size_t>(10));

		std::filesystem::remove_all(test_dir);
	}

	TEST(Core.FileSystem, DeleteFile)
	{
		std::filesystem::path test_file = std::filesystem::temp_directory_path() / "test_delete.txt";

		FileSystem::get().save_data(test_file, "temporary");
		ASSERT_TRUE(std::filesystem::exists(test_file));

		std::filesystem::remove(test_file);
		ASSERT_FALSE(std::filesystem::exists(test_file));
	}

	TEST(Core.FileSystem, RenameFile)
	{
		std::filesystem::path test_file = std::filesystem::temp_directory_path() / "test_old.txt";
		std::filesystem::path new_file = std::filesystem::temp_directory_path() / "test_new.txt";

		FileSystem::get().save_data(test_file, "content");
		ASSERT_TRUE(std::filesystem::exists(test_file));

		std::filesystem::rename(test_file, new_file);
		ASSERT_FALSE(std::filesystem::exists(test_file));
		ASSERT_TRUE(std::filesystem::exists(new_file));

		std::filesystem::remove(new_file);
	}

	TEST(Core.FileSystem, CopyFile)
	{
		std::filesystem::path original = std::filesystem::temp_directory_path() / "test_original.txt";
		std::filesystem::path copy = std::filesystem::temp_directory_path() / "test_copy.txt";

		std::string content = "original content";
		FileSystem::get().save_data(original, content);

		std::filesystem::copy_file(original, copy);
		ASSERT_TRUE(std::filesystem::exists(copy));

		auto copied_file = FileSystem::get().get_file(copy);
		ASSERT_EQ(copied_file->load_all(), content);

		std::filesystem::remove(original);
		std::filesystem::remove(copy);
	}

	TEST(Core.FileSystem, FileSize)
	{
		std::filesystem::path test_file = std::filesystem::temp_directory_path() / "test_file_size.txt";

		std::string content = "0123456789";
		FileSystem::get().save_data(test_file, content);

		size_t size = std::filesystem::file_size(test_file);
		ASSERT_EQ(size, static_cast<size_t>(10));

		std::filesystem::remove(test_file);
	}
}
