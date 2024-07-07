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
		   {name, Reflect(dp)},

			{"ValueType",ValuesMap{
				{"CB", ValueType::CB},{"SRV", ValueType::SRV},{"UAV",ValueType::UAV},{"SMP",ValueType::SMP},{"STRUCT",ValueType::STRUCT}
					}}
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
	stream << "#pragma once" << std::endl;
	stream << "#include \"sig_hlsl.hlsl\"" << std::endl;
	for (auto& v : table.used_tables)
	{
		auto t = parsed.find_table(v);

		if (t->can_compile)
			stream << "#include \"" << v << ".h\"" << std::endl;
	}

	// declaration
	auto declare_func = [&](ValueType type) {
		//	if (table.counts[type] == 0) return;

		for (auto& v : table.values)
		{
			if (v.value_type != type) continue;
			//	if (v.bindless) continue;


			if (!v.pointer && (v.value_type == ValueType::CB || v.value_type == ValueType::STRUCT))
				stream << v.get_type() << " " << v.name << generate_array(v) << "; // " << v.get_type() << std::endl;
			else
				stream << "uint" << " " << v.name << (v.bindless ? "" : generate_array(v)) << "; // " << v.get_type() << std::endl;
			//		stream << v.get_type()<<" " << v.name << ';' << std::endl;
		}
		};

	//if (!table.cb_provided) declare_func(ValueType::CB);
	//declare_func(ValueType::SRV);
	//declare_func(ValueType::UAV);
	//declare_func(ValueType::SMP);




	// result struct
	stream << "struct " << table.name << std::endl;

	stream << "{" << std::endl;

	{
		stream.push();
		//	if (table.counts[ValueType::CB] != 0) stream << table.name << "_cb cb;" << std::endl;
		//	if (table.counts[ValueType::SRV] != 0) stream << table.name << "_srv srv;" << std::endl;
		//	if (table.counts[ValueType::UAV] != 0) stream << table.name << "_uav uav;" << std::endl;
		//	if (table.counts[ValueType::SMP] != 0)  stream << table.name << "_smp smp;" << std::endl;

		declare_func(ValueType::CB);
		declare_func(ValueType::SRV);
		declare_func(ValueType::UAV);
		declare_func(ValueType::SMP);
		declare_func(ValueType::STRUCT);

		for (auto& v : table.values)
		{
			if (v.value_type != ValueType::CB && v.value_type != ValueType::STRUCT) continue;
			std::string cameled = v.name;
			cameled[0] = std::toupper(cameled[0]);


			auto t = v.get_type();
			if (v.pointer)  t = "uint";
			if (v.as_array)
			{
				stream << t << " Get" << cameled << "(int i) { " << "return " << v.name << "[i]; }" << std::endl;
			}
			else
			{
				stream << t << " Get" << cameled << "() { " << "return " << v.name << "; }" << std::endl;

			}
		}


		for (auto& v : table.values)
		{
			if (v.value_type == ValueType::STRUCT) continue;
			if (v.value_type == ValueType::CB) continue;
			std::string cameled = v.name;
			cameled[0] = std::toupper(cameled[0]);


			std::string type = v.get_type();

			if (type.starts_with("DepthStencil"))
				type.replace(0, strlen("DepthStencil"), "Texture2D");
			if (type.starts_with("RenderTarget"))
				type.replace(0, strlen("RenderTarget"), "Texture2D");


			if (v.as_array)
			{
				if (v.bindless)
				{
					stream << type << " Get" << cameled << "(int i) { " << std::endl;
					stream << "StructuredBuffer<uint> indirection = ResourceDescriptorHeap[" << v.name << "]; " << std::endl;
					stream << "uint id = indirection.Load(i);" << std::endl;
					stream << "return ResourceDescriptorHeap[id]; }" << std::endl;
				}
				else
					stream << type << " Get" << cameled << "(int i) { " << "return ResourceDescriptorHeap[" << v.name << "[i]]; }" << std::endl;
			}
			else
			{
				stream << type << " Get" << cameled << "() { " << "return ResourceDescriptorHeap[" << v.name << "]; }" << std::endl;

			}
		}


	}


	if (!table.hlsl.empty())
		stream << table.hlsl << std::endl;
	stream.pop();
	stream << "};" << std::endl;

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

	stream << "export module HAL:Autogen.Tables." << table.name << ";" << std::endl;

	stream << "import Core;" << std::endl;
	stream << "import :SIG;" << std::endl;
	stream << "import :Types;" << std::endl;
	stream << "import :HLSL;" << std::endl;
	stream << "import <HAL.h>;" << std::endl;
	//	stream << "import Core;"<<std::endl;
	//
	//
	//	
	//
	//
	//import :PipelineState;
	//import :SIG;
	//import :RT;
	//import :Layout;
	//import :Slots;
	//import :PSO;
	//import :RTX;
	//import :Enums;
	//import :RootSignature;
	//import :Types;


	for (auto& v : table.used_tables)
	{
		auto t = parsed.find_table(v);
		if (t->find_option("shader_only")) continue;

		if (table.find_option("IndirectCommand"))
			stream << "import :Autogen.Slots." << v << ";" << std::endl;
		else
			stream << "import :Autogen.Tables." << v << ";" << std::endl;

	}

	stream << "import :Enums;" << std::endl;
	// declaration
	auto declare_func = [&](ValueType type) {

		for (auto& v : table.values)
		{
			if (v.find_option("dynamic")) continue;
			if (v.value_type != type) continue;
			//if (v.array_count == 0)
			//	continue;


			stream << get_cpp_for(v) << " " << v.name << generate_array(v) << ';' << std::endl;
		}





		};


	stream << "export namespace Table " << std::endl;
	stream << "{" << std::endl;
	{
		stream.push();
		stream << "#pragma pack(push, 1)" << std::endl;

		stream << "struct " << table.name << std::endl;
		stream << "{" << std::endl;
		{
			stream.push();
			stream << "static constexpr SlotID ID = SlotID::" << table.name << ";" << std::endl;

			declare_func(ValueType::CB);
			declare_func(ValueType::SRV);
			declare_func(ValueType::UAV);
			declare_func(ValueType::SMP);
			declare_func(ValueType::STRUCT);

			if (table.bindless_table)
			{
				//		stream << "HAL::Bindless bindless;" << std::endl;
			}

			for (auto& v : table.values)
			{
				if (!v.find_option("dynamic"))
					continue;

				stream << "DynamicData " << v.name << generate_array(v) << ';' << std::endl;
			}



			for (auto& v : table.values)
			{
				if (v.value_type == ValueType::STRUCT) continue;
				//	if (v.array_count == 0) continue;
				if (v.find_option("dynamic"))
					continue;
				std::string cameled = v.name;
				cameled[0] = std::toupper(cameled[0]);

				//	if (v.array_count == 0)
				//		stream << get_cpp_for(v) << generate_cpp_array(v) << " Get" << cameled << "() { " << "return bindless; }" << std::endl;

				//	else
				stream << get_cpp_for(v) << generate_cpp_array(v) << " Get" << cameled << "() { " << "return " << v.name << "; }" << std::endl;
			}


			for (auto& v : table.values)
			{
				if (v.value_type == ValueType::STRUCT) continue;
				//	if (v.array_count == 0) continue;
				if (!v.find_option("dynamic"))
					continue;
				std::string cameled = v.name;
				cameled[0] = std::toupper(cameled[0]);

				stream << "DynamicData" << generate_cpp_array(v) << " Get" << cameled << "() { " << "return " << v.name << "; }" << std::endl;
			}



			for (auto& v : table.values)
			{
				if (v.value_type != ValueType::STRUCT) continue;
				//	stream << '\t' << v.get_type() << " " << v.name << ';' << std::endl;



				auto& vtable = *parsed.find_table(v.get_type());




				std::string cameled = v.name;
				cameled[0] = std::toupper(cameled[0]);

				if (vtable.find_option("shader_only"))
				{
					stream << v.get_type() << generate_cpp_array(v) << " Get" << cameled << "() { " << "return " << v.name << "; }" << std::endl;
					continue;
				};


				stream << v.get_type() << "& Get" << cameled << "() { " << "return " << v.name << "; }" << std::endl;
			}



			std::string pass;
			std::string args;

			auto f = [&](ValueType type) {
				if (table.counts[type] == 0) return;

				if (pass.size()) pass += ",";
				else pass += ":";

				pass += get_name_for(type) + "(" + get_name_for(type) + ")";

				if (args.size()) args += ",";
				args += str_toupper(get_name_for(type)) + "&" + get_name_for(type);

				};

			f(ValueType::CB);
			f(ValueType::SRV);
			f(ValueType::UAV);
			f(ValueType::SMP);

			if (table.bindless_table)
			{
				if (pass.size()) pass += ",";
				else pass += ":";

				pass += "bindless(bindless)";

				if (args.size()) args += ",";
				args += "HAL::Bindless &bindless";

			}

			//	stream << table.name << "(" << args << ") " << pass << "{}" << std::endl;


				// declaration
			auto compile_func = [&](ValueType type) {
				//	if (table.counts[type] == 0) return;

				for (auto& v : table.values)
				{
					if (v.value_type != type) continue;

					stream << "compiler.compile(" << v.name << ");" << std::endl;

				}
				};

			stream << "static constexpr SIG_TYPE TYPE = SIG_TYPE::Table;" << std::endl;



			stream << "template<class Compiler>" << std::endl;

			stream << "void compile(Compiler& compiler) const" << std::endl;
			stream << "{" << std::endl;

			stream.push();


			compile_func(ValueType::CB);
			compile_func(ValueType::SRV);
			compile_func(ValueType::UAV);
			compile_func(ValueType::SMP);
			compile_func(ValueType::STRUCT);
			stream.pop();

			stream << "}" << std::endl;


			{


				// declaration
				auto compile_func = [&](ValueType type) {
					//	if (table.counts[type] == 0) return;

					for (auto& v : table.values)
					{
						if (v.value_type != type) continue;
						if (v.value_type == ValueType::CB)
							stream << v.get_type() << " " << v.name << generate_array(v) << "; // " << v.get_type() << std::endl;
						else	if (v.value_type == ValueType::STRUCT)
							stream << v.get_type() << "::Compiled " << v.name << generate_array(v) << "; // " << v.get_type() << std::endl;
						else
							stream << "uint" << " " << v.name << (v.bindless ? "" : generate_array(v)) << "; // " << v.get_type() << std::endl;

					}
					};


				bool can_compiled = true;
				bool need_compiled = false;
				for (auto& v : table.values)
				{

					if (v.value_type == ValueType::CB)
						need_compiled = need_compiled;
					else	if (v.value_type == ValueType::STRUCT)
						need_compiled = need_compiled;
					else
						need_compiled = true;


					if (v.find_option("dynamic"))can_compiled = false;
				}
				if (can_compiled) {
					if (need_compiled) {
						stream << "struct Compiled" << std::endl;
						stream << "{" << std::endl;

						stream.push();


						compile_func(ValueType::CB);
						compile_func(ValueType::SRV);
						compile_func(ValueType::UAV);
						compile_func(ValueType::SMP);
						compile_func(ValueType::STRUCT);
						stream.pop();

						stream << "};" << std::endl;
						stream.pop();
					}
					else
					{
						stream << "using Compiled = " << table.name << ";" << std::endl;

					}
				}
			}




			if (table.find_option("IndirectCommand"))
			{


				stream << "static const IndirectCommands CommandID = IndirectCommands::" << table.name << ";" << std::endl;
				stream << "template<class Processor> static void for_each(Processor& processor) {" << std::endl;
				{
					stream.push();

					std::string result_string;
					bool first = true;
					for (auto& v : table.values) {
						//	if (!v.pointer) continue;

						if (!first) result_string += ',';

						auto t = parsed.find_table(v.get_type());
						if (t && !t->find_option("shader_only"))
							result_string += "Slots::";

						result_string += v.get_type();
						first = false;
					};


					stream << "processor.template process<" << result_string << ">();" << std::endl;
					stream.pop();
					stream << "}" << std::endl;

				}
			}


			if (table.find_option("serialize"))
			{

				stream << std::format(
					R"(private:
	SERIALIZE())") << std::endl;
				stream << "{" << std::endl;

				for (auto& v : table.values)
				{
					if (v.value_type != ValueType::CB && v.value_type != ValueType::STRUCT) continue;

					stream << std::format(
						R"(     ar& NVP({});)", v.name) << std::endl;

				}
				stream << "}" << std::endl;
			}


			stream << "};" << std::endl;


		}

		stream << "#pragma pack(pop)" << std::endl;

		stream.pop();
	}


	stream << "}" << std::endl;


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
		std::string  res = templates.generate2(L"cpp.pso", "AUTOGEN", n);
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

