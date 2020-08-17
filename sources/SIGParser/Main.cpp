#include <iostream>
#include <algorithm>
#include "Parsed.h"
#include "Parsing.h"
#include <filesystem>
#include <functional>

static const std::string cpp_path = "../../sources/RenderSystem/autogen";
static const std::string hlsl_path = "../../workdir/shaders/autogen";

Parsed parsed;
std::string generate_array(const have_array& a)
{

	if (a.as_array) {
		if (a.array_count) {
			return  std::string("[") + std::to_string(a.array_count) + "]";
		}
		else {
			return "[]";
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

int calculate_max_size(Table& table)
{
	int res = 0;

	auto generate = [&](ValueType type) {
		for (auto& v : table.values)
		{
			if (v.value_type != type) continue;
			if (v.bindless)  continue;


			res += v.array_count;
		}
	};

	generate(ValueType::SRV);


	for (auto& v : table.values)
	{
		if (v.value_type != ValueType::STRUCT) continue;

		res += calculate_max_size(*parsed.find_table(v.type));
	}

	return res;
}
void generate_pass(my_stream& stream, Table& table, table_offsets& offsets, Slot& slot)
{
	auto generate = [&](ValueType type, std::string subname) {
		for (auto& v : table.values)
		{
			if (v.value_type != type) continue;
			if (v.bindless)  continue;

			auto& e = offsets[type];
			stream << v.type << " " << get_name_for(type) << "_" << slot.id << "_" << e << generate_array(v) << ": register(" << subname << e << ", space" << slot.id << ");" << std::endl;
			e += v.array_count;
		}
	};

	generate(ValueType::SRV, "t");
	generate(ValueType::UAV, "u");
	generate(ValueType::SMP, "s");

	for (auto& v : table.values)
	{
		if (v.value_type != ValueType::STRUCT) continue;

		generate_pass(stream, *parsed.find_table(v.type), offsets, slot);
	}
}

void generate_bind(my_stream& stream, std::string parent, Table& table, Slot& slot, table_offsets offsets = table_offsets())
{
	std::string write_to = "result";

	if (parent.empty()) {
		if (table.counts[ValueType::CB] > 0)
			stream << "result.cb = " << std::string("cb_") << slot.id << "_0;" << std::endl;
	}

	auto process_type = [&](ValueType type) {

		auto& i = offsets[type];
		for (auto& v : table.values)
		{
			if (v.value_type != type) continue;
			if (v.bindless) continue;

			stream << "result." << get_name_for(type) << "." << parent << v.name << " = " << get_name_for(type) << "_" << slot.id << "_" << i << ";" << std::endl;

			i += v.array_count;
		}
	};


	process_type(ValueType::SRV);
	process_type(ValueType::UAV);
	process_type(ValueType::SMP);


	for (auto& v : table.values)
	{
		if (v.value_type != ValueType::STRUCT) continue;

		auto t = parsed.find_table(v.type);
		std::string parent_to;
		if (parent.empty())
			parent_to = v.name + ".";
		else
			parent_to = parent + "." + v.name + ".";
		generate_bind(stream, parent_to, *t, slot, offsets);
	}

	/*if (parent.empty())
		for (auto& v : table.values)
		{
			if (v.value_type != ValueType::STRUCT) continue;

			std::string pass;
			auto t = parsed.find_table(v.type);
			auto f = [&](ValueType type) {
				if (t->counts[type] == 0) return;


				if (pass.size()) pass += ",";
				pass += std::string("result.") + get_name_for(type) + "." + v.name;
			};

			f(ValueType::CB);
			f(ValueType::SRV);
			f(ValueType::UAV);
			f(ValueType::SMP);

			stream << "result." << v.name << " = Create" << v.type << "(" << pass << ");" << std::endl;
		}
		*/

}

void generate_pass_table(Table& table)
{
	my_stream stream(hlsl_path, table.name + ".h");


	stream << "#ifndef SLOT_" << table.slot->id << std::endl;
	stream << "\t#define SLOT_" << table.slot->id << std::endl;
	stream << "#else" << std::endl;
	stream << "\t#error Slot " << table.slot->id << " is already used" << std::endl;
	stream << "#endif" << std::endl;


	stream << "#include \"layout/" << table.slot->layout->name << ".h\"" << std::endl;



	if (table.bindless_srv)
		stream << table.bindless_srv->type << " bindless[]" << ": register(t" << calculate_max_size(table) << ", space" << table.slot->id << ");" << std::endl;

	stream << "#include \"tables/" << table.name << ".h\"" << std::endl;
	// cb
	if (table.counts[ValueType::CB] > 0) stream << "ConstantBuffer<" << table.name << "_cb> cb_" << table.slot->id << "_0:register(b0,space" << table.slot->id << ");" << std::endl;



	table_offsets offsets;
	offsets[ValueType::SMP] = 0;
	offsets[ValueType::UAV] = 0;
	offsets[ValueType::SRV] = 0;
	offsets[ValueType::CB] = 0;

	generate_pass(stream, table, offsets, *table.slot);


	//creation func

	{
		stream << table.name << " Create" << table.name << "()" << std::endl;
		stream << "{" << std::endl;
		{
			stream.push();
			stream << table.name << " result;" << std::endl;

			table_offsets offsets;
			offsets[ValueType::SMP] = 0;
			offsets[ValueType::UAV] = 0;
			offsets[ValueType::SRV] = 0;
			offsets[ValueType::CB] = 0;

			offsets[ValueType::SMP] = table.slot->layout->samplers.size();
			generate_bind(stream, "", table, *table.slot, offsets);
			stream << "return result;" << std::endl;
			stream.pop();
		}
		stream << "}" << std::endl;
	}

	std::string cameled = table.name;
	cameled[0] = std::toupper(cameled[0]);

	std::string lowcameled = table.name;
	lowcameled[0] = std::tolower(lowcameled[0]);


	stream << "static const " << table.name << " " << lowcameled << "_global = Create" << table.name << "();" << std::endl;



	stream << "const " << table.name << " Get" << cameled << "(){ return " << lowcameled << "_global; }" << std::endl;

}
void generate_table(Table& table)
{
	my_stream stream(hlsl_path + "/tables", table.name + ".h");
	stream << "#pragma once" << std::endl;

	for (auto& v : table.used_tables)
	{
		stream << "#include \"" << v << ".h\"" << std::endl;
	}

	// declaration
	auto declare_func = [&](ValueType type) {
		if (table.counts[type] == 0) return;
		stream << "struct " << table.name << "_" << get_name_for(type) << std::endl;

		stream << "{" << std::endl;	stream.push();
		for (auto& v : table.values)
		{
			if (v.value_type != type) continue;
			if (v.bindless) continue;

			stream << v.type << " " << v.name << generate_array(v) << ';' << std::endl;
		}

		for (auto& v : table.values)
		{
			if (v.value_type == ValueType::STRUCT)
			{
				auto t = parsed.find_table(v.type);
				if (t->counts[type] == 0) continue;
				stream << v.type << "_" << get_name_for(type) << " " << v.name << ";" << std::endl;
			}
		}
		
		stream.pop();
		stream << "};" << std::endl;
	};

	if (!table.cb_provided) declare_func(ValueType::CB);
	declare_func(ValueType::SRV);
	declare_func(ValueType::UAV);
	declare_func(ValueType::SMP);




	// result struct
	stream << "struct " << table.name << std::endl;

	stream << "{" << std::endl;

	{
		stream.push();
		if (table.counts[ValueType::CB] != 0) stream << table.name << "_cb cb;" << std::endl;
		if (table.counts[ValueType::SRV] != 0) stream << table.name << "_srv srv;" << std::endl;
		if (table.counts[ValueType::UAV] != 0) stream << table.name << "_uav uav;" << std::endl;
		if (table.counts[ValueType::SMP] != 0)  stream << table.name << "_smp smp;" << std::endl;



		for (auto& v : table.values)
		{
			if (v.value_type == ValueType::STRUCT) continue;
			std::string cameled = v.name;
			cameled[0] = std::toupper(cameled[0]);

			if (v.as_array)
			{
				if (v.bindless)
					stream << v.type << " Get" << cameled << "(int i) { " << "return bindless[i]; }" << std::endl;
				else
					stream << v.type << " Get" << cameled << "(int i) { " << "return " << get_name_for(v.value_type) << "." << v.name << "[i]; }" << std::endl;
			}
			else
				stream << v.type << " Get" << cameled << "() { " << "return " << get_name_for(v.value_type) << "." << v.name << "; }" << std::endl;

		}


		for (auto& v : table.values)
		{
			if (v.value_type != ValueType::STRUCT) continue;
		//	stream << v.type << " " << v.name << generate_array(v) << ';' << std::endl;


				std::string pass;
				auto t = parsed.find_table(v.type);
				auto f = [&](ValueType type) {
					if (t->counts[type] == 0) return;
					if (pass.size()) pass += ",";
					pass += get_name_for(type) + "." + v.name;
				};

				f(ValueType::CB);
				f(ValueType::SRV);
				f(ValueType::UAV);
				f(ValueType::SMP);


			std::string cameled = v.name;
			cameled[0] = std::toupper(cameled[0]);
/*
			if (v.as_array)
			{
				stream << v.type << " Get" << cameled << "(int i) { " << "return " << v.name << "[i]; }" << std::endl;
			}
			else
			
			*/

	stream << v.type << " Get" << cameled << "() { " << "return Create" << v.type << "(" << pass << "); }" << std::endl;
		}

		stream.pop();

	}
	stream << "};" << std::endl;



	//creation func bindless

	{

		std::string types;
		std::string args;


		auto f = [&](ValueType type) {
			if (table.counts[type] == 0) return;

			if (types.size()) types += ",";
			types += get_name_for(type);

			if (args.size()) args += ",";
			args += table.name + "_" + get_name_for(type) + " " + get_name_for(type);
		};


		f(ValueType::CB);
		f(ValueType::SRV);
		f(ValueType::UAV);
		f(ValueType::SMP);

		stream << " const " << table.name << " Create" << table.name << "(" << args << ")" << std::endl;
		stream << "{" << std::endl;
		{
			stream.push();






			stream << "const " << table.name << " result = {" << types << std::endl;
			/*
						stream << "result.cb = cb;" << std::endl;
						stream << "result.srv = srv;" << std::endl;
						stream << "result.uav = uav;" << std::endl;
						stream << "result.smp = smp;" << std::endl;
						*/


		

			stream << "};" << std::endl;

			stream << "return result;" << std::endl;

			stream.pop();
		}
		stream << "}" << std::endl;
	}




	// passes
	if (table.slot)
	{
		generate_pass_table(table);
	}

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


void generate_cpp_table(const Table& table)
{
	my_stream stream(cpp_path + "/tables", table.name + ".h");

	stream << "#pragma once" << std::endl;

	for (auto& v : table.used_tables)
	{
		auto t = parsed.find_table(v);
		if (t->find_option("shader_only")) continue;

		stream << "#include \"" << v << ".h\"" << std::endl;
	}


	// declaration
	auto declare_func = [&](ValueType type) {

		if (table.cb_provided && type == ValueType::CB)
		{
			stream << "using CB = DefaultCB;" << std::endl;

			stream << "CB &cb;" << std::endl;
			return;
		}

		if (table.counts[type] == 0)
		{
			stream << "using " << str_toupper(get_name_for(type)) << " = Empty;" << std::endl;

			return;
		}


		stream << "struct " << str_toupper(get_name_for(type)) << std::endl;

		stream << "{" << std::endl;
		{
			stream.push();
			for (auto& v : table.values)
			{
				if (v.find_option("dynamic"))
					continue;
				if (v.value_type != type) continue;
				if (v.array_count == 0)
					continue;
			

				stream << get_cpp_for(v) << " " << v.name << generate_array(v) << ';' << std::endl;
			}

			for (auto& v : table.values)
			{
				if (v.find_option("dynamic")) 
					continue;

				if (v.value_type == ValueType::STRUCT)
				{
					
						auto t = parsed.find_table(v.type);

						if(t->find_option("shader_only"))
							stream << v.type << " " << v.name << generate_array(v) << ';' << std::endl;
						else
							stream << v.type << "::" << str_toupper(get_name_for(type)) << " " << v.name << generate_array(v) << ";" << std::endl;
				}


			}

			
			stream.pop();
		}
		stream << "} &" << get_name_for(type) << ";" << std::endl;
	};


	stream << "namespace Table " << std::endl;
	stream << "{" << std::endl;
	{
		stream.push();
		stream << "#pragma pack(push, 1)" << std::endl;

		stream << "struct " << table.name << std::endl;
		stream << "{" << std::endl;
		{
			stream.push();
			declare_func(ValueType::CB);
			declare_func(ValueType::SRV);
			declare_func(ValueType::UAV);
			declare_func(ValueType::SMP);

			if (table.bindless_table)
			{
				stream << "Render::Bindless& bindless;" << std::endl;
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

				if (v.array_count == 0)
					stream << get_cpp_for(v) << generate_cpp_array(v) << " Get" << cameled << "() { " << "return bindless; }" << std::endl;

				else
					stream << get_cpp_for(v) << generate_cpp_array(v) << " Get" << cameled << "() { " << "return " << get_name_for(v.value_type) << "." << v.name << "; }" << std::endl;
			}


			for (auto& v : table.values)
			{
				if (v.value_type == ValueType::STRUCT) continue;
				//	if (v.array_count == 0) continue;
				if (!v.find_option("dynamic"))
					continue;
				std::string cameled = v.name;
				cameled[0] = std::toupper(cameled[0]);

				stream << "DynamicData"<< generate_cpp_array(v) << " Get" << cameled << "() { " << "return " << v.name << "; }" << std::endl;
			}



			for (auto& v : table.values)
			{
				if (v.value_type != ValueType::STRUCT) continue;
				//	stream << '\t' << v.type << " " << v.name << ';' << std::endl;



				auto& vtable = *parsed.find_table(v.type);


			

				std::string cameled = v.name;
				cameled[0] = std::toupper(cameled[0]);

				if (vtable.find_option("shader_only"))
				{
					stream << v.type << generate_cpp_array(v) << " Get" << cameled << "() { " << "return " << "cb." << v.name << "; }" << std::endl;
					continue;
				};


				std::string pass;
				auto t = parsed.find_table(v.type);
				auto f = [&](ValueType type) {
					if (t->counts[type] == 0) return;
					if (pass.size()) pass += ",";
					pass += get_name_for(type) + "." + v.name;
				};

				f(ValueType::CB);
				f(ValueType::SRV);
				f(ValueType::UAV);
				f(ValueType::SMP);



				if (vtable.bindless_table)
				{
					if (pass.size()) pass += ",";
					pass += "bindless";

				}


				stream << v.type << " Map" << cameled << "() { " << "return " << v.type << "(" << pass << "); }" << std::endl;
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
				args += "Render::Bindless &bindless";

			}

			stream << table.name << "(" << args << ") " << pass << "{}" << std::endl;
			stream.pop();
			stream << "};" << std::endl;
		}

		stream << "#pragma pack(pop)" << std::endl;

		stream.pop();
	}
	stream << "}" << std::endl;


	if (table.slot)
	{
		my_stream stream(cpp_path + "/slots", table.name + ".h");
		stream << "#pragma once" << std::endl;

		stream << "#include \"..\\Tables\\" << table.name << ".h\"" << std::endl;

		stream << "namespace Slots ";
		stream << "{" << std::endl;
		{
			stream.push();

			//	stream << "using " << table.name << " = DataHolder<Table::" << table.name << ", "<<table.slot->layout->name << "::" << table.slot->name <<">;"<< std::endl;


			stream << "struct " << table.name << ":public DataHolder<Table::" << table.name << "," << table.slot->layout->name << "::" << table.slot->name << ">" << std::endl;//,Table::" << table.name << std::endl;
			stream << "{" << std::endl;
			{
				stream.push();

				//stream << "using Slot = " << table.slot->layout->name << "::" << table.slot->name << ";" << std::endl;
			//	stream << "using TableType = Table::" << table.name << ";" << std::endl;

				std::string pass;
				std::string copy;

				auto f = [&](ValueType type) {
					if (table.counts[type] == 0) return;
					if (pass.size()) pass += ",";
					pass += get_name_for(type);

					copy+= get_name_for(type) + " = other." + get_name_for(type) +";";
					stream << str_toupper(get_name_for(type)) << " " << get_name_for(type) << ";" << std::endl;


				};

				f(ValueType::CB);
				f(ValueType::SRV);
				f(ValueType::UAV);
				f(ValueType::SMP);

				if (table.bindless_table)
				{
					if (pass.size()) pass += ",";
					pass += "bindless";
					stream << "Render::Bindless bindless;" << std::endl;
				}


				stream << table.name << "(): "\
					"DataHolder("
					<< pass <<
					"){}" << std::endl;


				stream << table.name << "(const "<< table.name<<"&other): "\
					"DataHolder("
					<< pass <<
					"){" << copy<< "}" << std::endl;


				stream.pop();
			}

			stream << "};" << std::endl;

		}
		stream.pop();
		stream << "}" << std::endl;
	}
}


void generate_include_list(const Parsed& parsed)
{
	my_stream stream(cpp_path, "includes.h");


	stream << "#pragma once" << std::endl;

	for (auto& l : parsed.layouts)
	{
		stream << "#include \"layout\\" << l.name << ".h\"" << std::endl;
	}

	for (auto& t : parsed.tables)
	{
		if (t.slot)
			stream << "#include \"slots\\" << t.name << ".h\"" << std::endl;
	}
/*	stream << "enum class Layouts: int" << std::endl;

	stream << "{" << std::endl;
	{
		stream.push();
		stream << "UNKNOWN" << std::endl;
		for (auto& l : parsed.layouts)
		{
			stream << l.name << "," << std::endl;
		}

		stream << "TOTAL" << std::endl;
		stream.pop();
	}

	stream << "};" << std::endl;

	*/
	//	stream << "static std::array<Render::RootSignature::ptr, static_cast<int>(Layouts::TOTAL)> signatures;" << std::endl;

	stream << "void init_signatures();" << std::endl;

	/*	stream << "{" << std::endl;
		{
			stream.push();

			for (auto& l : parsed.layouts)
			{
				stream << "signatures[static_cast<int>(Layouts::"<<l.name << ")] = std::make_shared<AutoGenSignature<"<< l.name << ">>();" << std::endl;
			}

			stream.pop();
		}

		stream << "}" << std::endl;
		*/

	stream << "Render::RootLayout::ptr get_Signature(Layouts id);" << std::endl;
	/*stream << "{" << std::endl;
	stream << "\treturn signatures[static_cast<int>(id)];" << std::endl;
	stream << "}" << std::endl;
	*/
}



void generate_layout(Layout& layout)
{
	my_stream stream(hlsl_path + "/layout", layout.name + ".h");
	stream << "#pragma once" << std::endl;
	if (layout.parent_ptr) stream << "#include \"" << layout.parent_ptr->name << ".h\"" << std::endl;
	int i = 0;
	for (Sampler& s : layout.samplers) {

		stream << "SamplerState " << s.name << ":register(s" << i++ << ");" << std::endl;
	};


}


void generate_cpp_layout(Layout& layout)
{
	my_stream stream(cpp_path + "/layout", layout.name + ".h");
	stream << "#pragma once" << std::endl;

	if (layout.parent_ptr)
	{
		stream << "#include \"" << layout.parent_ptr->name << ".h\"" << std::endl;
	}

	auto slot = [&](Slot& s) {


		stream << "struct " << s.name << std::endl;
		stream << "{" << std::endl;
		{
			stream.push();

			stream << "static const unsigned int ID = " << s.id << ";" << std::endl;

			for (int i = 0; i < ValueType::STRUCT; i++)
			{
				auto type = (ValueType)i;
				auto count = s.max_counts[type];

				if(count==0)
					continue;
				stream << "static const unsigned int " << str_toupper(get_name_for(type)) << " = " << count << ";" << std::endl;
				stream << "static const unsigned int " << str_toupper(get_name_for(type)) << "_ID = " << s.ids[type] << ";" << std::endl;

			}

			stream.pop();
		}
		stream << "};" << std::endl;

	};

	if (layout.parent_ptr)
		stream << "struct " << layout.name << ": public " << layout.parent_ptr->name << std::endl;
	else
		stream << "struct " << layout.name << std::endl;
	stream << "{" << std::endl;
	{
		stream.push();

		for (auto& s : layout.slots)
			slot(s);


		stream << "template<class Processor> static void for_each(Processor& processor) {" << std::endl;
		{
			stream.push();

			std::string result_string;
			bool first = true;
			layout.recursive_slots([&](Slot& slot) {

				if (!first) result_string += ',';
				result_string += slot.name;
				first = false;
				});

			std::string params_string; first = true;
			layout.recursive_samplers([&](Sampler& slot) {

				if (!first) params_string += ',';
				params_string += "Render::Samplers::" + slot.expr;
				first = false;
				});


			stream << "processor.process<" << result_string << ">({" << params_string << "});" << std::endl;

			stream.pop();
		}

		stream << "}" << std::endl;
		stream.pop();
	}

	stream << "};" << std::endl;

}

void iterate_files(std::filesystem::path path, std::function<void(std::wstring)> f)
{
	using namespace std::filesystem;

	recursive_directory_iterator dir(path), end;

	while (dir != end)
	{
		// do other stuff here.
		if (!is_directory(*dir))
			f(dir->path().wstring());

		++dir;
	}
}

int main() {
	//std::filesystem::remove_all(L"output");

	try
	{

	iterate_files("sigs/", [](std::wstring filename) {
		parsed.merge(parse(filename));
		});

	parsed.setup();

	for (auto& table : parsed.tables)
	{
		
		generate_table(table);

		if (!table.find_option("shader_only"))
		generate_cpp_table(table);
	}

	for (auto& layout : parsed.layouts)
	{
		generate_layout(layout);
		generate_cpp_layout(layout);
	}

	generate_include_list(parsed);
	}
	catch (std::exception& e)
	{
		std::cout << "Error: " << e.what() << std::endl;
	}

	return 0;
}