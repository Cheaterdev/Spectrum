#include <iostream>
#include <algorithm>
#include "Parsed.h"
#include "Parsing.h"
#include <filesystem>
#include <functional>
#include <format>
#include <string>
#include <iostream>
static const std::string cpp_path = "../../sources/DirectXFramework/autogen";
static const std::string hlsl_path = "../../workdir/shaders/autogen";

Parsed parsed;
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
void generate_pass(my_stream& stream, std::array<std::stringstream, ValueType::COUNT > &struct_str, Table& table, table_offsets& offsets, Slot& slot)
{
	auto generate = [&](ValueType type, std::string subname) {
		for (auto& v : table.values)
		{
			if (v.value_type != type) continue;
			if (v.bindless)  continue;

			auto& e = offsets[type];

			stream<<std::format("{} {}_{}_{}{}: register({}{}, space{});", v.type, get_name_for(type), slot.id,e, generate_array(v), subname, e, slot.id) << std::endl;
			struct_str[type] << std::format("uint {}_{};", get_name_for(type), e) << std::endl;

			e += v.array_count;
		}
	};



	generate(ValueType::SRV, "t");
	generate(ValueType::UAV, "u");
	generate(ValueType::SMP, "s");

	for (auto& v : table.values)
	{
		if (v.value_type != ValueType::STRUCT) continue;

		generate_pass(stream, struct_str , *parsed.find_table(v.type), offsets, slot);
	}



}

