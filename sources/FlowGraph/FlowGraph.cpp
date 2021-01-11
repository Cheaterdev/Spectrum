// FlowGraph.cpp : Defines the entry point for the console application.
//
#include "pch.h"
#include "FlowGraph.h"
#include "Math/ScalarMath.h"

namespace FlowGraph
{
	void graph::remove_node(std::shared_ptr<window>node)
	{
		node->on_remove();

		if (node->get_ptr<Node>())
			nodes.erase(node->get_ptr<Node>());
		else
			windows.erase(node);
	}

	void graph::clear()
	{


		for (auto window : windows)
		{
			window->on_remove();
		}

		for (auto node:nodes)
		{
			node->on_remove();
		}

		nodes.clear();
		windows.clear();
	}

	/*input::ptr graph::register_input( std::string name)
	{

		graph_input::ptr i(new graph_input(this));
		i->name = name;
		//i->type = type;
		input_parametres.push_back(i);

		for (auto listener : listeners)
			listener->on_add_input(i.get());

		return i;
	}

	output::ptr graph::register_output( std::string name)
	{

		output::ptr p(new graph_output(this));
	//	p->type = type;
		p->name = name;
		output_parametres.push_back(p);

		for (auto listener : listeners)
			listener->on_add_output(p.get());

		return p;
	}*/

	graph::graph()
	{
		// owner = this;
	}

	graph::~graph()
	{
		connections.clear();
		nodes.clear();
	}

	void graph::reset_for()
	{
		Node::reset_for();

		for (auto n : nodes)
			n->reset_for();

		output_count = 0;
		connected_outputs = 0;

		for (auto o : output_parametres)
			connected_outputs += o->input_connections.size();

		//TODO: work here
		//     test_start();
	}

	void graph::start(GraphContext * context)
	{
		this->context = context;
		reset_for();

		if (start_child_nodes)
			for (auto n : nodes)
			{
				graph* g = dynamic_cast<graph*>(n.get());

				if (g)
					g->start(context);
				else
				{
					if(!n->has_inputs())
					context->add_task(n.get());
				}
			}

		if (!owner)
		{
			(*this)(context);

			if (connected_outputs == 0)
				on_finish();
		}
	}

	void graph::register_node(std::shared_ptr<window> node)
	{
		if (node->owner && node->owner != this)
			node->remove();

		node->on_add(this);

		if (node->get_ptr<Node>())
			nodes.insert(node->get_ptr<Node>());
		else
			windows.insert(node);

		for (auto listener : listeners)
			node->add_listener(listener, true);
	}



	Node::~Node()
	{
		shutdown();
		/*	for (auto o : output_parametres)
				o->unlink_all();

			for (auto i : input_parametres)
				i->unlink_all();
				*/
	}

	void Node::reset_for()
	{
		setted_params = 0;

		for (auto p : input_parametres)
			p->clear();

		for (auto p : output_parametres)
			p->clear();

		//TODO: work here
		//     test_start();
	}

	void Node::shutdown()
	{
		for (auto && e : input_parametres)
			e->shutdown();

		for (auto && e : output_parametres)
			e->shutdown();
	}


	bool Node::test_start()
	{
		if (start_if_output)
		{
			bool start = false;

			for (auto& p : output_parametres)
			{
				if (p->enabled && p->output_connections.size())
					start = true;
			}

			if (!start) return false;
		}

		auto res = setted_params == input_parametres.size();

		if (owner && res)
		{
			setted_params = 0;
			owner->add_task(this);
		}

		return res;
	}

	void Node::on_input(input * i)
	{
		setted_params++;
		test_start();
	}

	void Node::on_output(output * i)
	{
	}

	void window::on_add(graph* g)
	{
		this->owner = g;
	}

	void Node::on_remove()
	{
		for (auto o : output_parametres)
			o->unlink_output();

		for (auto i : input_parametres)
			i->unlink_input();

		window::on_remove();
	}

	/*
	void Node::tell_connections()
	{
		if (g && g->listener)
		{
			for (auto o: output_parametres)
			{
			   for (auto i : o->output_connections)
				{
				   g->listener->on_link(this, o.get(), i->to->owner, i->to.get());
				}
			}
		}
	}
	*/

	void window::remove()
	{
		owner->remove_node(get_ptr());
	}

	void window::on_remove()
	{
		for (auto listener : listeners)
			listener->on_remove(this);

		owner = nullptr;
	}
	void Node::on_done(GraphContext*)
	{

		for (auto c : output_parametres)
		{
			if (!c->immediate_send_next)
				c->send_next();
		}
	}
	void Node::on_start(GraphContext*)
	{

	}
	/*
	input::ptr Node::register_input( std::string name)
	{
		for (auto i : input_parametres)
		{
			if (i->name == name)
				return i;
		}

		input::ptr i(new input(this));
		i->name = name;
	//	i->type = type;
		input_parametres.push_back(i);

		for (auto listener : listeners)
			listener->on_add_input(i.get());

		return i;
	}*/

//	output::ptr Node::register_output( std::string name /*= "unnamed parameter"*/)
	

