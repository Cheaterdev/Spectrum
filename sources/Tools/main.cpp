#include "Commands.h"

import Core;
import HAL;
import RenderSystem;

namespace
{
	struct Command
	{
		const char* name;
		const char* help;
		int (*run)(const CommandArgs&);
	};

	const Command commands[] = {
		{ "shaders", "precompile every PSO shader variant and write the cache + manifest for a target", command_shaders },
		{ "textures", "decode images and write their texture cache blobs (for devices without image decoders)", command_textures },
	};

	void print_usage()
	{
		std::printf("usage: spectrum_tool <command> [options]\n\ncommands:\n");
		for (auto& c : commands)
			std::printf("  %-10s %s\n", c.name, c.help);
		std::printf("\nRun from workdir/ (assets, shaders and the engine cache live there).\n");
	}
}

int main(int argc, char** argv)
{
	// Compile failures are reported through the log instead of a blocking message box.
	Application::test_mode = true;

	FileTXTLogger::create();
	StdoutLogger::create();
	Log::get().set_logging_level(Log::LEVEL_ALL);

	FileSystem::get().register_provider(std::make_shared<native_file_provider>());

	if (argc < 2)
	{
		print_usage();
		return 1;
	}

	const std::string name = argv[1];
	CommandArgs args(argv + 2, argv + argc);

	int result = 1;
	bool found = false;
	for (auto& c : commands)
	{
		if (name != c.name)
			continue;
		found = true;
		result = c.run(args);
		break;
	}

	if (!found)
	{
		std::printf("unknown command '%s'\n\n", name.c_str());
		print_usage();
	}

	scheduler::reset();
	return result;
}