void generate_pass_table(Table& table)
{
	my_stream stream(hlsl_path, table.name + ".h");
	stream << std::format(
R"(#ifndef SLOT_{0}
	#define SLOT_{0}
#else
	#error Slot {0} is already used
#endif)", table.slot->id) << std::endl;


	stream << std::format(R"(#include "layout/{}.h")" ,table.slot->layout->name)<< std::endl;

//	if (table.bindless_srv)
	//	stream << std::format(R"({} bindless[]: register(t{}, space{});)", table.bindless_srv->type , calculate_max_size(table) , table.slot->id) << std::endl;

	stream << std::format(R"(#include "tables/{}.h")", table.name) << std::endl;


	// cb
//	if (table.counts[ValueType::CB] > 0) stream << std::format(R"(ConstantBuffer<{}_cb> cb_{}_0:register(b0,space{});)", table.name , table.slot->id ,table.slot->id ) << std::endl; // "ConstantBuffer<" << table.name << "_cb> cb_" << table.slot->id << "_0:register(b0,space" << table.slot->id << ");" << std::endl;



	table_offsets offsets;
	offsets[ValueType::SMP] = 0;
	offsets[ValueType::UAV] = 0;
	offsets[ValueType::SRV] = 0;
	offsets[ValueType::CB] = 0;


	std::stringstream struct_str;

	struct_str << "ConstantBuffer<" << table.name << "> pass_" << table.name << ": register( b2, space" << table.slot->id << ");" << std::endl;

	stream << struct_str.str();

	//creation func

	{
		stream << "const "<<table.name << " Create" << table.name << "()" << std::endl;
		stream << "{" << std::endl;
		{
			stream.push();

			stream << "return pass_"<< table.name << ";" << std::endl;
			stream.pop();
		}
		stream << "}" << std::endl;
	}

	std::string cameled = table.name;
	cameled[0] = std::toupper(cameled[0]);

	std::string lowcameled = table.name;
	lowcameled[0] = std::tolower(lowcameled[0]);


	stream << "#ifndef NO_GLOBAL" << std::endl;

	stream << "static const " << table.name << " " << lowcameled << "_global = Create" << table.name << "();" << std::endl;
	stream << "const " << table.name << " Get" << cameled << "(){ return " << lowcameled << "_global; }" << std::endl;
	stream << "#endif" << std::endl;


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
	//	if (table.counts[type] == 0) return;
	
		for (auto& v : table.values)
		{
			if (v.value_type != type) continue;
		//	if (v.bindless) continue;


			if(v.value_type==ValueType::CB || v.value_type == ValueType::STRUCT)
			stream << v.type << " " << v.name <<generate_array(v) << "; // " << v.type << std::endl;
			else
				stream << "uint" << " " << v.name << (v.bindless?"":generate_array(v)) << "; // " << v.type << std::endl;
		//		stream << v.type<<" " << v.name << ';' << std::endl;
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
			if (v.value_type != ValueType::CB&& v.value_type != ValueType::STRUCT) continue;
			std::string cameled = v.name;
			cameled[0] = std::toupper(cameled[0]);


			if (v.as_array)
			{
				stream << v.type << " Get" << cameled << "(int i) { " << "return "  << v.name << "[i]; }" << std::endl;
			}
			else
			{
				stream << v.type << " Get" << cameled << "() { " << "return " << v.name << "; }" << std::endl;

			}
		}


		for (auto& v : table.values)
		{
			if (v.value_type == ValueType::STRUCT) continue;
			if (v.value_type == ValueType::CB) continue;
			std::string cameled = v.name;
			cameled[0] = std::toupper(cameled[0]);


			if (v.as_array)
			{
				if (v.bindless)
				{
					stream << v.type << " Get" << cameled << "(int i) { " << std::endl;
					stream << "StructuredBuffer<uint> indirection = ResourceDescriptorHeap["<<v.name<<"]; " << std::endl;
						stream << "uint id = indirection.Load(i);" << std::endl;
						stream << "return ResourceDescriptorHeap[id]; }" << std::endl;
				}
				else
					stream << v.type << " Get" << cameled << "(int i) { " << "return ResourceDescriptorHeap[" <<  v.name <<   "[i]]; }" << std::endl;
			}
			else
			{
				stream << v.type << " Get" << cameled << "() { " << "return ResourceDescriptorHeap[" << v.name <<  "]; }" << std::endl;

			}
		}


	}


	if(!table.hlsl.empty())
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
	stream << "#pragma once" << std::endl;

	for (auto& v : table.used_tables)
	{
		stream << "#include \"" << v << ".h\"" << std::endl;
	}

	// declaration
	auto declare_func = [&](ValueType type) {
		if (table.counts[type] == 0) return;

		for (auto& v : table.values)
		{
			if (v.value_type != type) continue;
			if (v.bindless) continue;

			stream << v.type << " " << v.name << ';' << std::endl;
		}

		for (auto& v : table.values)
		{
			if (v.value_type == ValueType::STRUCT)
			{
				auto t = parsed.find_table(v.type);
				if (t->counts[type] == 0) continue;

				if (t->find_option("nobind"))
					stream << v.type << " " << v.name << ";" << std::endl;
				else

					stream << v.type << "_" << get_name_for(type) << " " << v.name << ";" << std::endl;
			}
		}

	};






	// result struct
	stream << "struct " << table.name << std::endl;

	stream << "{" << std::endl;

	{
		stream.push();


		declare_func(ValueType::CB);

		stream.pop();

	}

	stream << table.hlsl << std::endl;


	stream << "};" << std::endl;



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

		for (auto& v : table.values)
		{
			if (v.find_option("dynamic")) continue;
			if (v.value_type != type) continue;
			//if (v.array_count == 0)
			//	continue;


			stream << get_cpp_for(v) << " " << v.name << generate_array(v) << ';' << std::endl;
		}

		if (type == ValueType::CB)
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
					stream << get_cpp_for(v) << generate_cpp_array(v) << " Get" << cameled << "() { " << "return " <<  v.name << "; }" << std::endl;
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
				//	stream << '\t' << v.type << " " << v.name << ';' << std::endl;



				auto& vtable = *parsed.find_table(v.type);




				std::string cameled = v.name;
				cameled[0] = std::toupper(cameled[0]);

				if (vtable.find_option("shader_only"))
				{
					stream << v.type << generate_cpp_array(v) << " Get" << cameled << "() { " << "return " <<  v.name << "; }" << std::endl;
					continue;
				};


				stream << v.type << "& Get" << cameled << "() { " << "return "  << v.name << "; }" << std::endl;
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


			stream << "struct " << table.name << ":public DataHolder<"<< table.name<<", SlotID::" << table.name <<",Table::" << table.name << "," << table.slot->layout->name << "::" << table.slot->name << ">" << std::endl;//,Table::" << table.name << std::endl;
			stream << "{" << std::endl;
			{
				stream.push();
			

				stream << table.name << "() = default;" << std::endl;
	

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
	{
		my_stream stream(cpp_path, "autogen.ixx");


	stream << R"(
module;


export module Graphics:Autogen;

import :PipelineState;
import Math;
import :Samplers;
import :Buffer;
import :Descriptors;


import :SIG;
import :RT;
import :Layout;
import :Slots;
import :PSO;
import :RTX;
import :Enums;

import Data;
import :RootSignature;

import Utils;
import serialization;
import :Types;

import :Definitions;
export
{

)" << std::endl;

	stream.push();
	for (auto& l : parsed.layouts)
	{
		stream << "#include \"layout\\" << l.name << ".h\"" << std::endl;
	}

	for (auto& t : parsed.tables)
	{
		if (t.slot)
			stream << "#include \"slots\\" << t.name << ".h\"" << std::endl;
		else if (!t.find_option("shader_only"))
			stream << "#include \"tables\\" << t.name << ".h\"" << std::endl;
	}


	for (auto& t : parsed.rt)
	{

		stream << "#include \"rt\\" << t.name << ".h\"" << std::endl;
	}


	for (auto& t : parsed.compute_pso)
	{

		stream << "#include \"pso\\" << t.name << ".h\"" << std::endl;
	}

	for (auto& t : parsed.graphics_pso)
	{

		stream << "#include \"pso\\" << t.name << ".h\"" << std::endl;
	}

	for (auto& t : parsed.raytrace_pso)
	{

		stream << "#include \"rtx\\" << t.name << ".h\"" << std::endl;
	}


	stream << "void init_signatures();" << std::endl;
	stream << "HAL::RootLayout::ptr get_Signature(Layouts id);" << std::endl;
	stream << "void init_pso(enum_array<PSO, PSOBase::ptr>&);" << std::endl;

	stream << "std::optional<SlotID> get_slot(std::string_view slot_name);" << std::endl;
	stream << "UINT get_slot_id(SlotID id);" << std::endl;

	stream.pop();
	stream << "}" << std::endl;

//	stream << "module: private;" << std::endl;

	stream << "static enum_array<Layouts, HAL::RootLayout::ptr> signatures;" << std::endl;
	stream << "void init_signatures()" << std::endl;

	stream << "{" << std::endl;
	{
		stream.push();

		for (auto& l : parsed.layouts)
		{
			stream << "signatures[Layouts::" << l.name << "] = AutoGenSignatureDesc<" << l.name << ">().create_signature(Layouts::" << l.name << ");" << std::endl;
		}

		stream.pop();
	}

	stream << "}" << std::endl;


	stream << "HAL::RootLayout::ptr get_Signature(Layouts id)" << std::endl;
	stream << "{" << std::endl;
	stream << "\treturn signatures[id];" << std::endl;
	stream << "}" << std::endl;

	{
		stream << "std::optional<SlotID> get_slot(std::string_view slot_name)" << std::endl;
		stream << "{" << std::endl;

		stream.push();
		{

			for (auto& t : parsed.tables)
			{
				if (!t.slot) continue;

				stream << "if(slot_name == \"" << t.name << "\")" << std::endl;
				stream << "{" << std::endl;
				stream.push();
				stream << "return SlotID::" << t.name << ";" << std::endl;
				stream.pop();
				stream << "}" << std::endl;

			}
			stream << "return std::nullopt;" << std::endl;

		}
		stream.pop();
		stream << "}" << std::endl;


}

	{
		my_stream stream(cpp_path, "enums.ixx");
		stream << R"(
module;

export module Graphics:Enums;

import crc32;
import Utils;

export
{
)" << std::endl;
		stream.push();
		stream << "enum class Layouts: int" << std::endl;

		stream << "{" << std::endl;
		{
			stream.push();
			stream << "UNKNOWN," << std::endl;
			for (auto& l : parsed.layouts)
			{
				stream << l.name << "," << std::endl;
			}

			stream << "TOTAL" << std::endl;
			stream << ", GENERATE_OPS" << std::endl;
			stream.pop();
		}

		stream << "};" << std::endl;

		stream << "enum class PSO: int" << std::endl;

		stream << "{" << std::endl;
		{
			stream.push();
			stream << "UNKNOWN," << std::endl;
			for (auto& l : parsed.compute_pso)
			{
				stream << l.name << "," << std::endl;
			}
			for (auto& l : parsed.graphics_pso)
			{
				stream << l.name << "," << std::endl;
			}
			stream << "TOTAL" << std::endl;
			stream << ", GENERATE_OPS" << std::endl;
			stream.pop();
		}

		stream << "};" << std::endl;


		stream << "enum class SlotID: unsigned int" << std::endl;

		stream << "{" << std::endl;
		{
			stream.push();
			
			for (auto& t : parsed.tables)
			{
				stream << t.name << " = \""<< t.name<<"\"_crc32 ," << std::endl;
			}
		
			stream << "GENERATE_OPS" << std::endl;
			stream.pop();
		}

		stream << "};" << std::endl;

		stream.pop();
		stream << "}" << std::endl;
	}
	


		{
			stream << "UINT get_slot_id(SlotID id)" << std::endl;

			stream << "{" << std::endl;

			stream.push();
			{

				for(auto &t:parsed.tables)
				{
					if (!t.slot) continue;

					stream << "if(id == SlotID::" << t.name<<")" << std::endl;
					stream << "{" << std::endl;
					stream.push();
					stream << "return Slots::"<<t.name<<"::Slot::ID;" << std::endl;
					stream.pop();
					stream << "}" << std::endl;
					
				}
				stream << "return -1;" << std::endl;

			}
			stream.pop();
			stream << "}" << std::endl;


		}
		
	}

	
	{
		my_stream stream(cpp_path, "pso.cpp");
		stream << R"(


import HAL;
import Data;

import ppl;
using namespace concurrency;
)"
			<< std::endl;

		//stream << "static std::array<HAL::ComputePipelineState::ptr, static_cast<int>(PSO::TOTAL)> pso;" << std::endl;
		stream << "void init_pso(enum_array<PSO, PSOBase::ptr>& pso)" << std::endl;

		stream << "{" << std::endl;
		{
			stream.push();
			stream << " std::vector<task<void>> tasks;" << std::endl;
			for (auto& l : parsed.compute_pso)
			{

				//stream << "pso[PSO::" << l.name << "] =  std::make_shared<PSOS::" << l.name << ">();" << std::endl;

				stream << "tasks.emplace_back(PSOBase::create<PSOS::" << l.name << ">(pso[PSO::" << l.name << "]));" << std::endl;

			}
			for (auto& l : parsed.graphics_pso)
			{
				//stream << "pso[PSO::" << l.name << "] =  std::make_shared<PSOS::" << l.name << ">();" << std::endl;

				stream << "tasks.emplace_back(PSOBase::create<PSOS::" << l.name << ">(pso[PSO::" << l.name << "]));" << std::endl;
			}


			stream << " when_all(begin(tasks), end(tasks)).wait();" << std::endl;
			stream.pop();
		}

		stream << "}" << std::endl;

/*
		stream << "HAL::ComputePipelineState::ptr get_PSO(PSO id)" << std::endl;
		stream << "{" << std::endl;
		stream << "\treturn pso[static_cast<int>(id)];" << std::endl;
		stream << "}" << std::endl;
		*/

	}
	
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

void generate_rt(RenderTarget& rt)
{
	my_stream stream(hlsl_path + "/rt", rt.name + ".h");
	stream << "#pragma once" << std::endl;

	stream << "struct " << rt.name << std::endl;
	stream << "{" << std::endl;
	{
		stream.push();

		int i = 0;

		for (auto& e : rt.rtvs)
		{
			stream << e.type << " " << e.name << ": SV_Target" << i << ";" << std::endl;
			i++;
		}

		stream.pop();
	}
	stream << "};" << std::endl;
}
/*
void generate_pso(PSO& pso)
{

	my_stream stream(cpp_path + "/pso", pso.name + ".h");
	stream << "#pragma once" << std::endl;

	stream << "namespace Autogen" << std::endl;
	stream << "{" << std::endl;
	{
		stream.push();

		if (pso.parent.size())
		{
			stream << "struct " << pso.name << ": public "<< pso.parent[0]<< std::endl;
		}
		else
		{
			stream << "struct " << pso.name << std::endl;
		}

		stream << "{" << std::endl;
		{
			stream.push();

			if(!pso.root_sig.name.empty())
			stream << "static inline const Layouts layout = Layouts::" << pso.root_sig.name << ";" << std::endl;


			if (!pso.compute.name.empty())
			{
				stream << "static inline const HAL::shader_header compute = { \"shaders/" << pso.compute.name << ".hlsl\", \"" << pso.compute.find_option("EntryPoint")->value_atom.expr << "\", 0,{";

				bool first = true;
				for (auto option : pso.compute.options)
				{
					if (!first) stream << ",";
					stream << "{";
					stream << "\"" << option.name << "\", \"" << option.value_atom.expr << "\"";
					stream << "}";

					first = false;
				}

				stream << "} }; " << std::endl;


			}


			stream.pop();
		}
		stream << "};" << std::endl;

		stream.pop();
	}
	stream << "}" << std::endl;
}
*/
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
				if (opt->value_atom.expr=="Format")
					keys += "HAL::";
				keys += opt->value_atom.expr;
				keys += ">";
			}else

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
				
		}else
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

		if(d.find_option("CS"))
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

			stream << "struct Keys {" << std::endl << keys << std::endl <<" 		GEN_DEF_COMP(Keys) };" << std::endl;

			std::string GEN_PSO;
			if (dynamic_cast<ComputePSO*>(&pso))
			{
				GEN_PSO = "GEN_COMPUTE_PSO";
			}
			else
			{
				GEN_PSO = "GEN_GRAPHICS_PSO";
			}
			if(keyslist.empty())
			stream << GEN_PSO<<"(" << pso.name << ")"<< std::endl;
			else
				stream << GEN_PSO<< "(" << pso.name <<","<< keyslist << ")" << std::endl;
			stream << keysgenerators << std::endl;
			stream << "SimplePSO init_pso(Keys & key)"<< std::endl;
			stream << "{" << std::endl;
			{
				stream.push();
				stream << defines << std::endl;


				stream << "SimplePSO mpso(\"" << pso.name << "\");" << std::endl;

				stream << "mpso.root_signature = Layouts::" << pso.root_sig.name << ";" << std::endl;
				for (auto& shader : pso.shader_list)
				{

				
					stream << "mpso."<< shader.type<<".file_name = \"shaders/" << shader.name << ".hlsl\";" << std::endl;
					stream << "mpso." << shader.type << ".entry_point = \"" << shader.find_option("EntryPoint")->value_atom.expr << "\";" << std::endl;
					stream << "mpso." << shader.type << ".flags = 0;" << std::endl;

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

		if (e.type == "ds")
							add = "HAL::Format::";
						if (e.type == "cull")
							add = "HAL::CullMode::";
						else if (e.type == "depth_func")
							add = "HAL::ComparisonFunc::";
						else if (e.type == "topology")
							add = "HAL::PrimitiveTopologyType::";
						else if (e.type == "stencil_func")
							add = "HAL::ComparisonFunc::";
						else if (e.type == "stencil_pass_op")
							add = "HAL::StencilOp::";

						stream << "mpso."<<e.type << "  = "<<add << e.expr << ";" << std::endl;
					}
				}
				stream << "return mpso;" << std::endl;
				stream.pop();
			}
			stream << "}" << std::endl;


			stream.pop();
		}
		stream << "};" << std::endl;

		stream.pop();
	}
	stream << "}" << std::endl;
}