	void window::on_tell(graph_listener* listener)
	{
		listener->on_register(this);
	}
	void Node::on_tell(graph_listener* listener)
	{
		window::on_tell(listener);

		for (auto& i : input_parametres)
			listener->on_add_input(i.get());

		for (auto& p : output_parametres)
			listener->on_add_output(p.get());
	}

	void Node::remove_input(input::ptr i)
	{
		i->unlink_input();
		i->unlink_output();
		input_parametres.erase(std::remove(input_parametres.begin(), input_parametres.end(), i), input_parametres.end());

		for (auto listener : listeners)
			listener->on_remove_input(i.get());
	}

	void Node::remove_output(output::ptr o)
	{
		o->unlink_input();
		o->unlink_output();
		output_parametres.erase(std::remove(output_parametres.begin(), output_parametres.end(), o), output_parametres.end());

		for (auto listener : listeners)
			listener->on_remove_output(o.get());
	}

	input::ptr Node::get_input(int i)
	{
		return input_parametres[i];
	}

	output::ptr Node::get_output(int i)
	{
		return output_parametres[i];
	}

	/*
	void Node::on_link(parameter* o, parameter* i)
	{
		if (g&&g->listener)
		{
			g->listener->on_link(this, o, i->owner, i);
		}
	}

	void Node::on_unlink(parameter* o, parameter* i)
	{
		if (g&&g->listener)
		{
			g->listener->on_unlink(this, o, i->owner, i);
		}
	}*/
	void graph::operator()(GraphContext* graph)
	{
		context = graph;

		for (auto& n : nodes)
			n->test_start();

		for (auto i : input_parametres)
			i->send_next();
	}

	void graph::on_output(output * i)
	{
		output_count++;

		if (output_count == output_parametres.size())
			//  if (output_count == connected_outputs)
			on_finish();
	}

	void graph::on_finish()
	{
	}

	void graph::add_task(Node * node)
	{
		context->add_task(node);
	}

	std::list<Node::ptr>  graph::on_drop(MyVariant value)
	{

		return {};
	}

	void graph::add_listener(graph_listener* listener, bool tell_all)
	{
		for (auto& n : nodes)
			n->add_listener(listener, tell_all);

		for (auto& n : windows)
			n->add_listener(listener, tell_all);

		Node::add_listener(listener, tell_all);

		for (auto& n : connections)
			n->add_listener(listener, tell_all);
	}

	bool parameter::link(parameter::ptr i)
	{
		if (!i->can_link(this))
			return false;

		connection::ptr c(new connection());

		auto ptr = get_ptr<parameter>();
		c->registrate(owner->get_graph(), ptr, i);

		for (auto listener : owner->listeners)
			c->add_listener(listener);

		/*inputs.insert(i);

		owner->on_link(this, i.get());
		i->input_elements.insert(this);*/
		return true;
	}

	bool parameter::unlink(parameter::ptr i)
	{
		auto t = get_ptr<parameter>();
		auto it = std::find_if(output_connections.begin(), output_connections.end(), [t, i](const connection::ptr & c)
		{
			return c->from == t && c->to == i;
		});

		if (it != output_connections.end())
			(*it)->unlink();

		/*
		i->input_elements.erase(this);
		inputs.erase(it);

		owner->on_unlink(this, i.get());
		*/
		return true;
	}

	output::~output()
	{
		//	unlink_all();
	}
	/*
	void output::unlink_all()
	{
		for (auto i : inputs)
		{
			i->input_elements.erase(this);
			owner->on_unlink(this, i.get());
		}
	}
	*/
	void output::on_put()
	{
		parameter::on_put();
		/*for (auto i : output_connections)
		{
			i->pass(value);
		}
		*/
		owner->on_output(this);
	}

	void output::remove()
	{
		owner->remove_output(get_ptr<output>());
	}

	/*
	void input::unlink_all()
	{
		auto t = get_ptr<input>();
		auto t_copy = input_elements;
		for (auto i : t_copy)
		{
			i->unlink(t);
		}
	}*/
	/*
	bool input::can_link(parameter* o)
	{


		if (o->owner == owner)
			return false;

		if (!owner->can_link(type,o->type))
			return false;

		if (!!dynamic_cast<output*>(o) && (owner->get_graph() == dynamic_cast<graph*>(o->owner)))
		{
			return false;
		}
		if (!!dynamic_cast<graph_input*>(o) && (owner->get_graph() != dynamic_cast<graph*>(o->owner)))
		{
			return false;
		}

		if (only_one_input)
			return input_connections.empty();

		return true;
	}
	*/
	void input::on_put()
	{
		parameter::on_put();
		owner->on_input(this);
	}

