module;

#include "DX12/dx12_types.h"
#include "pch_dx.h"

export module PSO;
export import "PSO_defines.h";
import PipelineState;
import Data;
import Threading;
import Profiling;

import Concepts;
import D3D.Shaders;

import RootSignature;
import Utils;
import d3d12_types;
import Memory;
import Enums;

import ppl;
using namespace concurrency;

export
{


struct Nullable {
	static const bool Value = true;
};

struct NonNullable {

	static const bool Value = false;
};

struct NoValue
{
	GEN_DEF_COMP(NoValue);
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

				assert(find);
			}
		}

	}

	GEN_DEF_COMP(KeyValue);
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


template<typename T> concept CanToString =
requires (T t) {
	std::to_string(t);
};



struct SimpleComputePSO {
	Layouts root_signature;
	D3D::shader_header compute;
	std::string name;

	SimpleComputePSO(std::string name) :name(name)
	{
	//	Log::get() << "PSO: " << name << Log::endl;

	}

	Render::ComputePipelineState::ptr create();
};


struct  SimpleGraphicsPSO {
	Layouts root_signature;
	D3D::shader_header pixel;
	D3D::shader_header vertex;
	D3D::shader_header geometry;
	D3D::shader_header hull;
	D3D::shader_header domain;
	D3D::shader_header mesh;
	D3D::shader_header amplification;


	std::vector<DXGI_FORMAT> rtv_formats;
	std::vector<Render::RenderTarget> blend;
	std::string name;

	bool conservative;
	bool depth_write;
	bool enable_depth;
	DXGI_FORMAT ds;
	D3D12_COMPARISON_FUNC depth_func;
	D3D12_CULL_MODE cull;
	D3D12_PRIMITIVE_TOPOLOGY_TYPE topology;
	char stencil_read_mask = 0;
	char stencil_write_mask = 0;

	bool enable_stencil = false;
	D3D12_COMPARISON_FUNC stencil_func;
	D3D12_STENCIL_OP stencil_pass_op;

	SimpleGraphicsPSO(std::string name) :name(name)
	{
		//Log::get() << "PSO: " << name << Log::endl;
		ds = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;
		conservative = false;
		depth_write = true;
		depth_func = D3D12_COMPARISON_FUNC_LESS_EQUAL;
		cull = D3D12_CULL_MODE_BACK;
		topology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		enable_depth = true;
		stencil_func = D3D12_COMPARISON_FUNC_ALWAYS;
		stencil_pass_op = D3D12_STENCIL_OP_REPLACE;
	}

	Render::PipelineState::ptr create();
};


template <auto Member, auto ... Args>
struct ShaderDefine
{
	const char* str;
	ShaderDefine(const char* str) :str(str)
	{

	}
	template<class PSO, auto Shader>
	void ApplyOne(PSO& pso, D3D::shader_macro& m) const
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
			D3D::shader_macro m;


			if constexpr (CanToString<decltype((keys.*Member).value)>)
				m = { std::string(str), std::to_string((keys.*Member).value) };
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
	static auto create(ptr& target)
	{
		return create_task([&]()
		{
				PROFILE(convert(typeid(T).name()));
				target = std::make_shared<T>();
		});
	}
};

class PSOHolder : public Singleton<PSOHolder>
{
	using ptr = std::shared_ptr<PSOBase>;
	enum_array<PSO, ptr> psos;

public:
	PSOHolder();

	template<class T>
	typename T::PSOState::ptr GetPSO(KeyPair<typename T::Keys> k = KeyPair<typename T::Keys>())
	{
		auto pso = static_cast<T*>(psos[T::ID].get());
		return pso->GetPSO(k);
	}
};


template<class T>
typename T::PSOState::ptr GetPSO(KeyPair<typename T::Keys> k = KeyPair<typename T::Keys>())
{
	return PSOHolder::get().GetPSO<T>(k);
}
template<class T>
struct AutoGenPSO
{
	Render::ComputePipelineStateDesc desc;

	AutoGenPSO()
	{
		desc.root_signature = get_Signature(T::layout);
		desc.shader = Render::compute_shader::get_resource(T::compute);
	}


	Render::ComputePipelineState::ptr create_pso(PSO pso, std::string name)
	{
		return Render::ComputePipelineState::create(desc, name);
	}
};

}