void generate_cpp_rt(RenderTarget& rt)
{
	my_stream stream(cpp_path + "/rt", rt.name + ".h");
	stream << "#pragma once" << std::endl;
	stream << "namespace RT" << std::endl;
	stream << "{" << std::endl;
	{
		stream.push();
		stream << "namespace Table" << std::endl;
		stream << "{" << std::endl;
		{
			stream.push();
			stream << "struct " << rt.name << std::endl;
			stream << "{" << std::endl;
			{
				stream.push();
				if (rt.rtvs.size())
				{
					stream << "struct RTV" << std::endl;
					stream << "{" << std::endl;
					{
						stream.push();
						int i = 0;
						for (auto& e : rt.rtvs)
						{
							stream << "HAL::Handle" << " " << e.name << ";" << std::endl;
							i++;
						}

						stream.pop();
					}
					stream << "} &rtv;" << std::endl;
				}

				if (rt.dsv)
				{
					stream << "struct DSV" << std::endl;
					stream << "{" << std::endl;
					{
						stream.push();

						stream << "HAL::Handle" << " " << rt.dsv->name << ";" << std::endl;

						stream.pop();
					}
					stream << "}&dsv;" << std::endl;
				}



				for (auto& e : rt.rtvs)
				{
					std::string cameled = e.name;
					cameled[0] = std::toupper(cameled[0]);


					stream << "HAL::Handle&" << " Get" << cameled << "() { " << "return " << "rtv." << e.name << "; }" << std::endl;
				}

				if (rt.dsv)
				{
					std::string cameled = rt.dsv->name;
					cameled[0] = std::toupper(cameled[0]);


					stream << "HAL::Handle&" << " Get" << cameled << "() { " << "return " << "dsv." << rt.dsv->name << "; }" << std::endl;

				}
				std::string pass;
				std::string args;

				if (rt.rtvs.size())
				{
					if (!args.empty()) args += ", ";
					if (!pass.empty()) pass += ", ";

					args += "RTV & rtv";
					pass += "rtv(rtv)";
				}
				if (rt.dsv)
				{
					if (!args.empty()) args += ", ";
					if (!pass.empty()) pass += ", ";

					args += "DSV & dsv";
					pass += "dsv(dsv)";
				}
				stream.pop();


				stream << rt.name << "(" << args << "):" << pass << "{}" << std::endl;

			}
			stream << "};" << std::endl;

			stream.pop();
		}
		stream << "}" << std::endl;


		stream << "namespace Slot" << std::endl;
		stream << "{" << std::endl;
		{
			stream.push();

			stream << "struct " << rt.name << ": public RTHolder<Table::" << rt.name << ">" << std::endl;
			stream << "{" << std::endl;
			{
				stream.push();
				std::string pass;
			
				if (rt.rtvs.size())
				{
					if (!pass.empty()) pass += ", ";

					stream << "RTV rtv;" << std::endl;
					pass += "rtv";

			
				}

				if (rt.dsv)
				{
					if (!pass.empty()) pass += ", ";
					pass += "dsv";
					stream << "DSV dsv;" << std::endl;
				}

		
		
				stream << rt.name << "():RTHolder<Table::" << rt.name << ">(" << pass << "){}" << std::endl;
				stream.pop();
			}
			stream << "};" << std::endl;

			stream.pop();
		}
		stream << "}" << std::endl;

		stream.pop();

	}
	stream << "}" << std::endl;


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

			std::string tables;
			
			for (int i = 0; i < ValueType::STRUCT; i++)
			{
				auto type = (ValueType)i;
				auto count = s.max_counts[type];

				if (count == 0)
					continue;

				stream << "static const unsigned int " << str_toupper(get_name_for(type)) << " = " << count << ";" << std::endl;
				stream << "static const unsigned int " << str_toupper(get_name_for(type)) << "_ID = " << s.ids[type] << ";" << std::endl;

				if (!tables.empty())
					tables += ", ";

				tables += std::to_string(s.ids[type]);

			}


			stream << "static inline const std::vector<UINT> tables = {" << tables << "};" << std::endl;

			
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
				params_string += "HAL::Samplers::" + slot.expr;
				first = false;
				});


			stream << "processor.template process<" << result_string << ">({" << params_string << "});" << std::endl;

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


