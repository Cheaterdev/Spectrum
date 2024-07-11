import Core;
import <Core_defs.h>;
import jinja2cpp;
import cereal.json;


#include "Parsed.h"
#include "Parsing.h"


static const std::string cpp_path = "../../sources/HAL/autogen";
static const std::string hlsl_path = "../../workdir/shaders/autogen";

using namespace jinja2;

void iterate_files(std::filesystem::path path, std::function<void(std::filesystem::path)> f)
{
	using namespace std::filesystem;

	recursive_directory_iterator dir(path), end;

	while (dir != end)
	{
		// do other stuff here.
		if (!is_directory(*dir))
			f(dir->path());

		++dir;
	}
}
jinja2::TemplateEnv global;
Parsed parsed;
rapidjson::Document parsed_doc;

jinja2::Value parsed_map;
rapidjson::Document make_map(  auto& p)
{
	std::stringstream s;
		{
			cereal::JSONOutputArchive archive(s);

			p.serialize(archive);

		}
		rapidjson::Document doc;

		auto str = s.str();
		doc.Parse(str.c_str());

		return doc;
}
class TemplatesLibrary
{
	std::map<std::wstring, jinja2::Template> templates;

	std::map<std::string, ValuesList> user_lists;
public:

	void init()
	{
		iterate_files("templates/", [this](std::filesystem::path path) {
			std::ifstream stream;
			stream.open(path);

			if (!stream.is_open())
				return;


			auto& tpl = templates[path.stem()];
			tpl = jinja2::Template(&global);
			tpl.Load(stream);


			stream.close();



			});
	}


	auto generate(std::wstring filename, auto& params)
	{
		jinja2::Template& tpl = templates[filename];
		auto res = tpl.RenderAsString(params);

		if (res.has_value() )
		return res.value();


		std::cout<<res.error().ToString()<<std::endl;
	}

	auto generate2(std::wstring filename, std::string name, auto& p)
	{
		auto dp=make_map(p);
//		auto dp2=make_map(parsed);

		ValuesMap params = {
		   {name, Reflect(dp)}
		};

		params["recursive_slots"] = jinja2::MakeCallable(
			[](const std::string& name) {
				auto* layout = parsed.find_layout(name);
				ValuesList list;
				layout->recursive_slots([&](Slot& slot) {
					list.emplace_back(slot.name);
					});


				return list;
			},
			ArgInfo{ "name" }
		);

		params["recursive_samplers"] = jinja2::MakeCallable(
			[](const std::string& name) {
				auto* layout = parsed.find_layout(name);
				ValuesList list;


					layout->recursive_samplers([&](Sampler& slot) {
						list.emplace_back("HAL::Samplers::" + slot.expr);
						});


					return list;
			},
			ArgInfo{ "name" }
		);

	params["get_name_for"] = jinja2::MakeCallable(
			[](const int& i) {
				return get_name_for((ValueType)i);
			},
			ArgInfo{ "i" }
		);

	
	params["create_list"] = jinja2::MakeCallable(
			[this](const std::string& b) {
			auto& list = user_lists[b];
			list.clear();
			//	 a.asList().emplace_back(b);
			return list;
			},
			ArgInfo{ "b"  }
		);
	
	params["get_list"] = jinja2::MakeCallable(
			[this](const std::string& b) {
			auto& list = user_lists[b];

			return list;
			},
			ArgInfo{ "b"  }
		);
	params["myappend"] = jinja2::MakeCallable(
			[this](const std::string& list_name,const std::string& b) {
			auto& list = user_lists[list_name];
	
				list.emplace_back(b);
			
			return list;
			},
			ArgInfo{ "a"  },ArgInfo{ "b"  }
		);
	params["merge_lists"] = jinja2::MakeCallable(
			[this](const GenericList& a, const GenericList& b) {

			ValuesList list;
			for(const auto& e:a)
				list.emplace_back(e);

			for(const auto& e:b)
				list.emplace_back(e);

			return list;
			},
			ArgInfo{ "a"  }, ArgInfo{ "b"  }
		);
	params["lowerize"] = jinja2::MakeCallable(
			[](const std::string& name) {
				
			std::string lowcameled = name;
	lowcameled[0] = std::tolower(lowcameled[0]);
					return lowcameled;
			},
			ArgInfo{ "name" }
		);

	params["camel"] = jinja2::MakeCallable(
			[](const std::string& name) {
				
			std::string lowcameled = name;
	lowcameled[0] = std::toupper(lowcameled[0]);
					return lowcameled;
			},
			ArgInfo{ "name" }
		);
	
	
	params["replace_start"] = jinja2::MakeCallable(
			[](const std::string& a, const std::string& b, const std::string& c) {
			std::string res = a;

			if (res.starts_with(b))
				res.replace(0, b.size(), c);

			return res;
			},
			ArgInfo{ "a" },ArgInfo{ "b" },ArgInfo{ "c" }
		);
		return generate(filename, params);
	}
};
TemplatesLibrary templates;

