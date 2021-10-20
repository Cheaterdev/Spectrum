module;
#include "Serialization/serialization_defines.h"
export module FlowGraph;


import Data;

import crc32;
import Utils;
import serialization;

import Constants;
import Vectors;
export
{

	namespace FlowGraph
	{
	

		struct parameter;

		class input;
		class output;
		class graph;
		class Node;
		class window;

		class connection;

		class listenable;
		class graph_listener
		{
			friend class listenable;
			std::set<listenable*> owners;
		protected:
			virtual ~graph_listener();

		public:
			virtual	void on_register(window*) = 0;
			virtual	void on_remove(window*) = 0;

			virtual	void on_add_input(parameter*) = 0;
			virtual	void on_remove_input(parameter*) = 0;

			virtual	void on_add_output(parameter*) = 0;
			virtual	void on_remove_output(parameter*) = 0;

			virtual	void on_link(parameter*, parameter*) = 0;
			virtual	void on_unlink(parameter*, parameter*) = 0;
			//virtual	void on_value_change(std::string name,) = 0;
		};

		class listenable
		{


		protected:
			std::set<graph_listener*> listeners;
			virtual void on_tell(graph_listener* listener) = 0;
			void tell()
			{
				for (auto listener : listeners)
					on_tell(listener);
			}

		public:
			std::set<graph_listener*> get_listeners()
			{
				return listeners;
			}
			virtual void add_listener(graph_listener* listener, bool send_all = true)
			{
				listeners.insert(listener);
				listener->owners.insert(this);

				if (listener && send_all)
					on_tell(listener);
			}

			virtual void remove_listener(graph_listener* listener)
			{
				listeners.erase(listener);
				listener->owners.erase(this);
			}
			virtual ~listenable()
			{
				auto c = listeners;

				for (auto listener : c)
					remove_listener(listener);
			}

		};


		//template<class T>
		struct parameter_type
		{
			virtual bool can_cast(parameter_type* other) = 0;
			virtual ~parameter_type() = default;
		private:
			SERIALIZE()
			{

			}


		};

		struct strict_parameter :public parameter_type
		{


			virtual bool can_cast(parameter_type* other) override
			{
				return true;
			}
			virtual ~strict_parameter() = default;
		private:
			SERIALIZE()
			{
				ar& NVP(boost::serialization::base_object<parameter_type>(*this));

			}



		};
		// concept to detect any derivered class from Resource
		template <class T>
		concept class_parameter_type = std::is_base_of<parameter_type, T>::value;

		struct parameter : public std::enable_shared_from_this<parameter>
		{
			// LEAK_TEST
			MyVariant value;
			MyVariant default_value;

			std::unique_ptr<parameter_type> type;
			std::string name;
			bool only_one_input = true;
			bool can_input = false;
			bool can_output = false;
			bool enabled = true;
			void shutdown()
			{
				input_connections.clear();
				output_connections.clear();
			}

			void clear()
			{
				value.clear();
			}

			using ptr = s_ptr<parameter>;

			virtual ~parameter() {}


			void set_enabled(bool value, bool force = true);

			template <class M = parameter>
			std::shared_ptr<M> get_ptr()
			{
				return std::static_pointer_cast<M>(shared_from_this());
			}


			virtual void on_put();

			template<class T>
			void put(T value)
			{
				if (this->value.exists())
					return;

				this->value = value;
				on_put();
			}

			template<class T>
			void reset(T value)
			{
				this->value = value;
			}

			template<class T>
			T get()
			{
				if (value.exists())
					return value.get<T>();

				return default_value.get<T>();
			}

			bool has_value()
			{
				return value.exists();
			}
			Node* owner;

			std::set< s_ptr<connection>> input_connections;
			std::set< s_ptr<connection>> output_connections;
			virtual bool link(parameter::ptr i);
			virtual bool unlink(parameter::ptr i);

			virtual bool can_link(parameter*);

			virtual void remove()
			{
			}
			bool has_input()
			{
				return !input_connections.empty();
			}


		public:
			bool immediate_send_next = true;
			virtual void send_next();

		private:
			friend class graph;
			friend class output;
			friend class input;
			friend class connection;
			friend class Node;

			//        virtual bool unlink(parameter::ptr i) { return false; }

			void unlink_input();
			void unlink_output();

			SERIALIZE()
			{
				ar& NVP(type);
				ar& NVP(name);
				ar& NVP(only_one_input);
				ar& NVP(can_input);
				ar& NVP(can_output);
			}

		};


		class connection : public std::enable_shared_from_this<connection>, public listenable
		{
			graph* g = nullptr;
		public:
			parameter::ptr from;
			parameter::ptr to;
			bool enabled = true;
			using ptr = s_ptr<connection>;

			template <class M = connection>
			std::shared_ptr<M> get_ptr()
			{
				return std::static_pointer_cast<M>(shared_from_this());
			}

			void registrate(graph* g, parameter::ptr& from, parameter::ptr& to);
			void registrate(graph* g);

			void pass(MyVariant value)
			{
				if (enabled)
					to->put(value);
			}

			void set_enabled(bool value);
			void unlink();

			virtual void on_tell(graph_listener* listener) override;
			~connection()
			{
				from = nullptr;
				to = nullptr;
			}

		private:
			SERIALIZE()
			{
				ar& NVP(from);
				ar& NVP(to);
				/*
							if (Archive::is_loading::value)
							{
								auto t = std::shared_ptr<connection>(this);
								from->output_connections.insert(t);
								to->input_connections.insert(t);
							}*/
			}

		};

		class input : public parameter
		{
		public:
			using ptr = s_ptr<input>;
		private:
			friend class graph;
			friend class output;
			friend class Node;
		protected:


			input(Node* owner)
			{
				this->owner = owner;
				can_input = true;
			}
			input() {};

			//   virtual bool can_link(parameter* o);
			virtual void remove();

		public:

			virtual void on_put();
			//
		private:
			SERIALIZE()
			{
				ar& NVP(boost::serialization::base_object<parameter>(*this));
			}



		};


		class output : public parameter
		{

		public:

			using ptr = s_ptr<output>;
		private:
			friend class graph;
			friend class Node;
		protected:

			output(Node* owner)
			{
				this->owner = owner;
				can_output = true;
			}
			output()
			{
			}

		public:

			virtual ~output();
			virtual void remove();
			virtual void on_put();
		private:
			SERIALIZE()
			{
				ar& NVP(boost::serialization::base_object<parameter>(*this));
			}
		};


		class window : public std::enable_shared_from_this<window>, public listenable
		{

		public:
			using ptr = s_ptr<window>;
			graph* owner = nullptr;

			std::string name;
			vec2 pos = vec2(10, 10);
			vec2 size = vec2(10, 10);


			graph* get_graph()
			{
				return owner;
			}
			virtual~window() {};
			virtual void on_add(graph* g);
			virtual void on_remove();

			void remove();
			template <class M = window>
			std::shared_ptr<M> get_ptr()
			{
				auto res = std::dynamic_pointer_cast<M>(shared_from_this());
				return res;
			}

			virtual void on_tell(graph_listener* listener) override;

		private:
			SERIALIZE()
			{
				ar& NVP(name);
				ar& NVP(pos);
				ar& NVP(size);
			}
		};


		class Node;
		class GraphContext;



		class Node : public window
		{
			friend class graph;
			friend class input;
			friend class output;
			friend class connection;
			friend struct parameter;
			friend class  GraphContext;

			/// input

			int setted_params = 0;
			std::mutex m;



		protected:

			virtual   void reset_for();

			bool start_if_output = false;
			void shutdown();


			std::vector<input::ptr> input_parametres;
			std::vector<output::ptr> output_parametres;

			//		virtual bool can_link(data_types a, data_types b);
			virtual bool test_start();

			virtual void on_input(input* i);

			virtual void on_output(output* i);

			virtual void on_remove();


		public:
			virtual unsigned int get_id()
			{
				return "undefined"_crc32;
			}

			unsigned int get_graph_id()
			{
				return "undefined"_crc32;
			}

			virtual unsigned int get_id_with_links()
			{

				std::stringstream total;

				total << "IN:";
				for (auto p : input_parametres)
				{
					if (p->has_input())
						total << p->owner->get_id() << "_";
				}
				total << "OUT:";
				for (auto p : output_parametres)
				{
					//if (p->())
					total << p->owner->get_id() << "_";
				}
				return "undefined"_crc32;
			}

			virtual void on_done(GraphContext*);
			virtual void on_start(GraphContext*);
			using ptr = s_ptr<Node>;

			template<class T = strict_parameter>
			input::ptr register_input(std::string name = "unnamed parameter", const T& param = T())
			{
				{
					for (auto i : input_parametres)
					{
						if (i->name == name)
							return i;
					}

					input::ptr i(new input(this));
					i->name = name;
					i->type = std::make_unique<T>(param);
					input_parametres.push_back(i);

					for (auto listener : listeners)
						listener->on_add_input(i.get());

					return i;
				}
			}

			template<class T = strict_parameter>
			output::ptr register_output(std::string name = "unnamed parameter", const T& param = T())
			{
				/* for (auto i : output_parametres)
				 {
					 if (i->name == name)
						 return i;
				 }*/
				output::ptr p(new output(this));
				p->type = std::make_unique<T>(param);
				p->name = name;
				output_parametres.push_back(p);

				for (auto listener : listeners)
					listener->on_add_output(p.get());

				return p;
			}
			virtual  void remove_input(input::ptr);
			virtual   void remove_output(output::ptr);

			input::ptr get_input(int i);
			output::ptr get_output(int i);

			bool has_inputs()
			{
				return !input_parametres.empty();
			}

			bool has_outputs()
			{
				return !output_parametres.empty();
			}
		protected:
			virtual  void operator()(GraphContext*) = 0;

		public:
			Node() = default;

			virtual ~Node();

			template <class M = Node>
			std::shared_ptr<M> get_ptr()
			{
				return std::static_pointer_cast<M>(shared_from_this());
			}
		private:
			SERIALIZE()
			{
				ar& NVP(boost::serialization::base_object<window>(*this));
				ar& NVP(input_parametres);
				ar& NVP(output_parametres);

				for (auto n : input_parametres)
					n->owner = this;

				for (auto n : output_parametres)
					n->owner = this;

				if (Archive::is_loading::value)
				{
					//	(*this).Node();
				}
			}

			virtual void on_tell(graph_listener* listener) override;

		};


		class graph_input : public input
		{
		public:
			using ptr = s_ptr<graph_input>;
			bool link(parameter::ptr i) override;
			graph_input() { immediate_send_next = false; can_output = true; };
			graph_input(Node* n) : input(n) { immediate_send_next = false; can_output = true; }


			bool can_link(parameter* o);



		private:
			SERIALIZE()
			{
				ar& NVP(boost::serialization::base_object<input>(*this));
			}

		};


		class graph_output : public output
		{

		public:
			using ptr = s_ptr<graph_output>;

			graph_output() { immediate_send_next = false; can_input = true; };
			graph_output(Node* n) : output(n) { immediate_send_next = false; can_input = true; }

			virtual bool can_link(parameter* o);
		private:
			SERIALIZE()
			{
				ar& NVP(boost::serialization::base_object<output>(*this));
			}

		};


		class GraphContext
		{
		protected:
			virtual  ~GraphContext() = default;

			void run(Node* node);


		public:
			virtual graph* create_graph();
			virtual void add_task(Node* node);


		};
		class graph : public Node
		{
			friend class Node;
			friend class connection;
		protected:

			std::set<std::shared_ptr<Node>> nodes;
			std::set<std::shared_ptr<window>> windows;

			std::set<connection::ptr> connections;

			size_t output_count;
			size_t connected_outputs;

			virtual  void operator()(GraphContext*) override;
			virtual void on_output(output* i) override;

			virtual void on_finish();

			virtual void add_task(Node* node);
		public:
			virtual std::list<Node::ptr> on_drop(MyVariant value);
			GraphContext* context = nullptr;

			bool start_child_nodes = false;
			vec2 pos_in = vec2(0, 0);
			vec2 pos_out = vec2(200, 0);

			vec2 cam_pos = vec2(0, 0);
			using ptr = s_ptr<graph>;

			void register_node(std::shared_ptr<window> node);
			void remove_node(std::shared_ptr<window> node);

			void clear();
			//	input::ptr register_input(std::string name = "unnamed parameter");
			//	output::ptr register_output(std::string name = "unnamed parameter");

			template<class_parameter_type T = strict_parameter>   input::ptr register_input(std::string name, const T& param = T())
			{
				/*   for (auto i : input_parametres)
				{
				if (i->name == name)
				return i;
				}
				*/
				graph_input::ptr i(new graph_input(this));
				i->name = name;
				i->type = std::make_unique<T>(param);
				input_parametres.push_back(i);

				for (auto listener : listeners)
					listener->on_add_input(i.get());

				return i;
			}

			template<class_parameter_type T = strict_parameter> output::ptr register_output(std::string name, const T& param = T())
			{
				/*    for (auto i : output_parametres)
				{
				if (i->name == name)
				return i;
				}
				*/
				output::ptr p(new graph_output(this));
				p->type = std::make_unique<T>(param);
				p->name = name;
				output_parametres.push_back(p);

				for (auto listener : listeners)
					listener->on_add_output(p.get());

				return p;
			}

			void auto_layout();
			graph();

			virtual ~graph();
			template <class M = graph>
			std::shared_ptr<M> get_ptr()
			{
				return std::static_pointer_cast<M>(shared_from_this());
			}

			/*
			size_t calculate_unique_id()
			{
				std::map<Node*, int> nodes_ids;

				std::vector<Node*> node_vec;
				for (auto& node : nodes)
				{
					node_vec.push_back(node.get());
				}

				std::sort(node_vec.begin(), node_vec.end(), [](Node* a, Node* b) {
					if (a->get_id() == b->get_id())
					{
						return a->get_id_with_links() < b->get_id_with_links();
					}

					return (a->get_id() < b->get_id());
					});


				for (int i = 0; i < node_vec.size(); i++)
				{
					nodes_ids[node_vec[i]] = i;
				}



			}*/

			virtual	void reset_for();
			virtual void start(GraphContext* context);

			virtual void add_listener(graph_listener* listener, bool) override;

			/*
					void on_tell(graph_listener* listener) override
					{
					//	listener->on_register(this);

						for (auto &i : input_parametres)
							listener->on_add_input(this, i.get());

						for (auto& p : output_parametres)
							listener->on_add_output(this, p.get());

					}*/
		private:
			SERIALIZE()
			{
				ar& NVP(boost::serialization::base_object<Node>(*this));
				ar& NVP(nodes);
				ar& NVP(cam_pos);
				ar& NVP(windows);
				ar& NVP(pos_in);
				ar& NVP(pos_out);

				for (auto n : nodes)
					n->owner = this;

				for (auto n : windows)
					n->owner = this;

				ar& NVP(connections);

				if (Archive::is_loading::value)
				{
					for (auto& c : connections)
						c->registrate(this);
				}
			}


		};




		template<class T>
		class GraphNode :public Node
		{
			virtual  void operator()(GraphContext* type) override
			{
				(*this)(static_cast<T*>(type));
			}
			virtual void on_done(GraphContext* type) override { on_done(static_cast<T*>(type)); };
			virtual void on_start(GraphContext* type) override { on_start(static_cast<T*>(type)); };
		protected:
			virtual  void operator()(T* type) = 0;
			virtual void on_done(T* type) { Node::on_done(type); };
			virtual void on_start(T* type) { Node::on_start(type); };
		private:
			SERIALIZE()
			{
				ar& NVP(boost::serialization::base_object<Node>(*this));
			}

		};

		template <class C>
		struct ContextOptions
		{
			using ContextType = C;
			using GraphType = graph;

			using NodeType = GraphNode<ContextType>;
		};


		using ContextDefault = ContextOptions<GraphContext>;


	}


}