void generate_pso(PSO& pso)
{

	my_stream stream(cpp_path + "/pso", pso.name + ".h");
	stream << "#pragma once" << std::endl;

	std::string keys;
	std::string keyslist;
	std::string keysgenerators;


	std::string defines;
	std::string definesapply;

	for (auto d : pso.defines)
	{

		bool hasvalue = d.values.size();

		keys += "KeyValue<";


		if (hasvalue)
		{
			auto opt = d.find_option("type");
			if (opt)
			{
				keys += "Underlying<";
				if (opt->value_atom.expr == "Format")
					keys += "HAL::";
				keys += opt->value_atom.expr;
				keys += ">";
			}
			else

				keys += "int";
		}

		else
			keys += "NoValue";

		if (hasvalue)
		{
			keys += ",NonNullable";
			for (auto v : d.values)
			{
				keys += ",";
				keys += v.expr;
			}

		}
		else
			keys += ",Nullable";

		keys += "> ";
		keys += d.name;
		keys += ";\n";

		if (!keyslist.empty())
			keyslist += ",";

		keyslist += d.name;


		keysgenerators += "GEN_KEY(";
		keysgenerators += d.name;

		if (d.find_option("indirect"))
			keysgenerators += ",false";

		else
			keysgenerators += ",true";

		keysgenerators += ");\n";


		defines += "static const ShaderDefine<&Keys::";
		defines += d.name;

		if (d.find_option("CS"))
			defines += ", &SimpleComputePSO::compute";

		if (d.find_option("PS"))
			defines += ", &SimpleGraphicsPSO::pixel";

		if (d.find_option("VS"))
			defines += ", &SimpleGraphicsPSO::vertex";

		if (d.find_option("GS"))
			defines += ", &SimpleGraphicsPSO::geometry";


		if (d.find_option("DS"))
			defines += ", &SimpleGraphicsPSO::domain";

		if (d.find_option("HS"))
			defines += ", &SimpleGraphicsPSO::hull";

		if (d.find_option("MS"))
			defines += ", &SimpleGraphicsPSO::mesh";

		if (d.find_option("AS"))
			defines += ", &SimpleGraphicsPSO::amplification";

		defines += "> ";
		defines += d.name;
		defines += " = \"";

		auto op = d.find_option("rename");
		if (op)
		{
			defines += op->value_atom.expr;
		}
		else
		{
			defines += d.name;
		}

		defines += "\";\n";


		definesapply += d.name;
		definesapply += ".Apply(mpso, key);\n";

	}
	stream << "namespace PSOS" << std::endl;
	stream << "{" << std::endl;
	{
		stream.push();

		stream << "struct " << pso.name << ": public PSOBase" << std::endl;


		stream << "{" << std::endl;
		{
			stream.push();

			stream << "struct Keys {" << std::endl;
			stream.push();
			stream << keys << std::endl;
			stream << "GEN_DEF_COMP(Keys);" << std::endl;

			stream << "private:" << std::endl;

			stream << "SERIALIZE()" << std::endl;
			stream << "{" << std::endl;
			stream.push();

			for (auto d : pso.defines)
				stream << "ar&NVP(" << d.name << ");" << std::endl;


			stream.pop();
			stream << "}" << std::endl;

			stream.pop();
			stream << " };" << std::endl;

			std::string GEN_PSO;
			if (dynamic_cast<ComputePSO*>(&pso))
			{
				GEN_PSO = "GEN_COMPUTE_PSO";
			}
			else
			{
				GEN_PSO = "GEN_GRAPHICS_PSO";
			}

			if (keyslist.empty())
				stream << GEN_PSO << "(" << pso.name << ")" << std::endl;
			else
				stream << GEN_PSO << "(" << pso.name << "," << keyslist << ")" << std::endl;
			stream << keysgenerators << std::endl;
			stream << "SimplePSO init_pso(Keys & key, std::function<void(SimplePSO&, Keys&)> f)" << std::endl;
			stream << "{" << std::endl;
			{
				stream.push();
				stream << defines << std::endl;


				stream << "SimplePSO mpso(\"" << pso.name << "\");" << std::endl;

				stream << "if(f) f(mpso,key);" << std::endl;

				stream << "mpso.root_signature = Layouts::" << pso.root_sig.name << ";" << std::endl;
				for (auto& shader : pso.shaders)
				{

					if (shader.find_option("Erase"))
					{
						stream << "mpso." << shader.name << ".file_name = \"\";" << std::endl;
						stream << "mpso." << shader.name << ".entry_point = \"\";" << std::endl;
						stream << "mpso." << shader.name << ".flags = HAL::ShaderOptions::None;" << std::endl;

					}
					else
					{
						stream << "mpso." << shader.name << ".file_name = \"shaders/" << shader.path << ".hlsl\";" << std::endl;
						stream << "mpso." << shader.name << ".entry_point = \"" << shader.find_option("EntryPoint")->value_atom.expr << "\";" << std::endl;

						if (shader.find_option("Enable16bits"))
						{
							stream << "mpso." << shader.name << ".flags = HAL::ShaderOptions::FP16;" << std::endl;

						}
						else
						{
							stream << "mpso." << shader.name << ".flags = HAL::ShaderOptions::None;" << std::endl;

						}

					}
				}

				stream << definesapply << std::endl;


				auto graphics = dynamic_cast<GraphicsPSO*>(&pso);
				if (graphics)
				{

					{
						std::string elems;

						for (auto e : graphics->rtv.values)
						{
							if (!elems.empty())
								elems += ", ";
							auto def = graphics->find_define(e.expr);

							if (def)
							{
								elems += e.expr;
								elems += ".get_value(mpso, key)";
							}
							else
							{
								elems += "HAL::Format::";
								elems += e.expr;
							}

						}

						stream << "mpso.rtv_formats = { " << elems << " };" << std::endl;
					}

					{
						std::string elems;

						for (auto e : graphics->blend.values)
						{
							if (!elems.empty())
								elems += ", ";
							elems += "HAL::Blends::";
							elems += e.expr;
						}

						stream << "mpso.blend = { " << elems << " };" << std::endl;
					}
					//	graphics->rtvs[0].


					for (auto& e : graphics->params)
					{

						std::string add;

						if (e.get_type() == "ds")
							add = "HAL::Format::";
						if (e.get_type() == "cull")
							add = "HAL::CullMode::";
						else if (e.get_type() == "depth_func")
							add = "HAL::ComparisonFunc::";
						else if (e.get_type() == "topology")
							add = "HAL::PrimitiveTopologyType::";
						else if (e.get_type() == "stencil_func")
							add = "HAL::ComparisonFunc::";
						else if (e.get_type() == "stencil_pass_op")
							add = "HAL::StencilOp::";

						stream << "mpso." << e.get_type() << "  = " << add << e.expr << ";" << std::endl;
					}
				}
				stream << "return mpso;" << std::endl;
				stream.pop();
			}
			stream << "}" << std::endl;
			stream << "private:" << std::endl;

			stream << "SERIALIZE()" << std::endl;
			stream << "{" << std::endl;
			stream.push();


			stream << "ar&NVP(wrap(psos));" << std::endl;


			stream.pop();
			stream << "}" << std::endl;


			stream.pop();
		}
		stream << "};" << std::endl;

		stream.pop();
	}
	stream << "}" << std::endl;
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

		for (auto& rt : parsed.rt)
		{
			//	generate_rt(rt);
			//	generate_cpp_rt(rt);
		}

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