void generate_raygen(RaytraceGen & pso)
{
	my_stream stream(cpp_path + "/rtx", pso.name + ".h");
	stream << "#pragma once" << std::endl;

	stream << std::format(R"(struct {0}: public RaytraceRaygen<{0}>)", pso.name) << std::endl;
	stream << "{" << std::endl;
	stream.push();
	{
		stream << std::format(R"(static const constexpr UINT ID = {};)", pso.index) << std::endl;

		stream << std::format(R"(static const constexpr std::string_view shader = "shaders\\{}.hlsl";)", pso.shaders["raygen"]->name) << std::endl;
		stream << std::format(R"(static const constexpr std::wstring_view raygen = L"{}";)", pso.shaders["raygen"]->find_option("EntryPoint")->value_atom.expr) << std::endl;

	}
	stream.pop();
	stream << "};" << std::endl;
	/*
	struct Shadow :public RaytraceRaygen<Shadow>
	{
		static const constexpr std::string_view shader = "shaders\\raytracing.hlsl";
		static const constexpr std::wstring_view raygen = L"ShadowPass";
	};*/



}
void generate_pass(RaytracePass& pso)
{
	my_stream stream(cpp_path + "/rtx", pso.name + ".h");
	stream << "#pragma once" << std::endl;


	stream << std::format(R"(struct {0}: public RaytracePass<{0}>)", pso.name) << std::endl;
	stream << "{" << std::endl;
	stream.push();
	{
		stream << std::format(R"(using Payload = Table::{};)", pso.find_param("payload")->expr) << std::endl;

		if(pso.find_param("local"))
		stream << std::format(R"(using LocalData =  Slots::{};)", pso.find_param("local")->expr) << std::endl;

		stream << std::format(R"(static const constexpr UINT ID = {};)", pso.index) << std::endl;

		stream << std::format(R"(static const constexpr std::string_view shader = "shaders\\{}.hlsl";)", pso.shaders["miss"]->name) << std::endl;
		stream << std::format(R"(static const constexpr std::wstring_view name = L"{}_GROUP";)", pso.name) << std::endl;

		stream << std::format(R"(static const constexpr std::wstring_view hit_name = L"{}";)", pso.shaders["closest_hit"]->find_option("EntryPoint")->value_atom.expr) << std::endl;
		stream << std::format(R"(static const constexpr std::wstring_view miss_name = L"{}";)", pso.shaders["miss"]->find_option("EntryPoint")->value_atom.expr) << std::endl;


		auto param = pso.find_param("per_material");
		stream << std::format(R"(static const constexpr bool per_material = {};)", param&& param->expr=="true"?"true":"false") << std::endl;

	}
	stream.pop();
	stream << "};" << std::endl;
}