	void input::remove()
	{
		owner->remove_input(get_ptr<input>());
	}

	bool graph_input::link(parameter::ptr i)
	{
		if (!i->can_link(this))
			return false;

		connection::ptr c(new connection());

		auto ptr = get_ptr<parameter>();
		c->registrate(dynamic_cast<graph*>(owner), ptr, i);

		for (auto listener : owner->get_listeners())
			c->add_listener(listener);

		/*inputs.insert(i);

		owner->on_link(this, i.get());
		i->input_elements.insert(this);*/
		return true;
	}

	bool graph_input::can_link(parameter* o)
	{
		if (!parameter::can_link(o))
			return false;

		output* p = dynamic_cast<output*>(o);

		if (p)
		{
			if (p->owner->get_graph() == dynamic_cast<graph*>(owner))
				return false;

			if (owner->get_graph() == dynamic_cast<graph*>(o->owner))
				return false;
		}

		return true;
	}


	void parameter::unlink_input()
	{
		std::set< s_ptr<connection>>  i_c = input_connections;

		for (auto c : i_c)
			c->unlink();
	}

	void parameter::unlink_output()
	{
		std::set< s_ptr<connection>>  o_c = output_connections;

		for (auto c : o_c)
			c->unlink();
	}

	void parameter::set_enabled(bool value, bool force)
	{
		//   if ( force)
		if (!value)
		{
			enabled = false;

			for (auto& o : output_connections)
				if (o->enabled)
				{
					enabled = true;
					break;
				}

			//	if()
			//enabled = value;
		}

		else
			enabled = value;

		for (auto& i : input_connections)
			i->set_enabled(value || enabled);
	}

	void parameter::on_put()
	{
		if (immediate_send_next)
			send_next();
	}

	void parameter::send_next()
	{
		if (value.exists())
			for (auto c : output_connections)
				c->pass(value);


		if(can_output)
		value = nullptr;
	}

	bool parameter::can_link(parameter* p)
	{
		if (!can_input) return false;

		if (!p->can_output) return false;

		if (p->owner == owner)
			return false;

		if (!type->can_cast(p->type.get()))
			return false;

		if (only_one_input)
			return input_connections.empty();

		return true;
	}

	void connection::registrate(graph* g, parameter::ptr& from, parameter::ptr& to)
	{
		//bool res = from->owner->owner == to->owner->owner;
		//if (!res)
		//	return;
		this->from = from;
		this->to = to;
		auto t = get_ptr();
		from->output_connections.insert(t);
		to->input_connections.insert(t);
		g->connections.insert(get_ptr());
		this->g = g;
		tell();
	}

	void connection::registrate(graph* g)
	{
		auto t = get_ptr();
		from->output_connections.insert(t);
		to->input_connections.insert(t);
		this->g = g;
		//	g->connections.insert(get_ptr());
		tell();
	}

	void connection::on_tell(graph_listener* listener)
	{
		listener->on_link(from.get(), to.get());
	}

	void connection::set_enabled(bool value)
	{
		//  if (enabled == value)
		//     return;
		enabled = value;
		from->set_enabled(enabled, false);
		//to->set_enabled(enabled);
	}

	void connection::unlink()
	{
		auto t = get_ptr();
		from->output_connections.erase(t);
		to->input_connections.erase(t);

		for (auto listener : listeners)
			listener->on_unlink(from.get(), to.get());

		if (g)
			g->connections.erase(get_ptr());
	}

	bool graph_output::can_link(parameter* o)
	{
		if (!parameter::can_link(o))
			return false;

		output* p = dynamic_cast<output*>(o);

		if (!p) return false;

		return p->owner->get_graph() == dynamic_cast<graph*>(owner);// || !!dynamic_cast<graph_input*>(o);
	}

	graph_listener::~graph_listener()
	{
		auto c = owners;

		for (auto o : c)
			o->remove_listener(this);
	}

	//const data_types data_types::INT("int");
	//const data_types data_types::STRING("string");

	void GraphContext::run(Node * node)
	{
		(*node)(this);
	}

	graph* GraphContext::create_graph()
	{
		return new graph();
	}

	void GraphContext::add_task(Node * node)
	{
		run(node);
	}
/*
	data_types::data_types(std::string name)
	{
		this->name = name;
	}

	bool data_types::can_link(data_types * r)
	{
		return name == r->name;
	}

	std::string data_types::to_string() const
	{
		return name;
	}*/

