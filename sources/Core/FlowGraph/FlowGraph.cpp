module Core:FlowGraph;

import :Data;
import :crc32;
import :Utils;
import :serialization;
import :Math;

namespace FlowGraph
{
	void graph::remove_node(std::shared_ptr<window> node)
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
			window->on_remove();

		for (auto node : nodes)
			node->on_remove();

		nodes.clear();
		windows.clear();
	}

	graph::graph()
	{
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
			on_finish();
	}

	void graph::on_finish()
	{
	}

	void graph::add_task(Node* node)
	{
		context->add_task(node);
	}

	std::list<Node::ptr> graph::on_drop(MyVariant value)
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

		return true;
	}

	output::~output()
	{
	}

	void output::on_put()
	{
		parameter::on_put();
		owner->on_output(this);
	}

	void output::remove()
	{
		owner->remove_output(get_ptr<output>());
	}

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
		std::set<std::shared_ptr<connection>> i_c = input_connections;

		for (auto c : i_c)
			c->unlink();
	}

	void parameter::unlink_output()
	{
		std::set<std::shared_ptr<connection>> o_c = output_connections;

		for (auto c : o_c)
			c->unlink();
	}

	void parameter::set_enabled(bool value, bool force)
	{
		if (!value)
		{
			enabled = false;

			for (auto& o : output_connections)
				if (o->enabled)
				{
					enabled = true;
					break;
				}
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
		tell();
	}

	void connection::on_tell(graph_listener* listener)
	{
		listener->on_link(from.get(), to.get());
	}

	void connection::set_enabled(bool value)
	{
		enabled = value;
		from->set_enabled(enabled, false);
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

		return p->owner->get_graph() == dynamic_cast<graph*>(owner);
	}

	graph_listener::~graph_listener()
	{
		auto c = owners;

		for (auto o : c)
			o->remove_listener(this);
	}

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

	void graph::auto_layout()
	{
		// ── 1. Index nodes ────────────────────────────────────────────────────
		std::vector<Node*> verts;
		std::map<Node*, int> idx;
		for (auto& n : nodes)
		{
			idx[n.get()] = (int)verts.size();
			verts.push_back(n.get());
		}
		const int N = (int)verts.size();
		if (N == 0) return;

		// ── 2. Build adjacency lists ──────────────────────────────────────────
		std::vector<std::vector<int>> succ(N), pred(N);
		struct Edge { int from, to; bool back = false; connection::ptr conn; };
		std::vector<Edge> edges;

		for (int u = 0; u < N; ++u)
			for (auto& p : verts[u]->output_parametres)
				for (auto& c : p->output_connections)
				{
					auto it = idx.find(c->to->owner);
					if (it == idx.end()) continue;
					int v = it->second;
					edges.push_back({u, v, false, c});
					succ[u].push_back((int)edges.size() - 1);
					pred[v].push_back((int)edges.size() - 1);
				}

		// ── 3. Back-edge detection (DFS) ─────────────────────────────────────
		enum class S : uint8_t { UNVISITED, ON_STACK, DONE };
		std::vector<S> state(N, S::UNVISITED);
		std::function<void(int)> dfs = [&](int u)
		{
			state[u] = S::ON_STACK;
			for (int ei : succ[u])
			{
				auto& e = edges[ei];
				if      (state[e.to] == S::ON_STACK) e.back = true;
				else if (state[e.to] == S::UNVISITED) dfs(e.to);
			}
			state[u] = S::DONE;
		};
		for (int i = 0; i < N; ++i)
			if (state[i] == S::UNVISITED) dfs(i);

		// ── 4. Layer assignment (Kahn BFS, longest path) ──────────────────────
		std::vector<int> layer(N, 0), in_deg(N, 0);
		for (auto& e : edges)
			if (!e.back) in_deg[e.to]++;

		std::queue<int> q;
		for (int i = 0; i < N; ++i)
			if (in_deg[i] == 0) q.push(i);

		while (!q.empty())
		{
			int u = q.front(); q.pop();
			for (int ei : succ[u])
			{
				auto& e = edges[ei];
				if (e.back) continue;
				layer[e.to] = std::max(layer[e.to], layer[u] + 1);
				if (--in_deg[e.to] == 0) q.push(e.to);
			}
		}

		const int max_layer = *std::max_element(layer.begin(), layer.end());

		// ── 5. Group by layer, assign initial rank ────────────────────────────
		std::vector<std::vector<int>> layers(max_layer + 1);
		for (int i = 0; i < N; ++i)
			layers[layer[i]].push_back(i);

		std::vector<int> rank(N, 0);
		for (auto& lyr : layers)
			for (int r = 0; r < (int)lyr.size(); ++r)
				rank[lyr[r]] = r;

		// ── 6. Crossing minimisation (2 sweeps, port-aware barycenter) ──────────
		// Port fraction: offset within [0,1) for which port of the neighbour
		// node is connected, so two nodes linking different ports of the same
		// neighbour get different barycenters and can be correctly ordered.
		auto port_frac = [](parameter* p, bool use_inputs) -> float
		{
			auto* owner = p->owner;
			if (use_inputs)
			{
				auto& v = owner->input_parametres;
				int total = (int)v.size();
				if (total <= 1) return 0.0f;
				for (int i = 0; i < total; ++i)
					if (v[i].get() == p) return (float)i / total;
			}
			else
			{
				auto& v = owner->output_parametres;
				int total = (int)v.size();
				if (total <= 1) return 0.0f;
				for (int i = 0; i < total; ++i)
					if (v[i].get() == p) return (float)i / total;
			}
			return 0.0f;
		};

		auto barycenter = [&](int u, int neighbour_layer) -> float
		{
			float sum = 0; int cnt = 0;
			for (int ei : succ[u])
			{
				auto& e = edges[ei];
				if (!e.back && layer[e.to] == neighbour_layer)
				{
					float pf = e.conn ? port_frac(e.conn->to.get(), true) : 0.0f;
					sum += rank[e.to] + pf;
					++cnt;
				}
			}
			for (int ei : pred[u])
			{
				auto& e = edges[ei];
				if (!e.back && layer[e.from] == neighbour_layer)
				{
					float pf = e.conn ? port_frac(e.conn->from.get(), false) : 0.0f;
					sum += rank[e.from] + pf;
					++cnt;
				}
			}
			return cnt ? sum / cnt : (float)rank[u];
		};

		for (int sweep = 0; sweep < 2; ++sweep)
		{
			for (int l = 1; l <= max_layer; ++l)
			{
				std::stable_sort(layers[l].begin(), layers[l].end(),
					[&](int a, int b){ return barycenter(a, l-1) < barycenter(b, l-1); });
				for (int r = 0; r < (int)layers[l].size(); ++r)
					rank[layers[l][r]] = r;
			}
			for (int l = max_layer - 1; l >= 0; --l)
			{
				std::stable_sort(layers[l].begin(), layers[l].end(),
					[&](int a, int b){ return barycenter(a, l+1) < barycenter(b, l+1); });
				for (int r = 0; r < (int)layers[l].size(); ++r)
					rank[layers[l][r]] = r;
			}
		}

		// ── 7. Coordinate assignment ──────────────────────────────────────────
		constexpr float X_SPACING = 350.0f;
		constexpr float Y_SPACING = 150.0f;

		for (int i = 0; i < N; ++i)
			verts[i]->pos = vec2(layer[i] * X_SPACING, rank[i] * Y_SPACING);

		// Y relaxation — nudge each node toward average neighbour y, preserving rank order
		for (int iter = 0; iter < 3; ++iter)
		{
			std::vector<float> new_y(N);
			for (int u = 0; u < N; ++u)
			{
				float sum = 0; int cnt = 0;
				for (int ei : succ[u]) { if (!edges[ei].back) { sum += verts[edges[ei].to  ]->pos.y; ++cnt; } }
				for (int ei : pred[u]) { if (!edges[ei].back) { sum += verts[edges[ei].from]->pos.y; ++cnt; } }
				new_y[u] = cnt ? sum / cnt : verts[u]->pos.y;
			}
			for (auto& lyr : layers)
			{
				for (int r = 0; r < (int)lyr.size(); ++r)
				{
					int u = lyr[r];
					float lo = r > 0                    ? verts[lyr[r-1]]->pos.y + Y_SPACING * 0.5f : -1e9f;
					float hi = r < (int)lyr.size() - 1 ? verts[lyr[r+1]]->pos.y - Y_SPACING * 0.5f :  1e9f;
					verts[u]->pos = vec2(verts[u]->pos.x, std::clamp(new_y[u], lo, hi));
				}
			}
		}

		// Position graph_in: layer -1, vertically centred on the nodes it feeds
		{
			float sum = 0; int cnt = 0;
			for (auto& p : input_parametres)
				for (auto& c : p->output_connections)
				{
					auto it = idx.find(c->to->owner);
					if (it != idx.end()) { sum += verts[it->second]->pos.y; ++cnt; }
				}
			pos_in = vec2(-X_SPACING, cnt ? sum / cnt : 0.0f);
		}

		// Position graph_out: layer max+1, vertically centred on the nodes feeding it
		{
			float sum = 0; int cnt = 0;
			for (auto& p : output_parametres)
				for (auto& c : p->input_connections)
				{
					auto it = idx.find(c->from->owner);
					if (it != idx.end()) { sum += verts[it->second]->pos.y; ++cnt; }
				}
			pos_out = vec2((max_layer + 1) * X_SPACING, cnt ? sum / cnt : 0.0f);
		}
	}

	void graph::place_node(Node* n)
	{
		constexpr float X_SPACING = 350.0f;
		constexpr float Y_SPACING = 150.0f;

		// Layer = one past the deepest predecessor
		int node_layer = 0;
		for (auto& p : n->input_parametres)
			for (auto& c : p->input_connections)
			{
				auto* pred = c->from->owner;
				if (pred == this) continue;
				int pred_layer = (int)std::round(pred->pos.x / X_SPACING);
				node_layer = std::max(node_layer, pred_layer + 1);
			}

		// Count siblings already at this layer
		int count = 0;
		for (auto& other : nodes)
		{
			if (other.get() == n) continue;
			if ((int)std::round(other->pos.x / X_SPACING) == node_layer)
				++count;
		}

		n->pos = { node_layer * X_SPACING, count * Y_SPACING };
	}

} // namespace FlowGraph

