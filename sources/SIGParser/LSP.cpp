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
		std::filesystem::path root;                       // the sigs/ directory being validated
		bool dirty = false;
		bool shutting_down = false;

		// Validation is cross-file (a condition in one .sig names a struct from
		// another), so the root is the enclosing sigs/ directory, not the file.
		void adopt_root(const std::filesystem::path& file)
		{
			if (!root.empty() && key_of(file).rfind(key_of(root) + "/", 0) == 0)
				return;

			std::filesystem::path dir = file.parent_path();
			for (auto p = dir; !p.empty() && p != p.root_path(); p = p.parent_path())
			{
				std::string name = p.filename().string();
				std::transform(name.begin(), name.end(), name.begin(), [](unsigned char c) { return (char)std::tolower(c); });
				if (name == "sigs")
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

			std::map<std::string, std::filesystem::path> files;
			std::map<std::string, std::string> texts;
			std::set<std::string> broken;
			Parsed merged;

			std::error_code ec;
			for (auto it = std::filesystem::recursive_directory_iterator(root, ec);
			     !ec && it != std::filesystem::recursive_directory_iterator(); it.increment(ec))
			{
				if (!it->is_regular_file() || it->path().extension() != ".sig")
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
				auto [c0, c1] = texts.count(key) ? word_range(texts[key], d.loc.line, col) : std::pair<size_t, size_t>{ col, col + 1 };

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
				respond(msg["id"], R"({"capabilities":{"textDocumentSync":{"openClose":true,"change":1,"save":{"includeText":false}}},)"
					R"("serverInfo":{"name":"sigparser","version":"1"}})");
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
		try
		{
			if (!server.handle(JsonReader::parse(body)))
				return 0;
			server.idle();
		}
		catch (std::exception& e)
		{
			std::cerr << "sigparser --lsp: " << e.what() << std::endl;
		}
	}
	return 0;
}
