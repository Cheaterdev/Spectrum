#include "Parsed.h"
#include <filesystem>

my_stream::my_stream(std::string dir, std::string filename)
{

	path = dir + "/" + filename;
	std::filesystem::create_directories(dir);


	//stream.open(dir+"/"+filename);
}

my_stream::~my_stream()
{
	auto result = stream.str();
	bool same = false;
	{
		std::ifstream f;
		f.open(path);

	
		if (f.is_open())
		{
			std::string v(std::istreambuf_iterator<char>{f}, {});

			same = (v == result);

			f.close();
		}
	}

	if (!same)
	{
		std::ofstream f;

		f.open(path);
		f << result;
		f.close();
	}
	//stream.close();
}

void my_stream::push()
{
	levels[level] = '\t';
	level++;
	levels[level] = 0;
}

void my_stream::pop()
{
	levels[level] = '\t';
	level--;
	levels[level] = 0;
}

std::string get_name_for(ValueType type)
{
	switch (type)
	{
	case ValueType::CB:
		return "cb";
	case ValueType::SRV:
		return "srv";
	case ValueType::UAV:
		return "uav";
	case ValueType::SMP:
		return "smp";
	}

	return "";
}

std::string get_cpp_for(Value v)
{

	bool noarg = v.template_arg.empty();
	std::string arg = v.type;
	switch (v.value_type)
	{
	case ValueType::STRUCT:
	case ValueType::CB:
	{
		return v.type;
	}
	case ValueType::SRV:
	{
		if (noarg)
			OutputDebugString(L"WTF");
		if (v.array_count == 0)
			return std::string("std::vector<HLSL::") + arg + ">";
		else
			return std::string("HLSL::") + arg;
	}
	case ValueType::UAV:
	{
		if (noarg)
			OutputDebugString(L"WTF");
		return std::string("HLSL::") + arg;
	}
	case ValueType::SMP:
	{	return "HAL::Handle"; }

	}

	return "";
}

void have_type::detect_type(have_options * options)
{
	value_type = ValueType::STRUCT;
	if (type.starts_with("Texture"))value_type = ValueType::SRV;
	if (type.starts_with("StructuredBuffer")) value_type = ValueType::SRV;
	if (type.starts_with("Buffer")) value_type = ValueType::SRV;
	if (type.starts_with("RaytracingAccelerationStructure")) value_type = ValueType::SRV;
	if (type.starts_with("ConstantBuffer")) value_type = ValueType::SRV;
	
	if (type.starts_with("RW")) value_type = ValueType::UAV;
	if (type.starts_with("AppendStructuredBuffer")) value_type = ValueType::UAV;
	
	if (type.starts_with("bool")) value_type = ValueType::CB;
	if (type.starts_with("uint")) value_type = ValueType::CB;
	if (type.starts_with("int")) value_type = ValueType::CB;
	if (type.starts_with("float")) value_type = ValueType::CB;
	if (type.starts_with("matrix")) value_type = ValueType::CB;
	if (type.starts_with("uint")) value_type = ValueType::CB;
	if (type.starts_with("mat4x4")) value_type = ValueType::CB;

	if (type == "SamplerState") value_type = ValueType::SMP;

	if(options&&options->find_option("dynamic"))
		value_type = ValueType::CB;
}

Slot::Slot()
{
	max_counts[ValueType::SMP] = 0;
	max_counts[ValueType::UAV] = 0;
	max_counts[ValueType::SRV] = 0;
	max_counts[ValueType::CB] = 2;
	max_counts[ValueType::STRUCT] = 0;


	ids[ValueType::SMP] = -1;
	ids[ValueType::UAV] = -1;
	ids[ValueType::SRV] = -1;
	ids[ValueType::CB] = -1;
	ids[ValueType::STRUCT] = -1;

}

Slot* Layout::find_slot(std::string name)
{
	for (auto& t : slots)
		if (t.name == name)
			return &t;

	return nullptr;
}

void Layout::set_slots(int offset /*= 0*/)
{
	for (auto& s : slots)
	{
		s.id = offset++;
	}

	for (auto& c : child_layouts)
	{
		c->set_slots(offset);
	}
}

void Layout::setup()
{
	if (parent_ptr)
		types_counts = parent_ptr->types_counts;
	for (auto& s : slots)
	{
		for (int type = 0; type < ValueType::COUNT; type++)
		{
			auto count = s.max_counts[type];

			if (type == ValueType::STRUCT) continue;
			if (s.max_counts[type])
			{
				s.ids[type] = types_counts++;

				// for offset, remove later, use cb
				if (type == ValueType::CB) types_counts++;
			}
		}



	}

	for (auto& l : child_layouts)
		l->setup();
}

