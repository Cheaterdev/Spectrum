#pragma once

// The syntax tree PrismBridge.cpp hands to Parsing.cpp. It mirrors the part of
// cpp-peglib's AST that the listener reads, so peglib.h itself stays inside the
// one plain translation unit that includes it (a header unit of it leaves its
// static string constants null in the importers). No includes: the bridge
// includes the standard headers itself and Parsing.cpp gets them from Core.

constexpr unsigned prism_tag(std::string_view name, unsigned h = 0)
{
	return name.empty() ? h : prism_tag(name.substr(1), h * 33 + static_cast<unsigned char>(name[0]));
}

struct PrismNode
{
	std::string name;     // the grammar rule that produced it
	unsigned tag = 0;     // prism_tag(name), for switch dispatch
	bool is_token = false;
	std::string_view token; // is_token only: exact text, a view into the parsed buffer
	size_t line = 1;      // 1-based, where the node's first token starts
	size_t column = 1;
	PrismNode* parent = nullptr;
	std::vector<std::unique_ptr<PrismNode>> nodes;
};

struct PrismSyntaxError
{
	size_t line = 0;
	size_t column = 0;
	std::string message;
};

// Parses `text` (which must outlive the returned tree). On a syntax error returns
// null and fills `errors`.
extern "C++" std::unique_ptr<PrismNode> prism_parse_ast(const std::string& text, const char* file, std::vector<PrismSyntaxError>& errors);

// Every keyword literal in the grammar, for completion.
extern "C++" std::vector<std::string> prism_grammar_keywords();
