module;

#include "utils/utils_macros.h"
#include "Serialization/serialization_defines.h"

export module Data;

import ZipLib;
import serialization;

import stl.core;
import Utils;
import Exceptions;

export
{

	namespace DataPacker
	{

		std::string pack(std::string unpacked);

		std::string unpack(std::string str);

		ZipArchiveEntry::Ptr create_entry(ZipArchive::Ptr archive, std::string name, std::string data, bool pack = false);

		std::string zip_to_string(ZipArchive::Ptr archive);

		std::string load_all(std::istream& s);

	};


	class MyVariant
	{

		class var_base
		{
			friend class MyVariant;
		protected:

			var_base(std::reference_wrapper<const std::type_info> r);

			virtual   std::unique_ptr<var_base> clone() = 0;

			virtual bool Equals(var_base* other) = 0;
		public:
			std::reference_wrapper<const std::type_info> type;
			virtual ~var_base() = default;
		};





		template<class T>
		class var_typed : public var_base
		{
		public:
			T value;

			/*	var_typed(const T& v) : var_base(typeid(T)), value(v)
				{
				}
				*/
			template<class ...Args>
			var_typed(Args...args) : var_base(typeid(T)), value(args...)
			{

			}

			var_typed() : var_base(typeid(T))
			{

			}

			virtual bool Equals(var_base* other)
			{
				auto otyped = dynamic_cast<var_typed<T>*>(other);

				if (otyped)
				{
					if constexpr (HaveEqual<T>)
						return value == otyped->value;
					else
						return false;
				}

				return false;
			}


			virtual std::unique_ptr<var_base> clone() override
			{
				if constexpr (std::is_copy_constructible_v<T>)
					return std::make_unique<var_typed<T>>(value);
				else
					return nullptr;// std::make_unique<var_typed<T>>();
			}

			virtual ~var_typed()
			{
			};
		};
		std::unique_ptr<var_base> typed;

	public:
		//      LEAK_TEST
		MyVariant() = default;
		MyVariant(const MyVariant& r);
		virtual ~MyVariant();

		template<class T>
		void operator = (const T& value)
		{
			typed.reset(new var_typed<T>(value));
		}

		template<class T, class ...Args>
		void create(Args...args)
		{
			typed.reset(new var_typed<T>(args...));
		}
		/*
		template<class T>
		void create()
		{
			typed.reset(new var_typed<T>());
		}

		*/
		void operator = (MyVariant value);


		//template<class T>
		bool operator!=(const MyVariant& other) const
		{
			//var_typed<T>* var = dynamic_cast<var_typed<T>*>(typed.get());
			if (!typed) return true;

			return !typed->Equals(other.typed.get());

			//	return true;
		}
		template<class T>
		bool operator!=(const T& other) const
		{
			var_typed<T>* var = dynamic_cast<var_typed<T>*>(typed.get());

			if (var)
				return var->value != other;

			return true;
		}



		template<class T>
		T& get() const
		{
			var_typed<T>* var = dynamic_cast<var_typed<T>*>(typed.get());

			if (var)
				return var->value;

			assert(false);
			throw Exceptions::Exception("wrong cast type");
		}

		template<class T>
		T* try_get() const
		{
			var_typed<T>* var = dynamic_cast<var_typed<T>*>(typed.get());

			if (var)
				return &var->value;

			return nullptr;
		}

		bool exists();
		void clear();
	};



	template<class I, class V>
	class Cache
	{

		std::mutex m;

	public:
		std::map<I, V> table;

		std::function<V(const I&)> create_func;

		size_t size()
		{
			return table.size();
		}
		Cache() = default;

		Cache(std::function<V(const I&)> create_func) : create_func(create_func) {}

		V& operator[](const I& i)
		{
			m.lock();
			bool first = table.find(i) == table.end();
			auto&& elem = table[i];
			m.unlock();

			if (!elem)
			{
				if (first)
					elem = create_func(i);
				else
				{
					while (!elem)
						std::this_thread::yield();
				}
			}

			return elem;
		}

	private:
		SERIALIZE()
		{
			ar& NVP(table);
		}

	};



	template<class T>
	class Void
	{
		std::shared_ptr<T> object;
		std::mutex m;
		bool loading = false;
	public:
		std::function<T()> create_func;



		Void() = default;

		Void(std::function<T()> create_func) : create_func(create_func) {}

		operator bool()
		{
			return object.valid();
		}

		void set(const T& obj)
		{
			object.reset(new T(obj));
		}

		std::shared_ptr<T>& get()
		{
			if (!object)
			{
				m.lock();
				bool me_loading = !loading;
				loading = true;
				m.unlock();

				if (me_loading)
					set(create_func());
				else
					while (!object)
						std::this_thread::yield();
			}

			return object;
		}
		/*template<class T>
		void get(T f)
		{

		}*/

		T* operator->()
		{
			return get().get();
		}
	};


	template<EnumType E, class T, std::size_t N = std::size_t(E::__GENERATE_OPS__)>
	class enum_array : public std::array<T, N> {
	public:
		T& operator[] (E e) {
			return std::array<T, N>::operator[]((std::size_t)e);
		}

		const T& operator[] (E e) const {
			return std::array<T, N>::operator[]((std::size_t)e);
		}
	};


}