std::string generate_array(const have_array& a)
{

	if (a.as_array) {
		if (a.array_count) {
			return std::format("[{}]", a.array_count);
		}
		else {
			return "";
		}
	}
	return "";
}

std::string generate_cpp_array(const have_array& a)
{
	if (a.as_array && a.array_count == 0)
		return "&";
	if (a.as_array && a.array_count > 0)
		return "*";
	return "&";
}


void generate_pass_table(Table& table)
{
	my_stream stream(hlsl_path, table.name + ".h");

	std::string  res = templates.generate2(L"pass_table", "table", table);
	stream << res << std::endl;
}

void generate_table(Table& table)
{
	my_stream stream(hlsl_path + "/tables", table.name + ".h");
	std::string  res = templates.generate2(L"hlsl.table", "table", table);
	stream << res << std::endl;

	// passes
	if (table.slot)
	{
		generate_pass_table(table);
	}
}


void generate_nobind_table(Table& table)
{
	my_stream stream(hlsl_path + "/tables", table.name + ".h");
	std::string  res = templates.generate2(L"nobind_table", "table", table);
	stream << res << std::endl;
}

std::string str_toupper(std::string s) {
	std::transform(s.begin(), s.end(), s.begin(),
		// static_cast<int(*)(int)>(std::toupper)         // неправильно
		// [](int c){ return std::toupper(c); }           // неправильно
		// [](char c){ return std::toupper(c); }          // неправильно
		[](unsigned char c) { return std::toupper(c); } // правильно
	);
	return s;
}


void generate_cpp_table(Table& table)
{
	my_stream stream(cpp_path + "/tables", table.name + ".table.ixx");
	std::string  res = templates.generate2(L"cpp.table", "table", table);
	stream << res << std::endl;
	


	if (table.slot)
	{
		my_stream stream(cpp_path + "/slots", table.name + ".ixx");
		std::string  res = templates.generate2(L"cpp.slot", "table", table);
		stream << res << std::endl;
	}
}


void generate_include_list(const Parsed& parsed)
{
	{
		my_stream stream(cpp_path, "autogen.ixx");
		have_name n;
		n.name="AUTOGEN";
		std::string  res = templates.generate2(L"cpp.autogen", "AUTOGEN", n);
		stream << res << std::endl;
	}

	{
		my_stream stream(cpp_path, "enums.ixx");
		have_name n;
		n.name="AUTOGEN";
		std::string  res = templates.generate2(L"cpp.enums", "AUTOGEN", n);
		stream << res << std::endl;
	}
	
	{
		my_stream stream(cpp_path, "pso.cpp");
		have_name n;
		n.name="AUTOGEN";
		std::string  res = templates.generate2(L"cpp.psos", "AUTOGEN", n);
		stream << res << std::endl;
	}
}

void generate_layout(Layout& layout)
{
	my_stream stream(hlsl_path + "/layout", layout.name + ".h");
	std::string  res = templates.generate2(L"layout", "layout", layout);
	stream << res << std::endl;
}

