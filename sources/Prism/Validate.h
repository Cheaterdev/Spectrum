#pragma once
#include "Parsed.h"

// Semantic checks over the merged model. Reports into diagnostics(); the
// caller decides whether to generate.
void validate(Parsed& parsed);

// The option names validate() accepts on a declaration kind ("PassNode",
// "PassNode field", "struct", ...); empty for an unknown kind.
const std::set<std::string>& known_options(const std::string& kind);

// Every declaration kind that accepts `option_name`.
std::vector<std::string> option_kinds(const std::string& option_name);

// Functions a [Size] expression may call -> the C++ each renders to. Kept in one
// place so the generator and the validator can't disagree about the set.
const std::map<std::string, std::string>& size_functions();

// workdir/shaders of the checkout a .prism file belongs to; empty if not found.
std::filesystem::path shaders_root(const std::string& sig_file);

// A C++ enum whose names an option's value must be: [Always]/[RecreateFlags]
// take FrameGraph::ResourceFlags, [Format] takes HAL::Format. The names are read
// from the engine source of the checkout, so there is no copy to keep in sync.
struct CppEnum
{
	std::string cpp_name;
	std::filesystem::path source;
	std::vector<std::string> names; // empty if the source couldn't be read or parsed
};

// nullptr when `option_name` isn't enum-valued.
const CppEnum* option_enum(const std::string& option_name, const std::string& sig_file);