// ============================================================================
// Extracted from FlowGraph.ixx — non-template in-class implementations
// ============================================================================

namespace FlowGraph
{

// ---- listenable -------------------------------------------------------------

void listenable::tell()
{
	for (auto listener : listeners)
		on_tell(listener);
}

std::set<graph_listener*> listenable::get_listeners()
{
	return listeners;
}

void listenable::add_listener(graph_listener* listener, bool send_all)
{
	listeners.insert(listener);
	listener->owners.insert(this);

	if (listener && send_all)
		on_tell(listener);
}

void listenable::remove_listener(graph_listener* listener)
{
	listeners.erase(listener);
	listener->owners.erase(this);
}

listenable::~listenable()
{
	auto c = listeners;

	for (auto listener : c)
		remove_listener(listener);
}

// ---- strict_parameter -------------------------------------------------------

bool strict_parameter::can_cast(parameter_type* /*other*/)
{
	return true;
}

// ---- parameter --------------------------------------------------------------

void parameter::shutdown()
{
	input_connections.clear();
	output_connections.clear();
}

void parameter::clear()
{
	value.clear();
}

bool parameter::has_value()
{
	return value.exists();
}

bool parameter::has_input()
{
	return !input_connections.empty();
}

// ---- connection -------------------------------------------------------------

void connection::pass(MyVariant value)
{
	if (enabled)
		to->put(value);
}

connection::~connection()
{
	from = nullptr;
	to = nullptr;
}

// ---- input ------------------------------------------------------------------

input::input(Node* owner)
{
	this->owner = owner;
	can_input = true;
}

input::input()
{
}

// ---- output -----------------------------------------------------------------

output::output(Node* owner)
{
	this->owner = owner;
	can_output = true;
}

output::output()
{
}

// ---- Node -------------------------------------------------------------------

unsigned int Node::get_id()
{
	return "undefined"_crc32;
}

unsigned int Node::get_graph_id()
{
	return "undefined"_crc32;
}

unsigned int Node::get_id_with_links()
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
		total << p->owner->get_id() << "_";
	}
	return "undefined"_crc32;
}