module:private;

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

		for (auto node : nodes)
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

	void graph::start(GraphContext* context)
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
					if (!n->has_inputs())
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
		for (auto&& e : input_parametres)
			e->shutdown();

		for (auto&& e : output_parametres)
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

	void Node::on_input(input* i)
	{
		setted_params++;
		test_start();
	}

	void Node::on_output(output* i)
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

	void graph::on_output(output* i)
	{
		output_count++;

		if (output_count == output_parametres.size())
			//  if (output_count == connected_outputs)
			on_finish();
	}

	void graph::on_finish()
	{
	}

	void graph::add_task(Node* node)
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
		auto it = std::find_if(output_connections.begin(), output_connections.end(), [t, i](const connection::ptr& c)
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


		if (can_output)
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

	void GraphContext::run(Node* node)
	{
		(*node)(this);
	}

	graph* GraphContext::create_graph()
	{
		return new graph();
	}

	void GraphContext::add_task(Node* node)
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
			for (auto& c : connections)
			{

				auto from = c->from->owner;
				auto to = c->to->owner;

				if (from == to)  continue;


				vec2& from_pos = from == this ? pos_in : from->pos;
				vec2& to_pos = to == this ? pos_out : to->pos;

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
				scale = (float)Math::clamp(to->input_parametres.size() + from->output_parametres.size(), 1_t, 5_t);

				if (from_pos.x > to_pos.x - scale * 150)
				{
					float d = from_pos.x - (to_pos.x - scale * 150);


					force += vec2(Math::clamp(-d / 10, -5.0f, 5.0f));


				}


				if (l < scale * 100)
				{
					force += delta.normalize(1);// *(scale * 100 - l) / (scale * 100.0f);

				}
				else if (l > scale * 150)
				{
					force += -delta.normalize(1);// *(l - scale * 200) / (scale * 1000.0f);
				}

				if (from == this) in_force += force;
				else	forces[from] += force;


				if (to == this) out_force -= force;
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
			for (auto& d : forces)
			{

				d.first->pos += speed / (d.first->input_parametres.size() + d.first->output_parametres.size()) * d.second;
			}

			pos_in += speed * in_force;
			pos_out += speed * out_force;


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

