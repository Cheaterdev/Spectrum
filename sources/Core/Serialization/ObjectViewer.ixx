export module Core:ObjectViewer;
import <Core_defs.h>;
export import :serialization;
using serialization_exception = cereal::Exception;
import :Log;
import :Utils;
import :crc32;

import :Data;
import windows;
import :Tree;

export
{



	class member_item : public ::tree<member_item, my_unique_vector<std::shared_ptr<member_item>>>
	{

		std::mutex m;

	public:
		std::wstring name;
		std::wstring type;
		std::wstring value;
		bool child_ignored = false;
		member_item(std::wstring name)
		{
			this->name = name;
		}
		using ptr = s_ptr<member_item>;
		virtual std::wstring get_name() const override
		{
			return name + L" " + value;
		}

	};

}



export
{



	class ObjectTreeSerializer : public cereal::OutputArchive<ObjectTreeSerializer>//, public cereal::traits::TextArchive
	{
		member_item::ptr current = nullptr;
		member_item::ptr root;
	public:
		static constexpr bool PRETTY = true;
	public:
									  
		template <typename T>
		 static member_item::ptr describe(const T& value)
		 {
		   ObjectTreeSerializer s(value);

		   	return s.root;
		 }
	private:


		template <typename T>
		ObjectTreeSerializer(const T& value) :
			cereal::OutputArchive<ObjectTreeSerializer>(this)
		{

			root = std::make_shared<member_item>(convert(get_typename<T>()));
			current = root;

			(*this)(value);
		}

	public:
		template<arithmetic T>
		void save(const T& t)
		{
			current->value += L" = ";
			//	if (!current->value.empty()) current->value += L", ";
			current->value += std::to_wstring(t);// + L" ";
		}


		template<class T, std::size_t C>
		void save(const std::array<T, C>& t)
		{
			save_array(std::span(t));
		}


		template<class T, std::size_t C>
		void save(const  T(&t)[C])
		{
			save_array(std::span(t));
		}

		template<class T, std::size_t size>
		void save_array(std::span<const T, size> t)
		{
			if (!current->child_ignored)
				current->type = convert(get_typename<std::span<const T, size>>());

			member_item::ptr prev = current;

			for (uint i = 0; i < t.size(); i++)
			{
				member_item::ptr member = std::make_shared<member_item>(L"");

				current->add_child(member);

				member->type = convert(get_typename<T>());

				current = member;
				if constexpr (can_save<ObjectTreeSerializer, T>)
				{
					save(t[i]);
				}
				else
				{
					(*this)(t[i]);
				}

				std::wstring name = std::wstring(L"[") + std::to_wstring(i) + L"]";

				auto child_iterator = [&name](this const auto& self, const member_item::ptr& item) ->void {

					if (!item->get_childs().empty())
					{
						name += L"{ ";
						for (auto& c : item->get_childs())
						{
							name += c->get_name() + L"; ";
							self(c);
						}
						name += L"}";
					}
					};


				child_iterator(member);
				member->name = name;

				current = prev;
			}
		}

		template<class T>
		void save(const cereal::NameValuePair<T>& t);

		void save(const std::string& t)
		{
			if (!current->value.empty()) current->value += L", ";
			current->value += convert(t);// + L" ";
		}

		void save(const std::wstring& t)
		{
			if (!current->value.empty()) current->value += L", ";
			current->value += (t);// + L" ";
		}

	};





	template<class T>
	void ObjectTreeSerializer::save(const cereal::NameValuePair<T>& t)
	{
		member_item::ptr prev = current;

		current->child_ignored = t.name == "elems";
		if (!current->child_ignored)
		{
			member_item::ptr member = std::make_shared<member_item>(convert(t.name));

			member->type = convert(get_typename<std::remove_cvref_t<T>>());
			current->add_child(member);

			current = member;
		}
		if constexpr (can_save<ObjectTreeSerializer, T>)
		{
			save(t.value);
		}
		else
		{
			(*this)(t.value);
		}

		current = prev;

	}





}




#define CEREAL_REGISTER_ARCHIVE(Archive)                              \
  namespace cereal { namespace detail {                                 \
  template <class T, class BindingTag>                                  \
  typename polymorphic_serialization_support<Archive, T>::type          \
  instantiate_polymorphic_binding( T*, Archive*, BindingTag, adl_tag ); \
  } } /* end namespaces */


CEREAL_REGISTER_ARCHIVE(ObjectTreeSerializer);



export
{

	namespace cereal
	{

		template<arithmetic T>
		void serialize(ObjectTreeSerializer& archive,
			T& m)
		{
			archive.save(m);
		}


		template<class T>
		void serialize(ObjectTreeSerializer& archive,
			cereal::NameValuePair<T>& m)
		{
			if (strcmp(m.name, "cereal_class_version") == 0) return;
			archive.save(m);

		}

		template<class T>
		void serialize(ObjectTreeSerializer& archive,
			cereal::SizeTag<T>& m)
		{
			//archive.save(m);
		}

		void serialize(ObjectTreeSerializer& archive,
			std::wstring& m)
		{
			archive.save(m);
		}
		/*
		 template<class T, unsigned int C>
	void serialize(ObjectTreeSerializer& archive,
		const std::array<T,C>&  m)
	{
		archive.save(m);
	}
				*/
		void serialize(ObjectTreeSerializer& archive,
			std::string& m)
		{
			archive.save(m);
		}

	}
}