// ---- graph_input ------------------------------------------------------------

graph_input::graph_input()
{
	immediate_send_next = false;
	can_output = true;
}

graph_input::graph_input(Node* n) : input(n)
{
	immediate_send_next = false;
	can_output = true;
}

// ---- graph_output -----------------------------------------------------------

graph_output::graph_output()
{
	immediate_send_next = false;
	can_input = true;
}

graph_output::graph_output(Node* n) : output(n)
{
	immediate_send_next = false;
	can_input = true;
}

} // namespace FlowGraph

REGISTER_TYPE(FlowGraph::window);
REGISTER_TYPE(FlowGraph::Node);
REGISTER_TYPE(FlowGraph::input);
REGISTER_TYPE(FlowGraph::output);
REGISTER_TYPE(FlowGraph::graph);
REGISTER_TYPE(FlowGraph::graph_input);
REGISTER_TYPE(FlowGraph::graph_output);
REGISTER_TYPE(FlowGraph::parameter_type);
REGISTER_TYPE(FlowGraph::strict_parameter);
CEREAL_FORCE_REGISTER(FlowGraph::window);
// FlowGraph::Node is abstract (pure virtual operator()) — no CEREAL_FORCE_REGISTER
CEREAL_FORCE_REGISTER(FlowGraph::input);
CEREAL_FORCE_REGISTER(FlowGraph::output);
CEREAL_FORCE_REGISTER(FlowGraph::graph);
CEREAL_FORCE_REGISTER(FlowGraph::graph_input);
CEREAL_FORCE_REGISTER(FlowGraph::graph_output);
// FlowGraph::parameter_type is abstract (pure virtual can_cast) — no CEREAL_FORCE_REGISTER
CEREAL_FORCE_REGISTER(FlowGraph::strict_parameter);

