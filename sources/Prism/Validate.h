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

// workdir/shaders of the checkout a .prism file belongs to; empty if not found.
std::filesystem::path shaders_root(const std::string& sig_file);
