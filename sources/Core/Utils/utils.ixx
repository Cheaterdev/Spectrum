module;
#include "utils/utils_macros.h"
export module Utils;

export import stl.core;
export import magic_enum;
import crossguid;


export
{

	constexpr std::size_t operator "" _t(unsigned long long int x) { return x; }
	constexpr std::size_t operator "" _kb(unsigned long long int x) { return x * 1024; }
	constexpr std::size_t operator "" _mb(unsigned long long int x) { return x * 1024 * 1024; }
	constexpr std::size_t operator "" _gb(unsigned long long int x) { return x * 1024 * 1024 * 1024; }



	constexpr std::chrono::milliseconds operator ""_ms(unsigned long long ms)
	{
		return std::chrono::milliseconds(ms);
	}
	constexpr std::chrono::duration<long double, std::milli> operator ""_ms(long double ms)
	{
		return std::chrono::duration<long double, std::milli>(ms);
	}


	template <class T> concept NonString = !std::is_convertible_v<T, std::string_view> && !std::is_convertible_v<T, std::wstring_view>;
	template<typename T> concept HaveEqual = requires (T a, T b) { a == b; };

	template<typename T>
	concept Enum = std::is_enum_v<T>;

	template<typename T>
	concept Scalar = std::is_scalar_v<T>;

	template<typename T>
	concept Fundamental = std::is_fundamental_v<T>;

	template<typename T>
	concept NonFundamental = !std::is_fundamental_v<T>;


	template<std::size_t index, typename T, typename Tuple>
	constexpr size_t tuple_element_index_helper()
	{
		if constexpr (index == std::tuple_size_v<Tuple>) {
			return index;
		}
		else {
			return std::is_same_v<T, std::tuple_element_t<index, Tuple>> ?
				index : tuple_element_index_helper<index + 1, T, Tuple>();
		}
	}

	template<typename T, typename Tuple>
	constexpr size_t tuple_element_index() {
		return tuple_element_index_helper<0, T, Tuple>();
	}

	std::wstring convert(std::string_view str);
	std::string convert(std::wstring_view wstr);
	std::string to_lower(std::string_view str);
	std::wstring to_lower(std::wstring_view str);

	template<class T> using s_ptr = std::shared_ptr<T>;
	template<class T> using w_ptr = std::weak_ptr<T>;




	template<typename T> concept EnumType =
		requires (T t) {
			{T::__GENERATE_OPS__ };
	};

	template<EnumType Enum>
	Enum operator ~(const Enum l)
	{
		using underlying = typename std::underlying_type<Enum>::type;
		return static_cast<Enum> (
			~static_cast<underlying>(l)
			);
	}

	template<EnumType Enum>
	Enum operator |(const Enum lhs, const Enum rhs)
	{
		using underlying = typename std::underlying_type<Enum>::type;
		return static_cast<Enum> (
			static_cast<underlying>(lhs) |
			static_cast<underlying>(rhs)
			);
	}



	template<EnumType Enum>
	Enum operator &(const Enum lhs, const Enum rhs)
	{
		using underlying = typename std::underlying_type<Enum>::type;
		return static_cast<Enum> (
			static_cast<underlying>(lhs) &
			static_cast<underlying>(rhs)
			);
	}


	template<EnumType Enum>
	bool operator &&(const Enum lhs, const Enum rhs)
	{
		using underlying = typename std::underlying_type<Enum>::type;
		return static_cast<Enum> (
			static_cast<underlying>(lhs) &&
			static_cast<underlying>(rhs)
			);
	}



	template<EnumType Enum>
	bool operator ==(const Enum lhs, const Enum rhs)
	{
		using underlying = typename std::underlying_type<Enum>::type;
		return static_cast<Enum> (
			static_cast<underlying>(lhs) ==
			static_cast<underlying>(rhs)
			);
	}

	template<EnumType Enum>
	bool operator !=(const Enum lhs, const Enum rhs)
	{
		using underlying = typename std::underlying_type<Enum>::type;
		return static_cast<Enum> (
			static_cast<underlying>(lhs) !=
			static_cast<underlying>(rhs)
			);
	}

	template<EnumType Enum>
	bool operator <=(const Enum lhs, const Enum rhs)
	{
		using underlying = typename std::underlying_type<Enum>::type;
		return static_cast<Enum> (
			static_cast<underlying>(lhs) <=
			static_cast<underlying>(rhs)
			);
	}

	template<EnumType Enum>
	bool check(Enum lhs)
	{
		using underlying = typename std::underlying_type<Enum>::type;
		return  (
			static_cast<underlying>(lhs)
			) != 0;
	}



	namespace Templates
	{
		template <typename Head0, typename Head1, typename... Tail>
		constexpr auto min(Head0&& head0, Head1&& head1, Tail &&... tail)
		{
			if constexpr (sizeof...(tail) == 0) {
				return head0 < head1 ? head0 : head1;
			}
			else {
				return min(min(head0, head1), tail...);
			}
		}

		template <typename Head0>
		constexpr auto max(Head0 head0)
		{
			return head0;
		}

		template <typename Head0, typename Head1, typename... Tail>
		constexpr auto max(Head0 head0, Head1 head1, Tail ... tail)
		{
			if constexpr (sizeof...(tail) == 0) {
				return head0 > head1 ? head0 : head1;
			}
			else {
				return max(max(head0, head1), tail...);
			}
		}
	}



	template <class T>
	class Member
	{
		T member;
	protected:
		T& get()
		{
			return member;
		}

	};
	template< class ...Args>
	class GenerateMembersImpl
	{

	};

	template<class T, class ...Args>
	class GenerateMembersImpl<T, Args...> : public Member<T>, public GenerateMembersImpl<Args...>
	{

	};


	template< class ...Args>
	class GenerateMembers :public GenerateMembersImpl<Args...>
	{
	protected:
		template<class T>
		T& get_member()
		{
			return Member<T>::get();
		}
	};



	struct guid_compare {
		bool operator()(const Guid& l, const Guid& r) const {
			auto& a = l.bytes();
			auto& b = r.bytes();


			for (unsigned int i = 0; i < b.size(); i++)
			{
				if (a[i] != b[i])
					return a[i] < b[i];
			}

			return false;
		}
	};


	using guid_set = std::set<Guid, guid_compare>;

	template<class T>
	using guid_map = std::map<Guid, T, guid_compare>;

}

module: private;


std::wstring convert(std::string_view s)
{
	std::wstring w;
	w.resize(s.size());
	std::transform(s.begin(), s.end(), w.begin(), btowc);
	return w;
}

std::string convert(std::wstring_view s)
{
	std::string w;
	w.resize(s.size());
	std::transform(s.begin(), s.end(), w.begin(), [](wint_t c) {return static_cast<char>(wctob(c)); });
	return w;
}


std::string to_lower(std::string_view str)
{
	std::string lowered;
	lowered.resize(str.size());
	std::transform(str.begin(),
		str.end(),
		lowered.begin(),
		::tolower);
	return lowered;
}

std::wstring to_lower(std::wstring_view str)
{
	std::wstring lowered;
	lowered.resize(str.size());
	std::transform(str.begin(),
		str.end(),
		lowered.begin(),
		::towlower);
	return lowered;
}