void Table::setup(Parsed* all)
{
	counts[ValueType::SMP] = 0;
	counts[ValueType::UAV] = 0;
	counts[ValueType::SRV] = 0;
	counts[ValueType::CB] = 0;

	offsets[ValueType::SMP] = 0;
	offsets[ValueType::UAV] = 0;
	offsets[ValueType::SRV] = 0;
	offsets[ValueType::CB] = 0;


	auto opt_provided = find_option("CB");

	if (opt_provided)
	{
		cb_raw = true;
		counts[ValueType::CB] = 1;
		offsets[ValueType::CB] = 1;
		cb_provided = true;
	}

	values.sort([](const Value&a, const Value &b) {
		if (b.array_count == 0) return true;
		if (a.array_count == 0) return false;
		return false;
		});
		


	for (auto& v : values)
	{
		if (v.value_type == ValueType::STRUCT)
		{
			used_tables.insert(v.type);
		}


		if (!v.template_arg.empty())
		{
			if (all->find_table(v.template_arg))
				used_tables.insert(v.template_arg);
		}

		if (v.as_array && v.array_count == 0)
		{
			bindless_srv = &v;
			bindless_table = this;
		}
	}

	for (auto& v : values)
	{

		if (v.value_type == ValueType::STRUCT)
		{
			auto t = all->find_table(v.type);
			t->setup(all);


			for (int i = 0; i < ValueType::COUNT; i++)
			{
				auto type = (ValueType)i;
				auto count = t->counts[type];
				counts[type] += count;
			}

			if (t->bindless_table)
				throw std::exception("unsupported, wait for GetResourceFromHeap");
			if (t->bindless_srv || t->bindless_table)
				bindless_table = t;
		}
		else
			counts[v.value_type]+=v.array_count;

		if (slot)
		{
			if (v.value_type == ValueType::STRUCT)
			{
				for (int i = 0; i < ValueType::STRUCT; i++)
				{
					slot->max_counts[i] = std::max(slot->max_counts[i], counts[i]);

				}
			}
			else

			slot->max_counts[v.value_type] = std::max(slot->max_counts[v.value_type], counts[v.value_type]);

			
		}
	}



	if (bindless_srv)
		slot->max_counts[ValueType::SRV] = std::max(slot->max_counts[ValueType::SRV], 1);


	//check aligning
	if (slot)
	{
	for (auto& v : values)
	{
		int offset = 0;

		if (v.value_type == ValueType::CB)
		{
			//assert(offset == 0);
			offset += v.size;

		}

		offset = offset % 4;
	}
	}
}
RaytracePSO* Parsed::find_rtx(std::string name)
{
	for (auto& t : raytrace_pso)
		if (t.name == name)
			return &t;

	return nullptr;
}

Layout* Parsed::find_layout(std::string name)
{
	for (auto& t : layouts)
		if (t.name == name)
			return &t;

	return nullptr;
}

Table* Parsed::find_table(std::string name)
{
	for (auto& t : tables)
		if (t.name == name)
			return &t;

	return nullptr;
}

void Parsed::setup()
{
	for (auto& l : layouts)
	{
		if (!l.parent.empty()) {
			l.parent_ptr = find_layout(l.parent.front());
			l.parent_ptr->child_layouts.push_back(&l);
		}
		for (auto& s : l.slots)
		{
			s.layout = &l;
		}
	}


	for (auto& l : layouts)
	{
		if (!l.parent.empty())
			l.parent_ptr = find_layout(l.parent.front());
		else
			root_layouts.push_back(&l);

		for (auto& s : l.slots)
		{
			s.layout = &l;
		}
	}


	for (auto& l : root_layouts)
		l->set_slots();


	for (auto& t : tables)
	{
		for (auto& o : t.options)
		{
			if (o.name == "Bind")
			{
				t.slot = find_layout(o.value_atom.owner_name)->find_slot(o.value_atom.expr);
				if(!t.slot)
					throw std::runtime_error("cannot find slot " +  o.value_atom.owner_name + " " + o.value_atom.expr);
			}
		}
		t.setup(this);
	}

	for (auto& layout : root_layouts)
	{
		layout->setup();
	}

}