// Polymorphic cast paths — registered shallower-first so the BFS builds
// transitive paths (e.g. parameter→graph_input) in a single pass.
CEREAL_REGISTER_POLYMORPHIC_RELATION(FlowGraph::window,         FlowGraph::Node);
CEREAL_REGISTER_POLYMORPHIC_RELATION(FlowGraph::Node,           FlowGraph::graph);
CEREAL_REGISTER_POLYMORPHIC_RELATION(FlowGraph::parameter,      FlowGraph::input);
CEREAL_REGISTER_POLYMORPHIC_RELATION(FlowGraph::parameter,      FlowGraph::output);
CEREAL_REGISTER_POLYMORPHIC_RELATION(FlowGraph::input,          FlowGraph::graph_input);
CEREAL_REGISTER_POLYMORPHIC_RELATION(FlowGraph::output,         FlowGraph::graph_output);
CEREAL_REGISTER_POLYMORPHIC_RELATION(FlowGraph::parameter_type, FlowGraph::strict_parameter);

CEREAL_FORCE_REGISTER_RELATION(FlowGraph::window,         FlowGraph::Node);
CEREAL_FORCE_REGISTER_RELATION(FlowGraph::Node,           FlowGraph::graph);
CEREAL_FORCE_REGISTER_RELATION(FlowGraph::parameter,      FlowGraph::input);
CEREAL_FORCE_REGISTER_RELATION(FlowGraph::parameter,      FlowGraph::output);
CEREAL_FORCE_REGISTER_RELATION(FlowGraph::input,          FlowGraph::graph_input);
CEREAL_FORCE_REGISTER_RELATION(FlowGraph::output,         FlowGraph::graph_output);
CEREAL_FORCE_REGISTER_RELATION(FlowGraph::parameter_type, FlowGraph::strict_parameter);
