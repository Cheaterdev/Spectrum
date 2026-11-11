export module HAL:PSO;

import ppl;			
import Core;
import :PipelineState;
import :RootSignature;
import :API.IndirectCommand;

using namespace concurrency;

export
{


struct Nullable {
	static const bool Value = true;

		SERIALIZE()
			{
				
			}
};

struct NonNullable {

	static const bool Value = false;
		SERIALIZE()
			{
				
			}
};

struct NoValue
{
	GEN_DEF_COMP(NoValue);

		SERIALIZE()
			{
				
			}
};

template<class T, class N, T ...Args>
struct KeyValue
{
	using Type = T;
	bool use;
	Type value;


	static inline const std::array<T, sizeof...(Args)> values = { Args... };

	template <class Wrapper = T>
	static std::vector<Wrapper> Aval()
	{
		if constexpr (N::Value)
		{
			if constexpr (sizeof...(Args) == 0)
			{
				return std::vector<Wrapper>{Wrapper(false, Type()), Wrapper(true, Type())};
			}
			else
			{
				return std::vector<Wrapper>{Wrapper(false, Type()), Wrapper(true, Args)... };
			}
		}
		else
		{
			return std::vector<Wrapper>{ Wrapper(true, Args)... };
		}

	}

	KeyValue() :value(), use(false) {


	}

	void set(bool use, Type value)
	{
		this->use = use;
		if (!use) value = Type();
		this->value = value;

		if constexpr (sizeof...(Args) > 0)
		{
			if (use) {
				bool find = false;
				for (auto e : { Args... })
					if (e == value) {
						find = true;
						break;
					}

				ASSERT(find);
			}
		}

	}

	GEN_DEF_COMP(KeyValue);

	SERIALIZE()
	{
		ar& NVP(use);
		ar& NVP(value);
	}
};

template<class Keys>
struct KeyPair
{
	Keys keys;

	KeyPair() : keys() {}

	template<class Key>
	KeyPair(const Key& key) : keys() { key.modify(keys); }

	template<class Key>
	KeyPair& operator|(const Key& key) {
		key.modify(keys);
		return *this;
	};

	template<class Key>
	KeyPair& operator|=(const Key& key) {
		key.modify(keys);
		return *this;
	};


	Keys GetKey()
	{
		return keys;
	}
};

template <class C, typename T>
T getPointerType(T C::* v);

template <class C, typename T>
C getMemberType(T C::* v);


template<auto Member>
struct Key {
	using KeyType = decltype(getPointerType(Member));
	using KeysType = decltype(getMemberType(Member));
	using Type = typename KeyType::Type;
	using KeyPairType = KeyPair<KeysType>;

	bool use;
	Type value;
	Key() = delete;

	Key(bool use, Type value = Type()) :use(use), value(value) {

	};

	template <class OtherKey>
	KeyPairType operator|(const OtherKey& other) const
	{
		return KeyPairType(*this) | other;
	};

	void modify(KeysType& keys) const
	{
		(keys.*Member).set(use, value);
	}
};

template<auto Member, bool direct = true>
struct KeyGenerator
{
	using KeyType = decltype(getPointerType(Member));
	using KeysType = decltype(getMemberType(Member));
	using Type = typename KeyType::Type;
	using KeyPairType = KeyPair<KeysType>;

	KeyGenerator()
	{

	}

	Key<Member> Use(bool v) const
	{
		return Key<Member>(v);
	}

	auto Aval() const
	{
		return KeyType::template  Aval<Key<Member>>();
	}

	Key<Member> operator()(Type v = Type(), bool use = true) const { 
		if constexpr (direct)
		{
			return Key<Member>(use, v);

		}
		else
		{
			return Key<Member>(use, KeyType::values[v]);

		}
		
	};

	template <class OtherKey>
	KeyPairType operator|(const OtherKey& other) const
	{
		return KeyPairType(this->operator()()) | other;
	};

};


template<typename T> concept CanEnumToString =
requires (T t) {
	std::string(magic_enum::enum_name(t));
};


template<typename T> concept CanToString =
requires (T t) {
	std::to_string(t);
};


template<typename T> concept CanToStringSelf =
requires (T t) {
	t.to_string();
};



// create() returns null for a PSO the device can't run -- a mesh/amplification
// stage without mesh shaders, or requires_raytracing (inline RayQuery; set from
// the .prism's [Requires = Raytracing]) without raytracing. Passes using such a
// PSO must be gated on the same feature; GetPSO() asserts on the null entry
// of an eager set (a lazy one returns null, see PSOVariantMap).
struct SimpleComputePSO {
	Layouts root_signature;
	HAL::shader_header compute;
	std::string name;
	bool requires_raytracing = false;
	bool requires_tiled_depth = false;   // [Requires = TiledDepth]

	SimpleComputePSO(std::string name) :name(name)
	{
	//	Log::get() << "PSO: " << name << Log::endl;

	}

	HAL::ComputePipelineState::ptr create(HAL::Device& device);
};

				 

struct  SimpleGraphicsPSO {
	Layouts root_signature;
	HAL::shader_header pixel;
	HAL::shader_header vertex;
	HAL::shader_header geometry;
	HAL::shader_header hull;
	HAL::shader_header domain;
	HAL::shader_header mesh;
	HAL::shader_header amplification;


	std::vector<HAL::Format> rtv_formats;
	std::vector<HAL::RenderTarget> blend;
	std::string name;
	bool requires_raytracing = false;
	bool requires_tiled_depth = false;   // [Requires = TiledDepth]

	bool conservative;
	bool depth_write;
	bool enable_depth;
	HAL::Format ds;
	HAL::ComparisonFunc depth_func;
	HAL::CullMode cull;
	HAL::PrimitiveTopologyType topology;
	char stencil_read_mask = 0;
	char stencil_write_mask = 0;

	// Same trio as HAL::RasterizerState's own -- see its comment for what
	// these do vs. VSM's shader-side compare bias.
	int depth_bias = 0;
	float depth_bias_clamp = 0.0f;
	float slope_scaled_depth_bias = 0.0f;

	bool enable_stencil = false;
	HAL::ComparisonFunc stencil_func;
	HAL::StencilOp stencil_pass_op;

	SimpleGraphicsPSO(std::string name) :name(name)
	{
		//Log::get() << "PSO: " << name << Log::endl;
		ds = HAL::Format::UNKNOWN;
		conservative = false;
		depth_write = true;
		depth_func = HAL::ComparisonFunc::GREATER_EQUAL;
		cull = HAL::CullMode::Back;
		topology = HAL::PrimitiveTopologyType::TRIANGLE;
		enable_depth = true;
		stencil_func = HAL::ComparisonFunc::ALWAYS;
		stencil_pass_op = HAL::StencilOp::Replace;
	}

	HAL::PipelineState::ptr create(HAL::Device& device);

	// What create() builds the pipeline from (its shaders loaded), without creating it. Empty for a PSO the
	// device can't run, which is reported to the observer as Unsupported here.
	std::optional<HAL::PipelineStateDesc> make_desc(HAL::Device& device);
};


enum class PSOBuildResult
{
	Created,
	Unsupported,   // device lacks a feature the PSO requires (mesh shaders, ray queries, tiled depth)
	Failed         // shader compile or pipeline creation failed
};

// Lets offline tooling (spectrum_tool) see every PSO + key combination the
// engine builds, without the engine knowing about the tool. Unset in the game.
// Function-local so no dynamic-init static lives in the module interface.
struct PSOBuildObserver
{
	using Compute  = std::function<void(const SimpleComputePSO&, PSOBuildResult)>;
	using Graphics = std::function<void(const SimpleGraphicsPSO&, PSOBuildResult)>;

	static Compute& compute() { static Compute f; return f; }
	static Graphics& graphics() { static Graphics f; return f; }

	// Compile the shaders of PSOs the current device can't run (and then report
	// Unsupported), so a precompile pass covers every target's variants.
	static bool& compile_unsupported() { static bool v = false; return v; }
};

// Eager: every key variant's pipeline is created in the constructor (engine sets, tools, tests).
// Lazy: the constructor only prepares each variant's description; a variant's pipeline is created on the thread pool
// the first time GetPSO() asks for it, and GetPSO() returns null until it is built.
// A set constructed while PSOBuildObserver::graphics() is set is eager either way, so the tool sees every variant.
enum class PSOCreation
{
	Eager,
	Lazy
};

// One key variant of a lazily created graphics PSO set (see PSOVariantMap). Its description is complete (shaders
// loaded), so any variant can be created later on a device without a shader compiler.
class PSOVariant : public std::enable_shared_from_this<PSOVariant>
{
	enum class Status : int
	{
		Idle,
		Queued,        // a pool task will create it, unless create_now() gets there first
		Creating,
		Created,       // `pso` is set; built unless its shaders are still compiling or the creation failed
		Unavailable    // the device can't run it, or the creation threw
	};

	HAL::Device& device;
	HAL::PipelineState::ptr pso;   // written once, before `status` becomes Created (release); read after (acquire)
	std::atomic<Status> status = Status::Idle;

	void create();

public:
	// Null for a variant the device can't run that has nothing to save either (see SimpleGraphicsPSO::make_desc).
	const std::optional<HAL::PipelineStateDesc> desc;

	PSOVariant(HAL::Device& device, std::optional<HAL::PipelineStateDesc> desc);

	bool idle() const { return status.load(std::memory_order_acquire) == Status::Idle; }

	// Queues the creation on the asset lane if nothing has started it. Never waits. True if this call queued it.
	bool start();

	// Creates it on this thread unless another thread already is (then waits for that one).
	void create_now();

	// The pipeline if it is built, otherwise null. Never waits.
	HAL::PipelineState::ptr get_built() const;

	// Nothing more to wait for: built, or never will be (unavailable).
	bool settled() const;

	// Bumped after every variant creation finishes (built or not), process-wide.
	static std::atomic<uint64_t>& created_count() { static std::atomic<uint64_t> c = 0; return c; }
};

// For a consumer that caches what it draws (static voxels, shadow pages): remembers that a draw was skipped because
// its variant was not built, and says once a variant has been created since, so the cached work is redone then and
// not every frame.
class PSOSkipRecord
{
	static constexpr uint64_t none = ~0ull;
	std::atomic<uint64_t> skipped_at = none;

public:
	// Read before asking for the variant: a creation that finishes after the request then always moves the count.
	static uint64_t now() { return PSOVariant::created_count().load(std::memory_order_acquire); }

	void skipped(uint64_t seen)
	{
		auto at = skipped_at.load(std::memory_order_relaxed);
		while (seen < at && !skipped_at.compare_exchange_weak(at, seen, std::memory_order_acq_rel)) {}
	}

	// True once per skip record, when a variant has been created after the earliest skip.
	bool take_redo()
	{
		auto at = skipped_at.load(std::memory_order_acquire);
		if (at == none || now() == at)
			return false;
		return skipped_at.compare_exchange_strong(at, none, std::memory_order_acq_rel);
	}
};

// The key variants of one graphics PSO set (`psos` of GEN_GRAPHICS_PSO). An eager map holds created pipelines and
// GetPSO() asserts on a missing one, as before. A lazy map holds a PSOVariant per key.
//
// Threading: the map's structure is filled by the constructor or the deserializer, before the set is shared, and
// never changes after; everything a draw thread touches afterwards is the variants' atomics.
template<class Keys>
class PSOVariantMap
{
	struct Entry
	{
		HAL::PipelineState::ptr pso;              // eager
		std::shared_ptr<PSOVariant> variant;      // lazy
	};

	std::map<Keys, Entry> entries;
	HAL::Device* lazy_device = nullptr;

	// Every key a draw has asked for, over all sets of this type: what a set that loads later warms up, so the
	// variants this session draws with exist before its first draw.
	struct UsedKeys
	{
		std::mutex m;
		std::set<Keys> keys;
	};
	static UsedKeys& used() { static UsedKeys u; return u; }

	static std::vector<Keys> used_keys()
	{
		auto& u = used();
		std::lock_guard<std::mutex> g(u.m);
		return std::vector<Keys>(u.keys.begin(), u.keys.end());
	}

	static const HAL::PipelineStateDesc* saved_desc(const Entry& e)
	{
		if (e.variant)
			return e.variant->desc ? &*e.variant->desc : nullptr;
		return e.pso ? &e.pso->desc : nullptr;
	}

public:
	// Eager maps only (a set filled by hand, like the material preview's).
	HAL::PipelineState::ptr& operator[](const Keys& key) { return entries[key].pso; }

	void make_lazy(HAL::Device& device) { lazy_device = &device; }

	void add_lazy(const Keys& key, std::optional<HAL::PipelineStateDesc> desc)
	{
		entries[key].variant = std::make_shared<PSOVariant>(*lazy_device, std::move(desc));
	}

	// Lazy: the built pipeline, or null after starting its creation. Never waits.
	HAL::PipelineState::ptr get(const Keys& key) const
	{
		auto it = entries.find(key);
		if (!lazy_device)
		{
			ASSERT(it != entries.end() && it->second.pso);
			return it != entries.end() ? it->second.pso : nullptr;
		}

		if (it == entries.end())
			return nullptr;

		auto& variant = *it->second.variant;
		if (auto pso = variant.get_built())
			return pso;

		if (variant.idle())
		{
			PROFILE(L"pso_variant_request");
			if (variant.start())
			{
				auto& u = used();
				std::lock_guard<std::mutex> g(u.m);
				u.keys.insert(key);
			}
		}
		return nullptr;
	}

	// Starts the variants draws have asked for (in any set of this type) and, with_default, the default key's.
	// True when all of those are settled. An eager map: all_settled().
	bool warm(bool with_default) const
	{
		if (!lazy_device)
			return all_settled();

		auto keys = used_keys();
		if (with_default)
			keys.push_back(Keys());

		bool ready = true;
		for (auto& key : keys)
		{
			auto it = entries.find(key);
			if (it == entries.end())
				continue;
			it->second.variant->start();
			ready = it->second.variant->settled() && ready;
		}
		return ready;
	}

	// Every variant settled; starts the ones nothing has started.
	bool all_settled() const
	{
		bool ready = true;
		for (auto& [key, e] : entries)
		{
			if (e.variant)
			{
				e.variant->start();
				ready = e.variant->settled() && ready;
			}
			else
				ready = (!e.pso || e.pso->is_built()) && ready;
		}
		return ready;
	}

	// Every variant created now, on this thread (tools, tests, the eager A/B setting). No-op for an eager map.
	void build_all()
	{
		for (auto& [key, e] : entries)
			if (e.variant)
				e.variant->create_now();
	}

	// Same archive layout as HAL::WrapperMap: a count, then (description, key) per variant. Loading creates no
	// pipeline; the loaded map is lazy.
	template<class Archive>
	void serialize_entries(Archive& ar)
	{
		IF_LOAD()
		{
			HAL::Device* dev = nullptr;
			if constexpr (requires { cereal::get_user_data<UniversalContext>(ar); })
				dev = cereal::get_user_data<UniversalContext>(ar).template get_context<HAL::Device*>();
			ASSERT(dev);

			entries.clear();
			lazy_device = dev;

			uint size;
			ar& NVP(size);
			for (uint i = 0; i < size; i++)
			{
				HAL::PipelineStateDesc desc;
				ar& NVP(desc);
				Keys k;
				ar& NVP(k);
				entries[k].variant = std::make_shared<PSOVariant>(*dev, std::move(desc));
			}
		}
		else
		{
			uint size = 0;
			for (auto& [k, e] : entries)
				if (saved_desc(e))
					size++;
			ar& NVP(size);

			for (auto& [k, e] : entries)
				if (auto desc = saved_desc(e))
				{
					ar& NP("v->desc", *desc);
					ar& NVP(k);
				}
		}
	}
};

template<class Keys>
struct PSOVariantMapArchive
{
	PSOVariantMap<Keys>& map;

	SERIALIZE()
	{
		map.serialize_entries(ar);
	}
};

// The generated sets serialize `wrap(psos)`.
template<class Keys>
auto wrap(PSOVariantMap<Keys>& map)
{
	return PSOVariantMapArchive<Keys>{ map };
}

template <auto Member, auto ... Args>
struct ShaderDefine
{
	const char* str;
	ShaderDefine(const char* str) :str(str)
	{

	}
	template<class PSO, auto Shader>
	void ApplyOne(PSO& pso, HAL::shader_macro& m) const
	{
		(pso.*Shader).macros.emplace_back(m);
	}
	template<class PSO, class K>
	auto get_value(PSO& pso, K& keys) const
	{
		return (keys.*Member).value;
	}
	template<class PSO, class K>
	void Apply(PSO& pso, K& keys) const
	{
		if ((keys.*Member).use)
		{
			HAL::shader_macro m;


			using KeyType = std::remove_cvref_t<decltype(keys.*Member)>;
			using ValueType = std::remove_cvref_t<decltype((keys.*Member).value)>;

			// A key that lists no values is an on/off switch: Use(true) leaves its value at the default 0, which would define
			// the macro as 0 and make a plain `#if X` false while `#ifdef X` is true. On defines it as 1; off defines nothing.
			if constexpr (std::is_integral_v<ValueType> && KeyType::values.size() == 0)
				m = { std::string(str), (keys.*Member).value ? std::to_string((keys.*Member).value) : std::string("1") };
			else if constexpr (CanEnumToString<decltype((keys.*Member).value)>)
				m = { std::string(str), std::string(magic_enum::enum_name((keys.*Member).value)) };
			else if constexpr (CanToString<decltype((keys.*Member).value)>)
				m = { std::string(str), std::to_string((keys.*Member).value) };
			else if constexpr (CanToStringSelf<decltype((keys.*Member).value)>)
				m = { std::string(str), (keys.*Member).value.to_string() };
			else
				m = { std::string(str), "1" };

			//(apply<PSO,Args>(pso, m),...);
			(ApplyOne<PSO, Args>(pso, m), ...);

			pso.name += std::string("_") + m.name + "_" + m.value;
		}
	}
};

class PSOBase
{
public:
	using ptr = std::shared_ptr<PSOBase>;
protected:
	template<class F, class K, class T>
	static void gen_pairs(const F& f, K& k, const T& t)
	{
		for (auto e : t.Aval())
		{
			k |= e;
			f(k);
		}
	}

	template<class F, class K, class T, class ...Args>
	static void gen_pairs(const F& f, K& k, const T& t, const Args&...args)
	{
		gen_pairs([&](K k) {
			gen_pairs(f, k, args...);
			}, k, t);
	}

	template<class P, class F, class T, class ...Args>
	static void shuffle_pairs(const F& f, const T& t, const Args&...args)
	{
		typename T::KeyPairType keys;
		gen_pairs([f](auto k) {
			auto key = k.GetKey();
			f(key);
			}, keys, t, args...);
	}


	template<class P,class F>
	static void shuffle_pairs(const F& f)
	{
		typename P::Keys keys;
		f(keys);
	}

public:
	template <class T>
	static auto create(HAL::Device&device, ptr& target)
	{
		return create_task([&]()
		{
		//		PROFILE(convert(typeid(T).name()));
				// A throw here would only surface at when_all().wait() (or, with PPL, never), leaving
				// the engine holder's entry null; name the PSO so the cause is in the log.
				try
				{
					target = std::make_shared<T>(device);
				}
				catch (const std::exception& e)
				{
					Log::get() << Log::LEVEL_ERROR << "PSO " << typeid(T).name() << " creation threw: " << e.what() << Log::endl;
					throw;
				}
		});
	}
};

class EngineRootLayoutHolder
{
	enum_array<Layouts, HAL::RootLayout::ptr> signatures;

public:
	void init(HAL::Device& device);

	HAL::RootLayout::ptr GetSignature(Layouts l)
	{
		return signatures[l];
	}
};

class EnginePSOHolder
{
	using ptr = std::shared_ptr<PSOBase>;
	enum_array<PSO, ptr> psos;
	enum_array<IndirectCommands, HAL::IndirectCommand> commands;

public:
	void init(HAL::Device& device);

	template<class T>
	typename T::PSOState::ptr GetPSO(KeyPair<typename T::Keys> k = KeyPair<typename T::Keys>())
	{
		auto pso = static_cast<T*>(psos[T::ID].get());
		return pso->GetPSO(k);
	}

	HAL::IndirectCommand& GetCommand(IndirectCommands command)
	{
		return commands[command];
	}
};


template<class T>
struct AutoGenPSO
{
	HAL::ComputePipelineStateDesc desc;

	AutoGenPSO()
	{
		desc.layout = T::layout;
		desc.shader = HAL::compute_shader::get_resource(T::compute);
	}


	HAL::ComputePipelineState::ptr create_pso(HAL::Device& device, PSO pso, std::string name)
	{
		return HAL::ComputePipelineState::create(device, desc, name);
	}
};

}