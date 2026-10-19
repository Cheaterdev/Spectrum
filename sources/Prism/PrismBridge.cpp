// cpp-peglib behind two plain functions, for Parsing.cpp.
//
// A plain (non-module) translation unit on purpose: peglib.h imported as a header
// unit leaves its `static const char*` constants null in the importing module, so
// the grammar fails to load. See PrismAst.h for the tree handed back.
#include <peglib.h>

#include <algorithm>
#include <cctype>
#include <format>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "PrismAst.h"
#include "PrismGrammar.h"

static const std::string& grammar_text()
{
	static const std::string text = std::string(prism_grammar_a) + prism_grammar_b + prism_grammar_c;
	return text;
}

static std::unique_ptr<PrismNode> convert(const peg::Ast& ast, PrismNode* parent)
{
	auto node = std::make_unique<PrismNode>();
	node->name = ast.name;
	node->tag = prism_tag(ast.name);
	node->is_token = ast.is_token;
	node->token = ast.token;
	node->line = ast.line;
	node->column = ast.column;
	node->parent = parent;
	node->nodes.reserve(ast.nodes.size());
	for (const auto& child : ast.nodes)
		node->nodes.push_back(convert(*child, node.get()));
	return node;
}

std::unique_ptr<PrismNode> prism_parse_ast(const std::string& text, const char* file, std::vector<PrismSyntaxError>& errors)
{
	static const peg::parser base = []
	{
		peg::parser p;
		std::string load_errors;
		p.enable_left_recursion(false);
		p.set_logger([&](size_t line, size_t col, const std::string& msg, const std::string&)
		{
			load_errors += std::format("Prism grammar ({},{}): {}\n", line, col, msg);
		});
		if (!p.load_grammar(grammar_text()))
			throw std::logic_error("internal: the Prism grammar does not load\n" + load_errors);
		p.enable_ast();
		return p;
	}();

	// A copy shares the compiled grammar and only gets its own logger.
	peg::parser parser = base;
	parser.set_logger([&](size_t line, size_t col, const std::string& msg, const std::string&)
	{
		errors.push_back({ line, col, msg });
	});

	std::shared_ptr<peg::Ast> ast;
	if (!parser.parse(text, ast, file))
	{
		if (errors.empty())
			errors.push_back({ 1, 1, "syntax error" });
		return nullptr;
	}

	return convert(*ast, nullptr);
}

std::vector<std::string> prism_grammar_keywords()
{
	// Every quoted word in the grammar's rules is a keyword; whole-line comments
	// are skipped because they quote words as well.
	std::vector<std::string> out;
	std::istringstream lines(grammar_text());

	for (std::string line; std::getline(lines, line);)
	{
		size_t first = line.find_first_not_of(" \t");
		if (first == std::string::npos || line[first] == '#')
			continue;

		for (size_t open = line.find('\''); open != std::string::npos; open = line.find('\'', open + 1))
		{
			size_t close = line.find('\'', open + 1);
			if (close == std::string::npos)
				break;

			std::string word = line.substr(open + 1, close - open - 1);
			bool is_word = !word.empty() && std::isalpha(static_cast<unsigned char>(word[0]))
				&& std::all_of(word.begin(), word.end(), [](unsigned char c) { return std::isalnum(c) || c == '_'; });
			if (is_word && std::find(out.begin(), out.end(), word) == out.end())
				out.push_back(word);

			open = close; // the closing quote must not open the next literal
		}
	}
	return out;
}
