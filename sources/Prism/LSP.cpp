import Core;
import windows;

#include "LSP.h"
#include "Parsing.h"
#include "Diagnostics.h"
#include "Validate.h"

// Deliberately does not use rapidjson: importing cereal.json into a second
// translation unit of this executable makes Main.cpp's make_map() crash with an
// access violation (MSVC header-unit issue; reproduced with a three-line stub
// that only parsed "{}"). LSP messages are small, so a minimal reader suffices.
namespace
{
	struct Json
	{
		enum Type { Null, Bool, Number, String, Array, Object } type = Null;
		bool boolean = false;
		std::string text;               // String value, or a Number's source text (echoed back verbatim as an id)
		std::vector<Json> items;        // Array elements, or Object values
		std::vector<std::string> keys;  // Object keys, parallel to items

		const Json& operator[](std::string_view key) const
		{
			static const Json null;
			for (size_t i = 0; i < keys.size(); ++i)
				if (keys[i] == key)
					return items[i];
			return null;
		}

		bool has(std::string_view key) const
		{
			return std::find(keys.begin(), keys.end(), key) != keys.end();
		}
	};

	class JsonReader
	{
		std::string_view s;
		size_t pos = 0;

		void skip_ws()
		{
			while (pos < s.size() && (s[pos] == ' ' || s[pos] == '\t' || s[pos] == '\n' || s[pos] == '\r'))
				++pos;
		}

		void expect(char c)
		{
			skip_ws();
			if (pos >= s.size() || s[pos] != c)
				throw std::runtime_error(std::format("json: expected '{}' at {}", c, pos));
			++pos;
		}

		static void append_utf8(std::string& out, uint32_t cp)
		{
			if (cp < 0x80)
				out += (char)cp;
			else if (cp < 0x800)
			{
				out += (char)(0xC0 | (cp >> 6));
				out += (char)(0x80 | (cp & 0x3F));
			}
			else if (cp < 0x10000)
			{
				out += (char)(0xE0 | (cp >> 12));
				out += (char)(0x80 | ((cp >> 6) & 0x3F));
				out += (char)(0x80 | (cp & 0x3F));
			}
			else
			{
				out += (char)(0xF0 | (cp >> 18));
				out += (char)(0x80 | ((cp >> 12) & 0x3F));
				out += (char)(0x80 | ((cp >> 6) & 0x3F));
				out += (char)(0x80 | (cp & 0x3F));
			}
		}

		uint32_t hex4()
		{
			if (pos + 4 > s.size())
				throw std::runtime_error("json: truncated \\u escape");
			uint32_t v = std::stoul(std::string(s.substr(pos, 4)), nullptr, 16);
			pos += 4;
			return v;
		}

		std::string read_string()
		{
			expect('"');
			std::string out;
			while (pos < s.size() && s[pos] != '"')
			{
				char c = s[pos++];
				if (c != '\\')
				{
					out += c;
					continue;
				}
				char e = s[pos++];
				switch (e)
				{
				case 'n': out += '\n'; break;
				case 'r': out += '\r'; break;
				case 't': out += '\t'; break;
				case 'b': out += '\b'; break;
				case 'f': out += '\f'; break;
				case 'u':
				{
					uint32_t cp = hex4();
					if (cp >= 0xD800 && cp < 0xDC00 && pos + 1 < s.size() && s[pos] == '\\' && s[pos + 1] == 'u')
					{
						pos += 2;
						cp = 0x10000 + ((cp - 0xD800) << 10) + (hex4() - 0xDC00);
					}
					append_utf8(out, cp);
					break;
				}
				default: out += e; break; // \" \\ \/
				}
			}
			expect('"');
			return out;
		}

		Json read_value()
		{
			skip_ws();
			if (pos >= s.size())
				throw std::runtime_error("json: unexpected end");

			Json v;
			char c = s[pos];
			if (c == '{')
			{
				v.type = Json::Object;
				++pos;
				skip_ws();
				if (pos < s.size() && s[pos] == '}')
				{
					++pos;
					return v;
				}
				for (;;)
				{
					v.keys.push_back(read_string());
					expect(':');
					v.items.push_back(read_value());
					skip_ws();
					if (pos < s.size() && s[pos] == ',')
					{
						++pos;
						continue;
					}
					expect('}');
					return v;
				}
			}
			if (c == '[')
			{
				v.type = Json::Array;
				++pos;
				skip_ws();
				if (pos < s.size() && s[pos] == ']')
				{
					++pos;
					return v;
				}
				for (;;)
				{
					v.items.push_back(read_value());
					skip_ws();
					if (pos < s.size() && s[pos] == ',')
					{
						++pos;
						continue;
					}
					expect(']');
					return v;
				}
			}
			if (c == '"')
			{
				v.type = Json::String;
				v.text = read_string();
				return v;
			}
			if (s.substr(pos, 4) == "true" || s.substr(pos, 5) == "false")
			{
				v.type = Json::Bool;
				v.boolean = s[pos] == 't';
				pos += v.boolean ? 4 : 5;
				return v;
			}
			if (s.substr(pos, 4) == "null")
			{
				pos += 4;
				return v;
			}

			size_t start = pos;
			while (pos < s.size() && (std::isdigit((unsigned char)s[pos]) || s[pos] == '-' || s[pos] == '+' || s[pos] == '.' || s[pos] == 'e' || s[pos] == 'E'))
				++pos;
			if (pos == start)
				throw std::runtime_error(std::format("json: unexpected '{}' at {}", c, pos));
			v.type = Json::Number;
			v.text = std::string(s.substr(start, pos - start));
			return v;
		}

	public:
		static Json parse(std::string_view text)
		{
			JsonReader r;
			r.s = text;
			return r.read_value();
		}
	};

	// stdin/stdout via Win32 handles rather than iostreams: the protocol needs
	// exact byte counts, and CRT text mode would rewrite \n in both directions.
	HANDLE in_handle = GetStdHandle(STD_INPUT_HANDLE);
	HANDLE out_handle = GetStdHandle(STD_OUTPUT_HANDLE);

	bool read_bytes(char* dst, size_t size)
	{
		while (size)
		{
			DWORD got = 0;
			if (!ReadFile(in_handle, dst, (DWORD)size, &got, nullptr) || got == 0)
				return false;
			dst += got;
			size -= got;
		}
		return true;
	}

	bool read_message(std::string& body)
	{
		size_t length = 0;
		std::string line;
		for (;;)
		{
			char c;
			if (!read_bytes(&c, 1))
				return false;
			if (c != '\n')
			{
				line += c;
				continue;
			}
			if (!line.empty() && line.back() == '\r')
				line.pop_back();
			if (line.empty())
				break;
			if (line.rfind("Content-Length:", 0) == 0)
				length = std::stoull(line.substr(15));
			line.clear();
		}

		body.resize(length);
		return read_bytes(body.data(), length);
	}