void generate_rt(Table& rt)
{
	my_stream stream(hlsl_path + "/rt", rt.name + ".h");
	std::string  res = templates.generate2(L"rt", "rt", rt);
	stream << res << std::endl;
}

void generate_pso(auto& pso)
{
	my_stream stream(cpp_path + "/pso", pso.name + ".h");
	std::string  res = templates.generate2(L"cpp.pso", "pso", pso);
	stream << res << std::endl;
}

void generate_cpp_rt(Table& table)
{
	my_stream stream(cpp_path + "/rt", table.name + ".h");
	std::string  res = templates.generate2(L"cpp.rt", "table", table);
	stream << res << std::endl;
}

void generate_cpp_layout(Layout& layout)
{
	my_stream stream(cpp_path + "/layout", layout.name + ".layout.ixx");
	std::string  res = templates.generate2(L"cpp_layout", "layout", layout);
	stream << res << std::endl;
}

void generate_raygen(RaytraceGen& pso)
{
	my_stream stream(cpp_path + "/rtx", pso.name + ".h");
	std::string  res = templates.generate2(L"raygen_pass", "pso", pso);
	stream << res << std::endl;
}

void generate_pass(RaytracePass& pso)
{
	my_stream stream(cpp_path + "/rtx", pso.name + ".h");
	std::string  res = templates.generate2(L"raytrace_pass", "pso", pso);
	stream << res << std::endl;
}

void generate_hlsl_pass(RaytracePass& pso)
{
	my_stream stream(hlsl_path + "/rtx", pso.name + ".h");
	std::string  res = templates.generate2(L"hlsl_pass", "pso", pso);
	stream << res << std::endl;
}

void generate_rtx_pso(RaytracePSO& pso)
{
	my_stream stream(cpp_path + "/rtx", pso.name + ".h");
	std::string  res = templates.generate2(L"rtx_pso", "pso", pso);
	stream << res << std::endl;
}

void generate_rtx(Parsed& parsed)
{

	for (auto& v : parsed.raytrace_gen)
	{
		auto bind = v.find_option("Bind");
		parsed.find_rtx(bind->value_atom.expr)->gens.emplace_back(v);

		generate_raygen(v);
	}

	for (auto& v : parsed.raytrace_pass)
	{
		auto bind = v.find_option("Bind");
		parsed.find_rtx(bind->value_atom.expr)->passes.emplace_back(v);


		generate_pass(v);
		generate_hlsl_pass(v);
	}

	for (auto& v : parsed.raytrace_pso)
	{
		generate_rtx_pso(v);
	}
}
int main() {
	//std::filesystem::remove_all(L"output");



	try
	{

		iterate_files("sigs/", [](std::wstring filename) {
			auto p = parse(filename);

			parsed.merge(p);
			});

		parsed.setup();
		parsed_doc=make_map(parsed);

		parsed_map = Reflect(parsed_doc);
		global.AddGlobal("parsed", parsed_map);
		global.AddGlobal("ValueType",ValuesMap{{"CB", ValueType::CB},{"SRV", ValueType::SRV},{"UAV",ValueType::UAV},{"SMP",ValueType::SMP},{"STRUCT",ValueType::STRUCT}});
		global.GetSettings().extensions.Do = true;
		templates.init();

		for (auto& table : parsed.tables)
		{

			if (table.find_option("nobind"))
				generate_nobind_table(table);

			else
				generate_table(table);

			if (!table.find_option("shader_only"))
				generate_cpp_table(table);

			if (table.find_option("RenderTarget"))
			{
				generate_cpp_rt(table);
				generate_rt(table);
			}
		}

		for (auto& layout : parsed.layouts)
		{
			generate_layout(layout);
			generate_cpp_layout(layout);
		}

		generate_include_list(parsed);

		for (auto& pso : parsed.compute_pso)
		{
			generate_pso(pso);
		}

		for (auto& pso : parsed.graphics_pso)
		{
			generate_pso(pso);
		}

		generate_rtx(parsed);

	}
	catch (std::exception& e)
	{
		std::cout << "Error: " << e.what() << std::endl;
	}

	return 0;
}