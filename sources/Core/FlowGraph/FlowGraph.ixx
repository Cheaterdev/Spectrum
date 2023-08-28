export module Core:FlowGraph;
import <Core_defs.h>;

import :Data;
import :crc32;
import :Utils;
import :serialization;
import :Math;

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
				SAVE_PARENT(parameter_type);

			}



		};
		// concept to detect any derivered class from Resource
		template <class T>
		concept class_parameter_type = std::is_base_of<parameter_type, T>::value;

		struct parameter : public SharedObject<parameter>
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

			using ptr = std::shared_ptr<parameter>;

			virtual ~parameter() {}


			void set_enabled(bool value, bool force = true);


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

			std::set< std::shared_ptr<connection>> input_connections;
			std::set< std::shared_ptr<connection>> output_connections;
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


		class connection : public SharedObject<connection>, public listenable
		{
			graph* g = nullptr;
		public:
			parameter::ptr from;
			parameter::ptr to;
			bool enabled = true;
			using ptr = std::shared_ptr<connection>;

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
			using ptr = std::shared_ptr<input>;
			virtual ~input() = default;
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
				SAVE_PARENT(parameter);

			}



		};


		class output : public parameter
		{

		public:

			using ptr = std::shared_ptr<output>;
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
				SAVE_PARENT(parameter);

			}
		};


		class window : public SharedObject<window>, public listenable
		{

		public:
			using ptr = std::shared_ptr<window>;
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
			using ptr = std::shared_ptr<Node>;

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

		
		private:
			SERIALIZE()
			{
				SAVE_PARENT(window);

				ar& NVP(input_parametres);
				ar& NVP(output_parametres);

				for (auto n : input_parametres)
					n->owner = this;

				for (auto n : output_parametres)
					n->owner = this;

				IF_LOAD()
				{
					//	(*this).Node();
				}
			}

			virtual void on_tell(graph_listener* listener) override;

		};


		class graph_input : public input
		{
		public:
			using ptr = std::shared_ptr<graph_input>;
			bool link(parameter::ptr i) override;
			graph_input() { immediate_send_next = false; can_output = true; };
			graph_input(Node* n) : input(n) { immediate_send_next = false; can_output = true; }


			bool can_link(parameter* o);



		private:
			SERIALIZE()
			{
				SAVE_PARENT(input);

			}

		};


		class graph_output : public output
		{

		public:
			using ptr = std::shared_ptr<graph_output>;

			graph_output() { immediate_send_next = false; can_input = true; };
			graph_output(Node* n) : output(n) { immediate_send_next = false; can_input = true; }

			virtual bool can_link(parameter* o);
		private:
			SERIALIZE()
			{
				SAVE_PARENT(output);

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
			using ptr = std::shared_ptr<graph>;

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
				SAVE_PARENT(Node);
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
				SAVE_PARENT(Node);

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


REGISTER_TYPE(FlowGraph::window);
REGISTER_TYPE(FlowGraph::Node);
REGISTER_TYPE(FlowGraph::input);
REGISTER_TYPE(FlowGraph::output);
REGISTER_TYPE(FlowGraph::graph);
REGISTER_TYPE(FlowGraph::graph_input);
REGISTER_TYPE(FlowGraph::graph_output);
REGISTER_TYPE(FlowGraph::parameter_type);
REGISTER_TYPE(FlowGraph::strict_parameter);