	void write_message(const std::string& body)
	{
		std::string msg = "Content-Length: " + std::to_string(body.size()) + "\r\n\r\n" + body;
		DWORD written = 0;
		WriteFile(out_handle, msg.data(), (DWORD)msg.size(), &written, nullptr);
	}

	// True when the client has already sent more: used to coalesce a burst of
	// didChange notifications (one per keystroke) into a single revalidation.
	bool input_pending()
	{
		DWORD available = 0;
		return PeekNamedPipe(in_handle, nullptr, 0, nullptr, &available, nullptr) && available > 0;
	}

	std::string json_escape(std::string_view s)
	{
		std::string out;
		out.reserve(s.size() + 2);
		for (unsigned char c : s)
		{
			switch (c)
			{
			case '"': out += "\\\""; break;
			case '\\': out += "\\\\"; break;
			case '\n': out += "\\n"; break;
			case '\r': out += "\\r"; break;
			case '\t': out += "\\t"; break;
			default:
				if (c < 0x20)
					out += std::format("\\u{:04x}", c);
				else
					out += (char)c;
			}
		}
		return out;
	}

	std::string id_json(const Json& id)
	{
		if (id.type == Json::String)
			return "\"" + json_escape(id.text) + "\"";
		if (id.type == Json::Number)
			return id.text;
		return "null";
	}

	std::string uri_to_path(std::string_view uri)
	{
		if (uri.rfind("file:///", 0) == 0)
			uri.remove_prefix(8);

		std::string out;
		for (size_t i = 0; i < uri.size(); ++i)
		{
			if (uri[i] == '%' && i + 2 < uri.size())
			{
				out += (char)std::stoi(std::string(uri.substr(i + 1, 2)), nullptr, 16);
				i += 2;
			}
			else
				out += uri[i] == '/' ? '\\' : uri[i];
		}
		return out;
	}

	std::string path_to_uri(const std::filesystem::path& p)
	{
		return "file:///" + p.generic_string();
	}