void generate_hlsl_pass(RaytracePass& pso)
{
	my_stream stream(hlsl_path + "/rtx", pso.name + ".h");
	auto payload = pso.find_param("payload")->expr;

	stream << std::format(R"(void {}(RaytracingAccelerationStructure scene, RayDesc ray, RAY_FLAG flag, inout {} payload) )", pso.name, payload) << std::endl;
	stream << "{" << std::endl;
	stream.push();
	{
		stream << std::format(R"(TraceRay(scene, flag, ~0, {0}, 0, {0}, ray, payload); )", pso.index) << std::endl;
	}
	stream.pop();
	stream << "};" << std::endl;

}


void generate_rtx_pso(RaytracePSO& pso)
{
	my_stream stream(cpp_path + "/rtx", pso.name + ".h");
	stream << "#pragma once" << std::endl;

	std::string gens, passes;

	for (auto e : pso.gens)
	{
		if (!gens.empty())gens += ", ";
		gens += e.name;

		stream << std::format(R"(#include "{}.h")", e.name) << std::endl;

	}

	for (auto e : pso.passes)
	{
		if (!passes.empty())passes += ", ";
		passes += e.name;

		stream << std::format(R"(#include "{}.h")", e.name) << std::endl;
	}

	stream << std::format(R"(struct {0}: public RTXPSO<{0}, Typelist<{1}>, Typelist<{2}>>)", pso.name, passes, gens) << std::endl;
	stream << "{" << std::endl;
	stream.push();
	{
		stream << std::format(R"(static const constexpr Layouts global_sig  = Layouts::{};)", pso.root_sig.name) << std::endl;
		stream << std::format(R"(static const constexpr UINT MaxTraceRecursionDepth = 2;)") << std::endl;

	}
	stream.pop();
	stream << "};" << std::endl;
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

		for (auto& table : parsed.tables)
		{

			if (table.find_option("nobind"))
				generate_nobind_table(table);

			else
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

		for (auto& rt : parsed.rt)
		{
			generate_rt(rt);
			generate_cpp_rt(rt);
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