	void graph::auto_layout()
	{
		/*	struct link
			{
				Node* from;
				Node* to;
			};*/

		std::map<Node*, vec2> forces;
	//	for (int i = 0; i < 100; i++)
		{
			forces.clear();

			const float speed = 1;
			float scale = 5;


			vec2 in_force;
			vec2 out_force;
			for (auto&c : connections)
			{

				auto from = c->from->owner;
				auto to = c->to->owner;

				if (from == to)  continue;


				vec2 &from_pos = from == this? pos_in : from->pos;
				vec2 &to_pos = to == this ? pos_out : to->pos;

				vec2 delta = from_pos - to_pos + vec2(rand(), rand()) / RAND_MAX;

				float l = delta.length();

				vec2 force = { 0,0 };

			/*	if (l > 300)
				{

					force=-10*delta.normalize(10);
				}
				if (l < 100)
				{
					force =10 * delta.normalize(10);

				}
		*/
				scale = (float)Math::clamp(to->input_parametres.size() + from->output_parametres.size(),1_t,5_t);
			
				if (from_pos.x >to_pos.x - scale*150)
				{
					float d = from_pos.x - (to_pos.x - scale * 150);

					
					force += Math::clamp(-d/10, -5.0f,5.0f);


				}
				

				if(l <  scale * 100)
				{
					force += delta.normalize(1);// *(scale * 100 - l) / (scale * 100.0f);

				}else if (l >  scale * 150)
				{
					force += -delta.normalize(1);// *(l - scale * 200) / (scale * 1000.0f);
				}
				
				if (from == this) in_force += force;
				else	forces[from] += force;


				if (to == this) out_force-= force;
				else forces[to] -= force;
			}
			/*
			for (auto &a : forces)
				for (auto &b : forces)
				{
					if (a.first == b.first) continue;

					vec2 &a_pos = a.first == this ? pos_in : a.first->pos;
					vec2 &b_pos = b.first == this ? pos_out : b.first->pos;


					vec2 delta = a_pos - b_pos + vec2(rand(), rand()) / RAND_MAX;

					float l = delta.length();


					if (l <  scale * 150)
					{
						delta=	delta.normalize(1)*(scale * 150-l)/(scale*150.0f);
						a.second += delta;
						b.second -= delta;
					}

				}
			*/
			for (auto &d : forces)
			{
				
				d.first->pos += speed /(d.first->input_parametres.size() + d.first->output_parametres.size()) * d.second;
			}

			pos_in += speed*in_force;
			pos_out += speed*out_force;


		}
	
	}
	/*

	void graph::auto_layout()
	{
		std::map<Node*, int> depths;


		std::function<void(Node*)> process_node;

		int max_depth = 0;
		process_node = [&process_node, &depths,&max_depth](Node* node) {

			int my_depth = depths[node];

			max_depth = std::max(max_depth, my_depth);
			for (auto &p : node->output_parametres)
			{
				for (auto &c : p->output_connections)
				{
					auto &&other = depths[c->to->owner];


					other = std::max(other, my_depth + 1);
					process_node(c->to->owner);
				}


			}


		};

	auto 	process_graph= [&process_node, &depths](Node* node) {

			int my_depth = depths[node];


			for (auto &p : node->input_parametres)
			{
				for (auto &c : p->output_connections)
				{
					auto &&other = depths[c->to->owner];
					if (other < my_depth + 1)
					{
						other = my_depth + 1;
						process_node(c->to->owner);

					}
					
				}


			}


		};



		depths[this] = 0;
		process_graph(this);

		for (auto&n : nodes)
		{



			if (!n->has_inputs())
			{
				depths[n.get()] = 1;
				process_node(n.get());

			}


		}




		std::vector<int> node_map(max_depth,0);

		for (auto &d : depths)
		{
			node_map[d.second]++;
	
		}

		for (auto &d : depths)
		{
			d.first->pos = { d.second * 200,100*(node_map[d.second]--) };
		}
	
	}*/
}



BOOST_CLASS_EXPORT(FlowGraph::window);

BOOST_CLASS_EXPORT(FlowGraph::Node);
BOOST_CLASS_EXPORT(FlowGraph::input);
BOOST_CLASS_EXPORT(FlowGraph::output);

BOOST_CLASS_EXPORT(FlowGraph::graph);

BOOST_CLASS_EXPORT(FlowGraph::graph_input);
BOOST_CLASS_EXPORT(FlowGraph::graph_output);

BOOST_CLASS_EXPORT(FlowGraph::parameter_type);


BOOST_CLASS_EXPORT(FlowGraph::strict_parameter);

