#pragma once
// Plain header (no module): commands live in their own .cpp and are listed in
// main.cpp's table. A command gets the arguments after its name and returns
// the process exit code.
#include <string>
#include <vector>

using CommandArgs = std::vector<std::string>;

int command_shaders(const CommandArgs& args);
int command_textures(const CommandArgs& args);