module: private;


namespace DataPacker
{

	std::string pack(std::string unpacked)
	{
		std::string packed;
		packed.resize(int(1.01 * unpacked.size()) + 12 + sizeof(unsigned long));
		unsigned long T = (unsigned long)packed.size() - sizeof(unsigned long);
		compress((Bytef*)packed.data() + sizeof(unsigned long), &T, (Bytef*)unpacked.data(), (unsigned long)unpacked.size());
		packed.resize(T + sizeof(unsigned long));
		T = (unsigned long)unpacked.size();
		memcpy((void*)packed.data(), &T, sizeof(T));
		return std::move(packed);
	}

	std::string unpack(std::string str)
	{
		std::string result;
		unsigned long T = *reinterpret_cast<unsigned long*>(const_cast<char*>(str.data()));
		result.resize(T);
		uncompress((Bytef*)result.data(), &T, (Bytef*)str.data() + sizeof(unsigned long), (unsigned long)str.size() - sizeof(unsigned long));
		return std::move(result);
	}


	ZipArchiveEntry::Ptr create_entry(ZipArchive::Ptr archive, std::string name, std::string data, bool pack)
	{
		ZipArchiveEntry::Ptr entry = archive->CreateEntry(name);
		std::istringstream sss(data);

		if (pack)
			entry->SetCompressionStream(sss, DeflateMethod::Create(), ZipArchiveEntry::CompressionMode::Immediate);
		else
			entry->SetCompressionStream(sss, StoreMethod::Create(), ZipArchiveEntry::CompressionMode::Immediate);

		return entry;
	}


	std::string zip_to_string(ZipArchive::Ptr archive)
	{
		std::ostringstream s;
		archive->WriteToStream(s);
		return s.str();
	}

	std::string load_all(std::istream& s)
	{
		std::string result;
		s.seekg(0, std::ios::end);
		auto size = unsigned int(s.tellg());
		result.resize(size);
		s.seekg(0, std::ios::beg);
		s.read(const_cast<char*>(result.data()), result.size());
		//memcpy(reinterpret_cast<char*>(result.data()),s.)
		return std::move(result);
	}


};



MyVariant::var_base::var_base(std::reference_wrapper<const std::type_info> r) : type(r) {}



MyVariant::MyVariant(const MyVariant& r)
{
	if (r.typed)
		typed = r.typed->clone();
}

MyVariant::~MyVariant()
{
	typed.reset();
}




void MyVariant::operator = (MyVariant value)
{
	typed = value.typed->clone();

	if (!typed)
		typed = nullptr;
}

bool MyVariant::exists()
{
	return !!typed;
}
void MyVariant::clear()
{
	typed.reset();
}



std::strong_ordering operator<=>(const std::string& l, const std::string& r) {

	if (auto cmp = l.size() <=> r.size(); cmp != 0) return cmp;


	for (size_t i = 0; i < l.size(); i++)
		if (auto cmp = (l[i] <=> r[i]); cmp != 0) return cmp;

	return  std::strong_ordering::equal;
}

