export module HAL:ShaderCompiler;


import :API.ShaderCompiler;
import :Types;
import Core;

import :Slots;

export namespace HAL
{


	struct CompiledFunction
	{
		std::string name;
		std::wstring wname;
		UsedSlots slots;

		SERIALIZE()
			{
				ar& NVP(name);
				ar& NVP(wname);

				ar& NVP(slots);
			}
	};

	// Registry for shader debug Log() format strings. Compile_Shader
	// (DXC.ShaderCompiler.cpp) preprocesses each shader, rewrites every
	// Log("format", ...) call's literal string into a stable hash id, and
	// registers {id, format} here; CommandList::print_debug() looks the
	// format back up by id when it reads the GPU-written log entries back.
	struct DebugLogStrings : public Singleton<DebugLogStrings>
	{
		void register_format(uint32_t id, std::string fmt);

		// Substitutes %u/%d/%f/%x/%% in the registered format string against
		// args.x/y/z/w in order (raw bits -- the format spec says how to
		// reinterpret them, not the caller). Falls back to printing the raw
		// id + args if id was never registered.
		std::string format(uint32_t id, uint4 args) const;

	private:
		DebugLogStrings() = default;
		friend class Singleton<DebugLogStrings>;

		mutable std::mutex m;
		std::unordered_map<uint32_t, std::string> strings;
	};

	struct CompiledShader
	{
		std::vector<CompiledFunction> functions;
		binary blob;
		std::string entry_point; // name passed to -E (e.g. "CS", "VS", "PS")

		// Log("format", ...) id -> format string discovered while compiling
		// this shader (see rewrite_debug_log_strings, DXC.ShaderCompiler.cpp).
		// Persisted alongside the compiled blob because the shader cache
		// (Shader<T>::SERIALIZE, HAL.Shader.ixx) can load a cached blob
		// straight from disk without ever calling Compile_Shader again --
		// without this, DebugLogStrings (an in-memory-only registry) would
		// come back empty on a cache hit and every Log() readback would show
		// "unregistered log id" instead of the real message.
		std::unordered_map<uint32_t, std::string> debug_log_formats;

		SERIALIZE()
			{
				ar& NVP(functions);
				ar& NVP(blob);
				ar& NVP(entry_point);
				ar& NVP(debug_log_formats);

				IF_LOAD()
				{
					for (auto& [id, fmt] : debug_log_formats)
						DebugLogStrings::get().register_format(id, fmt);
				}
			}
	};

	struct ShaderCompiler :public Singleton<ShaderCompiler>, public API::ShaderCompiler
	{
		std::optional<CompiledShader> Compile_Shader(std::string shaderText, std::vector < shader_macro> macros, std::string target = "lib_6_3", std::string entry_point = "", ShaderOptions options = ShaderOptions::None, shader_include* includer = nullptr, std::string file_name = "");
		std::optional<CompiledShader> Compile_Shader_File(std::string filename, std::vector < shader_macro> macros, std::string target = "lib_6_3", std::string entry_point = "", ShaderOptions options = ShaderOptions::None, shader_include* includer = nullptr);

	private:
		ShaderCompiler();
		friend class Singleton<ShaderCompiler>;
	};

}