	// Case-insensitive key: VS and the filesystem disagree on drive-letter case.
	std::string key_of(const std::filesystem::path& p)
	{
		std::string s = std::filesystem::absolute(p).lexically_normal().generic_string();
		std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return (char)std::tolower(c); });
		return s;
	}

	std::string read_file(const std::filesystem::path& p)
	{
		std::ifstream f(p, std::ios::binary);
		return std::string(std::istreambuf_iterator<char>(f), {});
	}

	class Server
	{
		struct OpenDoc
		{
			std::string uri;
			std::string text;
		};

		std::map<std::string, OpenDoc> open_docs;         // key_of(path) -> buffer
		std::map<std::string, Parsed> last_good;          // key_of(path) -> last parse without syntax errors
		std::map<std::string, std::string> published_uri; // key_of(path) -> uri that currently shows diagnostics
		std::filesystem::path root;                       // the defs/ directory being validated

		// State of the last revalidate(), which definition/completion answer from.
		// A file with syntax errors is represented by its last good parse.
		Parsed model;
		std::map<std::string, std::filesystem::path> files; // key_of(path) -> path
		std::map<std::string, std::string> texts;           // key_of(path) -> text that was validated

		// What was last published per file, kept so a codeAction request can
		// find the fix behind the diagnostic the user clicked.
		struct PublishedDiag
		{
			size_t line, c0, c1;
			std::string message;
			std::optional<Diagnostics::Fix> fix;
		};
		std::map<std::string, std::vector<PublishedDiag>> file_diags;

		bool dirty = false;
		bool shutting_down = false;

		// Validation is cross-file (a condition in one .prism names a struct from
		// another), so the root is the enclosing defs/ directory, not the file.
		void adopt_root(const std::filesystem::path& file)
		{
			if (!root.empty() && key_of(file).rfind(key_of(root) + "/", 0) == 0)
				return;

			std::filesystem::path dir = file.parent_path();
			for (auto p = dir; !p.empty() && p != p.root_path(); p = p.parent_path())
			{
				std::string name = p.filename().string();
				std::transform(name.begin(), name.end(), name.begin(), [](unsigned char c) { return (char)std::tolower(c); });
				if (name == "defs")
				{
					dir = p;
					break;
				}
			}
			root = dir;
			last_good.clear();
		}

		static std::pair<size_t, size_t> word_range(const std::string& text, size_t line, size_t column)
		{
			size_t pos = 0;
			for (size_t l = 1; l < line && pos != std::string::npos; ++l)
			{
				pos = text.find('\n', pos);
				if (pos != std::string::npos)
					++pos;
			}
			if (pos == std::string::npos)
				return { column, column + 1 };

			size_t start = pos + column;
			size_t end = start;
			while (end < text.size() && (std::isalnum((unsigned char)text[end]) || text[end] == '_'))
				++end;
			return { column, column + std::max<size_t>(end - start, 1) };
		}

		void revalidate()
		{
			dirty = false;
			if (root.empty())
				return;

			diagnostics().clear();

			files.clear();
			texts.clear();
			std::set<std::string> broken;
			Parsed& merged = model = Parsed{};

			std::error_code ec;
			for (auto it = std::filesystem::recursive_directory_iterator(root, ec);
			     !ec && it != std::filesystem::recursive_directory_iterator(); it.increment(ec))
			{
				if (!it->is_regular_file() || it->path().extension() != ".prism")
					continue;

				std::filesystem::path path = std::filesystem::absolute(it->path()).lexically_normal();
				std::string key = key_of(path);
				auto doc = open_docs.find(key);
				const std::string& text = texts[key] = doc != open_docs.end() ? doc->second.text : read_file(path);
				files[key] = path;

				size_t before = diagnostics().count();
				Parsed p;
				try
				{
					p = parse_text(text, path.string());
				}
				catch (std::exception& e)
				{
					diagnostics().error(SourceLocation{ path.string(), 1, 1 }, e.what());
				}

				// A half-typed file would otherwise vanish from the merged model
				// and make every other file's references to it look broken.
				if (diagnostics().count() != before)
				{
					broken.insert(key);
					if (auto good = last_good.find(key); good != last_good.end())
					{
						Parsed copy = good->second;
						merged.merge(copy);
					}
				}
				else
				{
					last_good[key] = p;
					merged.merge(p);
				}
			}

			size_t parse_errors = diagnostics().count();
			try
			{
				validate(merged);
			}
			catch (std::exception& e)
			{
				diagnostics().error(SourceLocation{}, std::string("internal: ") + e.what());
			}

			file_diags.clear();
			std::map<std::string, std::vector<std::string>> per_file;
			const auto& all = diagnostics().entries();
			for (size_t i = 0; i < all.size(); ++i)
			{
				const auto& d = all[i];
				if (d.loc.file.empty())
					continue;

				std::string key = key_of(d.loc.file);
				// Validation of a file with syntax errors ran against its last
				// good version, so those locations are stale; only its syntax
				// errors are current.
				if (i >= parse_errors && broken.count(key))
					continue;

				size_t line = d.loc.line ? d.loc.line - 1 : 0;
				size_t col = d.loc.column ? d.loc.column - 1 : 0;
				auto [c0, c1] = d.fix ? std::pair<size_t, size_t>{ col, col + d.fix->length }
				              : texts.count(key) ? word_range(texts[key], d.loc.line, col) : std::pair<size_t, size_t>{ col, col + 1 };

				file_diags[key].push_back({ line, c0, c1, d.message, d.fix });
				per_file[key].push_back(std::format(
					R"({{"range":{{"start":{{"line":{},"character":{}}},"end":{{"line":{},"character":{}}}}},"severity":1,"source":"sig","message":"{}"}})",
					line, c0, line, c1, json_escape(d.message)));
			}

			std::map<std::string, std::string> now_published;
			auto publish = [&](const std::string& uri, const std::vector<std::string>& diags)
			{
				std::string list;
				for (const auto& d : diags)
					list += (list.empty() ? "" : ",") + d;
				write_message(std::format(R"({{"jsonrpc":"2.0","method":"textDocument/publishDiagnostics","params":{{"uri":"{}","diagnostics":[{}]}}}})",
					json_escape(uri), list));
			};

			for (const auto& [key, diags] : per_file)
			{
				auto doc = open_docs.find(key);
				std::string uri = doc != open_docs.end() ? doc->second.uri
				                : files.count(key) ? path_to_uri(files[key]) : path_to_uri(key);
				publish(uri, diags);
				now_published[key] = uri;
			}

			// Clear files that had diagnostics last time and have none now.
			for (const auto& [key, uri] : published_uri)
				if (!now_published.count(key))
					publish(uri, {});

			published_uri = std::move(now_published);
		}

		// ---- navigation: go-to-definition and completion ----------------------

		struct Symbol
		{
			std::string name;
			std::string detail;
			int kind = 0; // LSP CompletionItemKind
			SourceLocation loc;
		};

		static constexpr int K_Method = 2, K_Field = 5, K_Class = 7, K_Module = 9, K_Property = 10, K_Enum = 13,
		                     K_Keyword = 14, K_File = 17, K_EnumMember = 20, K_Constant = 21, K_Struct = 22;

		// Declaration keyword -> KNOWN_OPTIONS kind of that declaration.
		inline static const std::map<std::string, std::string> DECL_KIND = {
			{ "struct", "struct" }, { "PassNode", "PassNode" }, { "PassView", "view" }, { "layout", "layout" },
			{ "rt", "render target" }, { "enum", "enum" }, { "Pipeline", "Pipeline" },
			{ "ComputePSO", "ComputePSO" }, { "GraphicsPSO", "GraphicsPSO" }, { "WorkgraphPSO", "WorkgraphPSO" },
			{ "RaytracePSO", "RaytracePSO" }, { "RaytraceRaygen", "RaytraceRaygen" }, { "RaytracePass", "RaytracePass" },
		};

		// Declaration keyword -> KNOWN_OPTIONS kind of the things declared in its body.
		inline static const std::map<std::string, std::string> BODY_KIND = {
			{ "struct", "struct field" }, { "PassNode", "PassNode field" }, { "PassView", "view field" },
			{ "layout", "slot" }, { "Pipeline", "pipeline entry" },
		};

		// Mirrors Prism.g4's shader_type rule.
		inline static const std::set<std::string> SHADER_STAGES = {
			"compute", "vertex", "pixel", "domain", "hull", "geometry", "miss", "closest_hit",
			"any_hit", "raygen", "amplification", "mesh", "shader",
		};

		static bool is_ident(char c)
		{
			return std::isalnum((unsigned char)c) || c == '_';
		}

		static size_t offset_of(const std::string& text, size_t line0, size_t ch)
		{
			size_t pos = 0;
			for (size_t l = 0; l < line0; ++l)
			{
				pos = text.find('\n', pos);
				if (pos == std::string::npos)
					return text.size();
				++pos;
			}
			size_t eol = text.find('\n', pos);
			return std::min(pos + ch, eol == std::string::npos ? text.size() : eol);
		}

		struct Word
		{
			size_t begin = 0, end = 0;
			std::string text;
			std::string owner; // identifier before the separator, if any
			char separator = 0; // ':' for Owner::word, '.' for owner.word
		};

		// `whole` takes the identifier around the cursor (definition); otherwise
		// only the part before it, which is what completion filters on.
		static Word word_at(const std::string& text, size_t offset, bool whole)
		{
			Word w;
			w.begin = offset;
			while (w.begin > 0 && is_ident(text[w.begin - 1]))
				--w.begin;
			w.end = offset;
			if (whole)
				while (w.end < text.size() && is_ident(text[w.end]))
					++w.end;
			w.text = text.substr(w.begin, w.end - w.begin);

			size_t p = w.begin;
			while (p > 0 && (text[p - 1] == ' ' || text[p - 1] == '\t'))
				--p;
			if (p >= 2 && text[p - 1] == ':' && text[p - 2] == ':')
			{
				w.separator = ':';
				p -= 2;
			}
			else if (p >= 1 && text[p - 1] == '.')
			{
				w.separator = '.';
				p -= 1;
			}
			if (w.separator)
			{
				while (p > 0 && (text[p - 1] == ' ' || text[p - 1] == '\t'))
					--p;
				size_t e = p;
				while (p > 0 && is_ident(text[p - 1]))
					--p;
				w.owner = text.substr(p, e - p);
			}
			return w;
		}

		// Walks text[0, offset) skipping comments, strings, backtick spans and
		// %{ }% blocks, and returns the top-level declaration whose body the
		// offset is inside ({keyword, name}; empty at top level).
		static std::pair<std::string, std::string> enclosing_decl(const std::string& text, size_t offset)
		{
			std::pair<std::string, std::string> current, pending;
			bool want_name = false;
			int depth = 0;

			for (size_t i = 0; i < offset; ++i)
			{
				char c = text[i];
				if (c == '#')
				{
					while (i < offset && text[i] != '\n')
						++i;
				}
				else if (c == '`' || c == '"')
				{
					for (++i; i < offset && text[i] != c; ++i) {}
				}
				else if (c == '%' && i + 1 < offset && text[i + 1] == '{')
				{
					size_t end = text.find("}%", i + 2);
					i = end == std::string::npos ? offset : end + 1;
				}
				else if (c == '{')
				{
					if (depth++ == 0)
						current = pending;
				}
				else if (c == '}')
				{
					if (depth > 0 && --depth == 0)
						current = pending = {};
				}
				else if (is_ident(c) && depth == 0)
				{
					size_t b = i;
					while (i < offset && is_ident(text[i]))
						++i;
					std::string id = text.substr(b, i - b);
					--i;
					if (want_name)
					{
						pending.second = id;
						want_name = false;
					}
					else if (DECL_KIND.count(id))
					{
						pending = { id, "" };
						want_name = true;
					}
				}
			}
			return depth > 0 ? current : std::pair<std::string, std::string>{};
		}

		// Where the declaration that the option block(s) at `offset` belong to
		// starts; npos if there is none.
		static size_t after_options(const std::string& text, size_t offset)
		{
			// While typing, the block at the cursor is usually still unclosed; a
			// plain find(']') would jump to the next block and read the wrong
			// declaration. Stop at the end of the line instead.
			size_t i = text.find_first_of("]\n", offset);
			if (i == std::string::npos)
				return std::string::npos;
			for (++i; i < text.size();)
			{
				char c = text[i];
				if (std::isspace((unsigned char)c))
					++i;
				else if (c == '#')
					while (i < text.size() && text[i] != '\n')
						++i;
				else if (c == '[')
				{
					size_t end = text.find(']', i);
					if (end == std::string::npos)
						return std::string::npos;
					i = end + 1;
				}
				else
					return i;
			}
			return std::string::npos;
		}

		// The first word of that declaration.
		static std::string word_after_options(const std::string& text, size_t offset)
		{
			size_t b = after_options(text, offset);
			if (b == std::string::npos)
				return {};
			size_t e = b;
			while (e < text.size() && is_ident(text[e]))
				++e;
			return text.substr(b, e - b);
		}

		// KNOWN_OPTIONS kind for an option name typed at `offset`.
		static std::string option_kind_at(const std::string& text, size_t offset)
		{
			auto [keyword, name] = enclosing_decl(text, offset);
			std::string next = word_after_options(text, offset);

			if (keyword.empty())
			{
				auto it = DECL_KIND.find(next);
				return it != DECL_KIND.end() ? it->second : "";
			}
			if (keyword == "struct")
			{
				// `ret name(` is a function; a field reaches `;` first.
				size_t b = after_options(text, offset);
				size_t stop = b == std::string::npos ? std::string::npos : text.find_first_of("(;{}", b);
				if (stop != std::string::npos && text[stop] == '(')
					return "function";
			}
			if (auto it = BODY_KIND.find(keyword); it != BODY_KIND.end())
				return it->second;

			// Inside a PSO body the options belong to whatever statement follows.
			if (next == "define") return "define";
			if (next == "rtv") return "rtv";
			if (next == "blend") return "blend";
			if (next == "NodeOutput") return "workgraph node output";
			if (SHADER_STAGES.count(next)) return "shader";
			return "PSO param";
		}

		// True when the cursor is where an option *name* goes: inside `[`, after
		// `[` or `,`, before any `=`.
		static bool at_option_name(const std::string& text, size_t offset)
		{
			size_t line_start = text.rfind('\n', offset ? offset - 1 : 0);
			line_start = line_start == std::string::npos ? 0 : line_start + 1;
			std::string_view before(text.data() + line_start, offset - line_start);

			size_t open = before.rfind('[');
			if (open == std::string_view::npos || before.find(']', open) != std::string_view::npos)
				return false;
			std::string_view seg = before.substr(open + 1);
			if (size_t comma = seg.rfind(','); comma != std::string_view::npos)
				seg = seg.substr(comma + 1);
			return seg.find('=') == std::string_view::npos && seg.find('{') == std::string_view::npos;
		}

		static bool in_comment(const std::string& text, size_t offset)
		{
			size_t line_start = text.rfind('\n', offset ? offset - 1 : 0);
			line_start = line_start == std::string::npos ? 0 : line_start + 1;
			return text.find('#', line_start) < offset;
		}

		std::vector<Symbol> top_level_symbols() const
		{
			std::vector<Symbol> out;
			auto add = [&](const auto& container, const char* detail, int kind)
			{
				for (const auto& item : container)
					out.push_back({ item.name, detail, kind, item.name_loc });
			};
			add(model.tables, "struct", K_Struct);
			add(model.enums, "enum", K_Enum);
			add(model.layouts, "layout", K_Module);
			add(model.views, "PassView", K_Class);
			add(model.passes, "PassNode", K_Class);
			add(model.compute_pso, "ComputePSO", K_Class);
			add(model.graphics_pso, "GraphicsPSO", K_Class);
			add(model.workgraph_pso, "WorkgraphPSO", K_Class);
			add(model.raytrace_pso, "RaytracePSO", K_Class);
			add(model.raytrace_gen, "RaytraceRaygen", K_Class);
			add(model.raytrace_pass, "RaytracePass", K_Class);
			add(model.rt, "render target", K_Class);
			add(model.pipelines, "Pipeline", K_Module);
			add(model.consts, "const", K_Constant);
			return out;
		}

		void collect_members(const std::string& owner, std::vector<Symbol>& out, int depth = 0) const
		{
			if (depth > 16)
				return;

			if (const Table* t = model.tables.find(owner))
			{
				for (const auto& v : t->values)
					out.push_back({ v.name, v.get_type(), K_Field, v.name_loc });
				// Functions carry their whole signature as detail.
				for (const auto& f : t->functions)
					out.push_back({ f.name, f.get_type() + " " + f.name + "(" + f.params + ")", K_Method, f.name_loc });
				for (const auto& parent : t->parent)
					collect_members(parent, out, depth + 1);
			}
			if (const Enum* e = model.enums.find(owner))
				for (const auto& v : e->values)
					out.push_back({ v.name, owner, K_EnumMember, v.name_loc });
			if (const Layout* l = model.layouts.find(owner))
			{
				for (const auto& s : l->slots)
					out.push_back({ s.name, "slot", K_Field, s.name_loc });
				for (const auto& s : l->samplers)
					out.push_back({ s.name, "sampler", K_Field, s.name_loc });
				for (const auto& parent : l->parent)
					collect_members(parent, out, depth + 1);
			}
			auto add_params = [&](const View* v)
			{
				for (const auto& p : v->params)
					out.push_back({ p.name, p.get_type(), K_Field, p.name_loc });
				if (v->find_option("Multiple"))
					out.push_back({ "pass_index", "uint32_t, implicit on [Multiple]", K_Field, {} });
			};
			if (const Pass* p = model.passes.find(owner))
				add_params(p);
			if (const View* v = model.views.find(owner))
				add_params(v);
		}

		std::string uri_for_file(const std::string& file) const
		{
			std::string key = key_of(file);
			if (auto doc = open_docs.find(key); doc != open_docs.end())
				return doc->second.uri;
			return path_to_uri(std::filesystem::path(file));
		}

		std::string location_json(const SourceLocation& loc, size_t length) const
		{
			size_t line = loc.line ? loc.line - 1 : 0, col = loc.column ? loc.column - 1 : 0;
			return std::format(R"({{"uri":"{}","range":{{"start":{{"line":{},"character":{}}},"end":{{"line":{},"character":{}}}}}}})",
				json_escape(uri_for_file(loc.file)), line, col, line, col + length);
		}

		const std::string* doc_text(const std::string& uri)
		{
			std::string key = key_of(uri_to_path(uri));
			if (auto doc = open_docs.find(key); doc != open_docs.end())
				return &doc->second.text;
			if (auto t = texts.find(key); t != texts.end())
				return &t->second;
			return nullptr;
		}

		// `<stage> = "<path>"` with the cursor inside the quotes (the closing
		// quote may not be typed yet).
		struct ShaderString
		{
			std::string stage;
			size_t content_begin = 0; // offset of the first character after the opening quote
			std::string content;      // the whole quoted text
		};

		static std::optional<ShaderString> shader_string_at(const std::string& text, size_t offset)
		{
			size_t ls = text.rfind('\n', offset ? offset - 1 : 0);
			ls = ls == std::string::npos ? 0 : ls + 1;
			size_t le = text.find('\n', offset);
			le = le == std::string::npos ? text.size() : le;

			size_t i = ls;
			auto skip_ws = [&] { while (i < le && (text[i] == ' ' || text[i] == '\t')) ++i; };
			skip_ws();
			size_t sb = i;
			while (i < le && is_ident(text[i])) ++i;
			ShaderString s;
			s.stage = text.substr(sb, i - sb);
			skip_ws();
			if (!SHADER_STAGES.count(s.stage) || i >= le || text[i] != '=')
				return {};
			++i;
			skip_ws();
			if (i >= le || text[i] != '"')
				return {};

			s.content_begin = i + 1;
			size_t close = text.find('"', s.content_begin);
			size_t content_end = close == std::string::npos || close > le ? le : close;
			if (offset < s.content_begin || offset > content_end)
				return {};
			s.content = text.substr(s.content_begin, content_end - s.content_begin);
			return s;
		}

		// When the cursor is on an option's value (`[Always = Re|`), that
		// option's name; not inside a `{...}` value list.
		static std::optional<std::string> option_value_at(const std::string& text, size_t offset)
		{
			size_t line_start = text.rfind('\n', offset ? offset - 1 : 0);
			line_start = line_start == std::string::npos ? 0 : line_start + 1;
			std::string_view before(text.data() + line_start, offset - line_start);

			size_t open = before.rfind('[');
			if (open == std::string_view::npos || before.find(']', open) != std::string_view::npos)
				return {};

			std::string_view seg = before.substr(open + 1);
			int braces = 0;
			size_t item = 0;
			for (size_t i = 0; i < seg.size(); ++i)
			{
				if (seg[i] == '{') ++braces;
				else if (seg[i] == '}') --braces;
				else if (seg[i] == ',' && braces == 0) item = i + 1;
			}
			if (braces != 0)
				return {};

			seg = seg.substr(item);
			size_t eq = seg.find('=');
			if (eq == std::string_view::npos || (eq + 1 < seg.size() && seg[eq + 1] == '='))
				return {};

			std::string_view name = seg.substr(0, eq);
			while (!name.empty() && std::isspace((unsigned char)name.front())) name.remove_prefix(1);
			while (!name.empty() && std::isspace((unsigned char)name.back())) name.remove_suffix(1);
			if (name.empty() || !std::all_of(name.begin(), name.end(), is_ident))
				return {};
			return std::string(name);
		}

		template <class F>
		void for_each_option_holder(F&& f) const
		{
			auto pso = [&](const PSO& p)
			{
				f(p);
				for (const auto& s : p.shaders) f(s);
				for (const auto& d : p.defines) f(d);
			};
			for (const auto& t : model.tables) { f(t); for (const auto& v : t.values) f(v); }
			for (const auto& l : model.layouts) { f(l); for (const auto& s : l.slots) f(s); }
			for (const auto& r : model.rt) f(r);
			for (const auto& p : model.compute_pso) pso(p);
			for (const auto& p : model.graphics_pso) { pso(p); for (const auto& x : p.params) f(x); }
			for (const auto& p : model.workgraph_pso)
			{
				pso(p);
				for (const auto& n : p.nodes) { for (const auto& x : n.params) f(x); for (const auto& o : n.outputs) f(o); }
			}
			for (const auto& p : model.raytrace_pso) pso(p);
			for (const auto& p : model.raytrace_gen) pso(p);
			for (const auto& p : model.raytrace_pass) { pso(p); for (const auto& x : p.params) f(x); }
			for (const auto& v : model.views) { f(v); for (const auto& x : v.params) f(x); }
			for (const auto& v : model.passes) { f(v); for (const auto& x : v.params) f(x); }
			for (const auto& p : model.pipelines) for (const auto& e : p.entries) f(e);
		}

		// Every value some .prism already gives this option. No hand-kept lists:
		// [Always] offers the flags actually in use, [Bind] the slots, [Format]
		// the formats.
		std::set<std::string> values_used_for(const std::string& option_name) const
		{
			std::set<std::string> out;
			for_each_option_holder([&](const have_options& holder)
			{
				const option* o = holder.find_option(option_name);
				if (!o || o->value_atom.is_raw)
					return;
				const auto& atom = o->value_atom;
				if (!atom.values.empty())
				{
					for (const auto& v : atom.values)
						if (!v.expr.empty() && !v.is_raw)
							out.insert(v.expr);
				}
				else if (atom.terms.size() <= 1 && !atom.expr.empty())
					out.insert(atom.owner_name.empty() ? atom.expr : atom.owner_name + "::" + atom.expr);
			});
			return out;
		}

		std::vector<std::string> shader_files(const std::filesystem::path& shaders) const
		{
			std::vector<std::string> out;
			std::error_code ec;
			for (auto it = std::filesystem::recursive_directory_iterator(shaders, ec);
			     !ec && it != std::filesystem::recursive_directory_iterator(); it.increment(ec))
				if (it->is_regular_file() && it->path().extension() == ".hlsl")
					out.push_back(std::filesystem::relative(it->path(), shaders).generic_string());
			std::sort(out.begin(), out.end());
			return out;
		}

		// ---- hover -------------------------------------------------------------

		std::string members_markdown(const std::string& owner) const
		{
			std::vector<Symbol> members;
			collect_members(owner, members);
			std::string out;
			size_t shown = 0;
			for (const auto& m : members)
			{
				if (shown++ == 40)
				{
					out += std::format("\n... {} more", members.size() - 40);
					break;
				}
				out += "\n" + (m.kind == K_Method ? m.detail + ";"
				             : m.detail.empty() || m.kind == K_EnumMember ? m.name : m.detail + " " + m.name);
			}
			return out;
		}

		std::string where(const SourceLocation& loc) const
		{
			return loc.line ? std::format("{}:{}", std::filesystem::path(loc.file).filename().string(), loc.line) : "";
		}

		std::string hover(const Json& params)
		{
			if (dirty)
				revalidate();

			std::string uri = params["textDocument"]["uri"].text;
			const std::string* text = doc_text(uri);
			if (!text)
				return "null";
			size_t offset = offset_of(*text, std::stoul(params["position"]["line"].text), std::stoul(params["position"]["character"].text));

			auto reply = [](const std::string& markdown)
			{
				return std::format(R"({{"contents":{{"kind":"markdown","value":"{}"}}}})", json_escape(markdown));
			};

			if (auto s = shader_string_at(*text, offset))
			{
				auto file = (shaders_root(uri_to_path(uri)) / s->content).lexically_normal();
				bool exists = !s->content.empty() && std::filesystem::is_regular_file(file);
				return reply(std::format("**{}** shader\n\n`{}`{}", s->stage, file.string(), exists ? "" : "\n\n*file not found*"));
			}

			Word w = word_at(*text, offset, true);
			if (w.text.empty())
				return "null";

			if (at_option_name(*text, w.begin))
			{
				std::vector<std::string> kinds = option_kinds(w.text);
				if (kinds.empty())
					return reply(std::format("`[{}]` is not a known option", w.text));
				std::string used;
				size_t n = 0;
				for (const auto& v : values_used_for(w.text))
				{
					if (n++ == 12) { used += ", ..."; break; }
					used += (used.empty() ? "" : ", ") + ("`" + v + "`");
				}
				std::string on;
				for (const auto& k : kinds)
					on += (on.empty() ? "" : ", ") + k;
				return reply(std::format("option `[{}]`\n\naccepted on: {}{}", w.text, on, used.empty() ? "" : "\n\nvalues in use: " + used));
			}

			std::vector<Symbol> members;
			if (w.separator == ':')
				collect_members(w.owner, members);
			else
				collect_members(enclosing_decl(*text, offset).second, members); // data.x, exists(x), and plain names in a struct

			// Every match, so all overloads of a function show together.
			std::string lines, first_where;
			for (const auto& m : members)
				if (m.name == w.text)
				{
					lines += (lines.empty() ? "" : "\n") + (m.kind == K_Method ? m.detail
						: (m.kind == K_EnumMember ? "" : m.detail + " ") + m.name);
					if (first_where.empty())
						first_where = where(m.loc);
				}
			if (!lines.empty())
				return reply(std::format("```\n{}\n```\n{}", lines, first_where));

			for (const auto& s : top_level_symbols())
				if (s.name == w.text)
				{
					std::string body = members_markdown(s.name);
					return reply(std::format("```\n{} {}{}\n```\n{}", s.detail, s.name, body.empty() ? "" : "\n{" + body + "\n}", where(s.loc)));
				}
			return "null";
		}

		// ---- quick fixes -----------------------------------------------------------

		std::string code_actions(const Json& params)
		{
			if (dirty)
				revalidate();

			std::string uri = params["textDocument"]["uri"].text;
			size_t first = std::stoul(params["range"]["start"]["line"].text);
			size_t last = std::stoul(params["range"]["end"]["line"].text);

			auto it = file_diags.find(key_of(uri_to_path(uri)));
			if (it == file_diags.end())
				return "[]";

			std::string list;
			for (const auto& d : it->second)
			{
				if (!d.fix || d.line < first || d.line > last)
					continue;
				std::string range = std::format(R"({{"start":{{"line":{},"character":{}}},"end":{{"line":{},"character":{}}}}})", d.line, d.c0, d.line, d.c1);
				list += (list.empty() ? "" : ",") + std::format(
					R"({{"title":"Change to '{}'","kind":"quickfix","isPreferred":true,)"
					R"("diagnostics":[{{"range":{},"severity":1,"source":"sig","message":"{}"}}],)"
					R"("edit":{{"changes":{{"{}":[{{"range":{},"newText":"{}"}}]}}}}}})",
					json_escape(d.fix->replacement), range, json_escape(d.message), json_escape(uri), range, json_escape(d.fix->replacement));
			}
			return "[" + list + "]";
		}

		// ---- outline and symbol search ---------------------------------------------

		struct Decl
		{
			std::string name, detail;
			int kind; // LSP SymbolKind
			SourceLocation start, name_loc;
		};

		std::vector<Decl> declarations() const
		{
			std::vector<Decl> out;
			auto add = [&](const auto& container, const char* detail, int kind)
			{
				for (const auto& item : container)
					out.push_back({ item.name, detail, kind, item.loc, item.name_loc });
			};
			add(model.tables, "struct", 23);
			add(model.enums, "enum", 10);
			add(model.layouts, "layout", 3);
			add(model.views, "PassView", 5);
			add(model.passes, "PassNode", 5);
			add(model.compute_pso, "ComputePSO", 5);
			add(model.graphics_pso, "GraphicsPSO", 5);
			add(model.workgraph_pso, "WorkgraphPSO", 5);
			add(model.raytrace_pso, "RaytracePSO", 5);
			add(model.raytrace_gen, "RaytraceRaygen", 5);
			add(model.raytrace_pass, "RaytracePass", 5);
			add(model.rt, "render target", 5);
			add(model.pipelines, "Pipeline", 4);
			add(model.consts, "const", 14);
			return out;
		}

		static std::string range_json(size_t l0, size_t c0, size_t l1, size_t c1)
		{
			return std::format(R"({{"start":{{"line":{},"character":{}}},"end":{{"line":{},"character":{}}}}})", l0, c0, l1, c1);
		}

		std::string document_symbols(const Json& params)
		{
			if (dirty)
				revalidate();

			std::string key = key_of(uri_to_path(params["textDocument"]["uri"].text));
			std::vector<Decl> mine;
			for (auto& d : declarations())
				if (d.name_loc.line && key_of(d.name_loc.file) == key)
					mine.push_back(d);
			std::sort(mine.begin(), mine.end(), [](const Decl& a, const Decl& b) { return a.start.line < b.start.line; });

			size_t last_line = texts.count(key) ? (size_t)std::count(texts[key].begin(), texts[key].end(), '\n') : 0;
			std::string list;
			for (size_t i = 0; i < mine.size(); ++i)
			{
				const Decl& d = mine[i];
				size_t l0 = d.start.line - 1;
				// The next declaration's start bounds this one; the grammar
				// records no end positions.
				size_t l1 = i + 1 < mine.size() ? std::max(l0, mine[i + 1].start.line - 2) : last_line;
				size_t nl = d.name_loc.line - 1, nc = d.name_loc.column - 1;

				std::vector<Symbol> members;
				collect_members(d.name, members);
				std::string children;
				for (const auto& m : members)
				{
					if (!m.loc.line || key_of(m.loc.file) != key)
						continue;
					size_t ml = m.loc.line - 1, mc = m.loc.column - 1;
					children += (children.empty() ? "" : ",") + std::format(R"({{"name":"{}","detail":"{}","kind":{},"range":{},"selectionRange":{}}})",
						json_escape(m.name), json_escape(m.detail), m.kind == K_EnumMember ? 22 : m.kind == K_Method ? 6 : 8,
						range_json(ml, mc, ml, mc + m.name.size()), range_json(ml, mc, ml, mc + m.name.size()));
				}

				list += (list.empty() ? "" : ",") + std::format(R"({{"name":"{}","detail":"{}","kind":{},"range":{},"selectionRange":{},"children":[{}]}})",
					json_escape(d.name), d.detail, d.kind, range_json(l0, 0, l1, 0), range_json(nl, nc, nl, nc + d.name.size()), children);
			}
			return "[" + list + "]";
		}

		std::string workspace_symbols(const Json& params)
		{
			if (dirty)
				revalidate();

			std::string query = params["query"].text;
			std::transform(query.begin(), query.end(), query.begin(), [](unsigned char c) { return (char)std::tolower(c); });

			std::string list;
			for (const auto& d : declarations())
			{
				std::string lower = d.name;
				std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) { return (char)std::tolower(c); });
				if (!d.name_loc.line || lower.find(query) == std::string::npos)
					continue;
				list += (list.empty() ? "" : ",") + std::format(R"({{"name":"{}","kind":{},"containerName":"{}","location":{}}})",
					json_escape(d.name), d.kind, d.detail, location_json(d.name_loc, d.name.size()));
			}
			return "[" + list + "]";
		}

		// ---- semantic tokens: colour user-declared names wherever they are used ----

		// Order is the legend sent in initialize.
		enum TokenType { T_Struct, T_Enum, T_Class, T_Namespace, T_EnumMember, T_Property, T_Variable };

		std::string semantic_tokens(const Json& params)
		{
			if (dirty)
				revalidate();

			const std::string* text = doc_text(params["textDocument"]["uri"].text);
			if (!text)
				return R"({"data":[]})";

			std::map<std::string, int> types;
			for (const auto& d : declarations())
				types.emplace(d.name, d.kind == 23 ? T_Struct : d.kind == 10 ? T_Enum : d.kind == 3 ? T_Namespace
				                    : d.kind == 14 ? T_Variable : T_Class);

			std::string data;
			size_t prev_line = 0, prev_col = 0, line = 0, line_start = 0;
			auto emit = [&](size_t begin, size_t length, int type)
			{
				size_t col = begin - line_start;
				size_t dl = line - prev_line;
				size_t dc = dl == 0 ? col - prev_col : col;
				data += (data.empty() ? "" : ",") + std::format("{},{},{},{},0", dl, dc, length, type);
				prev_line = line;
				prev_col = col;
			};

			const std::string& t = *text;
			for (size_t i = 0; i < t.size(); ++i)
			{
				char c = t[i];
				if (c == '\n')
				{
					++line;
					line_start = i + 1;
				}
				else if (c == '#' || (c == '/' && i + 1 < t.size() && t[i + 1] == '/'))
				{
					// # comments in Prism, // comments inside HLSL function bodies.
					while (i + 1 < t.size() && t[i + 1] != '\n')
						++i;
				}
				else if (c == '"')
				{
					while (i + 1 < t.size() && t[i + 1] != '"' && t[i + 1] != '\n')
						++i;
					++i;
				}
				else if (is_ident(c) && !std::isdigit((unsigned char)c))
				{
					size_t b = i;
					while (i + 1 < t.size() && is_ident(t[i + 1]))
						++i;
					std::string id = t.substr(b, i - b + 1);

					// `Owner::member`: colour the member by what the owner is.
					size_t p = b;
					while (p > 0 && (t[p - 1] == ' ' || t[p - 1] == '\t')) --p;
					if (p >= 2 && t[p - 1] == ':' && t[p - 2] == ':')
					{
						size_t e = p - 2;
						while (e > 0 && (t[e - 1] == ' ' || t[e - 1] == '\t')) --e;
						size_t ob = e;
						while (ob > 0 && is_ident(t[ob - 1])) --ob;
						std::string owner = t.substr(ob, e - ob);
						if (model.enums.find(owner))
							emit(b, id.size(), T_EnumMember);
						else if (model.tables.find(owner) || model.layouts.find(owner))
							emit(b, id.size(), T_Property);
						continue;
					}

					if (auto it = types.find(id); it != types.end())
						emit(b, id.size(), it->second);
				}
			}
			return R"({"data":[)" + data + "]}";
		}

		std::string definition(const Json& params)
		{
			if (dirty)
				revalidate();

			std::string uri = params["textDocument"]["uri"].text;
			const std::string* text = doc_text(uri);
			if (!text)
				return "null";

			size_t offset = offset_of(*text, std::stoul(params["position"]["line"].text), std::stoul(params["position"]["character"].text));

			// `compute = "occlusion/downsample_depth.hlsl";` -> the file itself
			if (auto s = shader_string_at(*text, offset))
			{
				auto hlsl = shaders_root(uri_to_path(uri)) / s->content;
				if (!s->content.empty() && std::filesystem::is_regular_file(hlsl))
					return std::format(R"({{"uri":"{}","range":{{"start":{{"line":0,"character":0}},"end":{{"line":0,"character":0}}}}}})",
						json_escape(uri_for_file(hlsl.lexically_normal().string())));
				return "null";
			}

			Word w = word_at(*text, offset, true);
			if (w.text.empty())
				return "null";

			auto first_located = [&](const std::vector<Symbol>& symbols) -> std::string
			{
				for (const auto& s : symbols)
					if (s.name == w.text && s.loc.line)
						return location_json(s.loc, s.name.size());
				return {};
			};

			std::vector<Symbol> candidates;
			if (w.separator == ':')
				collect_members(w.owner, candidates);
			else if (w.separator == '.' && w.owner == "data")
				collect_members(enclosing_decl(*text, offset).second, candidates);
			else if (w.begin >= 7 && text->compare(w.begin - 7, 7, "exists(") == 0)
				collect_members(enclosing_decl(*text, offset).second, candidates);
			else
			{
				// Innermost scope first: inside a struct, a function body naming
				// another member (LogWrite, voxels_per_tile) means that member.
				collect_members(enclosing_decl(*text, offset).second, candidates);
				auto top = top_level_symbols();
				candidates.insert(candidates.end(), top.begin(), top.end());
			}

			std::string found = first_located(candidates);
			return found.empty() ? "null" : found;
		}

		std::string completion(const Json& params)
		{
			if (dirty)
				revalidate();

			std::string uri = params["textDocument"]["uri"].text;
			const std::string* text = doc_text(uri);
			if (!text)
				return R"({"isIncomplete":false,"items":[]})";

			size_t offset = offset_of(*text, std::stoul(params["position"]["line"].text), std::stoul(params["position"]["character"].text));
			if (in_comment(*text, offset))
				return R"({"isIncomplete":false,"items":[]})";

			// Shader file dropdown. The edit replaces everything typed so far
			// inside the quotes, so a pick works even though '/' breaks VS's
			// notion of the current word.
			if (auto s = shader_string_at(*text, offset))
			{
				size_t line0 = std::stoul(params["position"]["line"].text);
				size_t line_start = text->rfind('\n', offset ? offset - 1 : 0);
				line_start = line_start == std::string::npos ? 0 : line_start + 1;
				size_t from = s->content_begin - line_start, to = offset - line_start;

				std::string list;
				for (const auto& f : shader_files(shaders_root(uri_to_path(uri))))
					list += (list.empty() ? "" : ",") + std::format(
						R"({{"label":"{}","kind":{},"filterText":"{}","textEdit":{{"range":{},"newText":"{}"}}}})",
						json_escape(f), K_File, json_escape(f), range_json(line0, from, line0, to), json_escape(f));
				return R"({"isIncomplete":false,"items":[)" + list + "]}";
			}

			Word w = word_at(*text, offset, false);
			std::vector<Symbol> items;

			if (w.separator == ':')
				collect_members(w.owner, items);
			else if (w.separator == '.' && w.owner == "data")
				collect_members(enclosing_decl(*text, offset).second, items);
			else if (at_option_name(*text, offset))
			{
				std::string kind = option_kind_at(*text, offset);
				for (const auto& name : known_options(kind))
					items.push_back({ name, "option on " + kind, K_Property, {} });
			}
			else if (auto option = option_value_at(*text, offset))
			{
				for (const auto& v : values_used_for(*option))
					items.push_back({ v, "used for [" + *option + "]", K_Constant, {} });
			}
			else
			{
				items = top_level_symbols();
				for (const auto& kw : sig_keywords())
					items.push_back({ kw, "keyword", K_Keyword, {} });
			}

			std::set<std::string> seen;
			std::string list;
			for (const auto& s : items)
			{
				if (!seen.insert(s.name).second)
					continue;
				list += (list.empty() ? "" : ",") + std::format(R"({{"label":"{}","kind":{},"detail":"{}"}})",
					json_escape(s.name), s.kind, json_escape(s.detail));
			}
			return R"({"isIncomplete":false,"items":[)" + list + "]}";
		}

		void respond(const Json& id, const std::string& result)
		{
			write_message(R"({"jsonrpc":"2.0","id":)" + id_json(id) + R"(,"result":)" + result + "}");
		}

		void respond_error(const Json& id, int code, const std::string& message)
		{
			write_message(R"({"jsonrpc":"2.0","id":)" + id_json(id) + R"(,"error":{"code":)" + std::to_string(code)
				+ R"(,"message":")" + json_escape(message) + "\"}}");
		}

		void set_doc(const std::string& uri, std::string text)
		{
			std::filesystem::path path = uri_to_path(uri);
			adopt_root(path);
			open_docs[key_of(path)] = { uri, std::move(text) };
			dirty = true;
		}

	public:
		// Returns false when the client asked the server to exit.
		bool handle(const Json& msg)
		{
			if (msg.type != Json::Object || !msg.has("method"))
				return true; // a response to something we never send

			const std::string& method = msg["method"].text;
			bool is_request = msg.has("id");
			const Json* params = msg.has("params") ? &msg["params"] : nullptr;

			if (method == "initialize")
			{
				respond(msg["id"], R"({"capabilities":{"textDocumentSync":{"openClose":true,"change":1,"save":{"includeText":false}},)"
					R"("definitionProvider":true,"hoverProvider":true,"codeActionProvider":true,)"
					R"("documentSymbolProvider":true,"workspaceSymbolProvider":true,)"
					R"("semanticTokensProvider":{"legend":{"tokenTypes":["struct","enum","class","namespace","enumMember","property","variable"],"tokenModifiers":[]},"full":true},)"
					R"("completionProvider":{"triggerCharacters":[":",".","[",",","\"","/","="]}},)"
					R"("serverInfo":{"name":"prismc","version":"1"}})");
			}
			else if (method == "textDocument/definition" && params)
			{
				respond(msg["id"], definition(*params));
			}
			else if (method == "textDocument/completion" && params)
			{
				respond(msg["id"], completion(*params));
			}
			else if (method == "textDocument/hover" && params)
			{
				respond(msg["id"], hover(*params));
			}
			else if (method == "textDocument/codeAction" && params)
			{
				respond(msg["id"], code_actions(*params));
			}
			else if (method == "textDocument/documentSymbol" && params)
			{
				respond(msg["id"], document_symbols(*params));
			}
			else if (method == "workspace/symbol" && params)
			{
				respond(msg["id"], workspace_symbols(*params));
			}
			else if (method == "textDocument/semanticTokens/full" && params)
			{
				respond(msg["id"], semantic_tokens(*params));
			}
			else if (method == "shutdown")
			{
				shutting_down = true;
				respond(msg["id"], "null");
			}
			else if (method == "exit")
			{
				return false;
			}
			else if (method == "textDocument/didOpen" && params)
			{
				const auto& td = (*params)["textDocument"];
				set_doc(td["uri"].text, td["text"].text);
			}
			else if (method == "textDocument/didChange" && params)
			{
				const auto& changes = (*params)["contentChanges"];
				if (!changes.items.empty())
					set_doc((*params)["textDocument"]["uri"].text, changes.items.back()["text"].text);
			}
			else if (method == "textDocument/didClose" && params)
			{
				// Closed files are validated from disk from now on.
				open_docs.erase(key_of(uri_to_path((*params)["textDocument"]["uri"].text)));
				dirty = true;
			}
			else if (method == "textDocument/didSave" || method == "workspace/didChangeWatchedFiles")
			{
				dirty = true;
			}
			else if (is_request)
			{
				respond_error(msg["id"], -32601, "method not supported: " + method);
			}

			return true;
		}

		void idle()
		{
			if (dirty && !shutting_down && !input_pending())
				revalidate();
		}
	};
}

int run_lsp()
{
	Server server;
	std::string body;

	while (read_message(body))
	{
		Json msg;
		try
		{
			msg = JsonReader::parse(body);
			if (!server.handle(msg))
				return 0;
			server.idle();
		}
		catch (std::exception& e)
		{
			std::cerr << "prismc --lsp: " << e.what() << std::endl;

			// A request must always get a reply, or the client waits on it forever.
			if (msg.has("id") && msg.has("method"))
				write_message(R"({"jsonrpc":"2.0","id":)" + id_json(msg["id"]) + R"(,"error":{"code":-32603,"message":")"
					+ json_escape(e.what()) + "\"}}");
		}
	}
	return 0;
}
