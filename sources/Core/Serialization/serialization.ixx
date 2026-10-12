export module Core:serialization;
export import "serialization_defines.h";
export import cereal;
export import stl.core;
export import stl.threading;
export import stl.filesystem;

export import crossguid;

import :Utils;

export template<typename T> concept PrettyArchive = requires () { T::PRETTY; };

export using serialization_exception = cereal::Exception;

export
{

	namespace cereal
	{
		template<class Archive>
		void serialize(Archive& ar, Guid& g)
		{
			IF_LOAD()
			{
				std::array<unsigned char, 16> v;
				ar& NVP(v);
				g = Guid(v);

			}
			else
			{
			auto& v = g.bytes();
			ar& NVP(v);
			}
		}

		// wchar_t is 2 bytes on Windows but 4 on Android, and cereal writes a std::wstring as raw
		// wchar_t elements, so a cache file written on one platform is unreadable on the other.
		// Wide strings that end up in cached files are archived as std::u16string instead (UTF-16
		// code units; byte-identical to the Windows wstring format) via these converters.
		// (A cereal save/load overload for std::wstring loses to cereal's generic basic_string one.)
		namespace wide_detail
		{
			inline std::u16string to_utf16(const std::wstring& s)
			{
				if constexpr (sizeof(wchar_t) == 2)
					return std::u16string(s.begin(), s.end());
				else
				{
					std::u16string out;
					out.reserve(s.size());
					for (wchar_t c : s)
					{
						const char32_t cp = (char32_t)c;
						if (cp >= 0x10000)
						{
							out.push_back((char16_t)(0xD800 + ((cp - 0x10000) >> 10)));
							out.push_back((char16_t)(0xDC00 + ((cp - 0x10000) & 0x3FF)));
						}
						else
							out.push_back((char16_t)cp);
					}
					return out;
				}
			}

			inline std::wstring from_utf16(const std::u16string& s)
			{
				if constexpr (sizeof(wchar_t) == 2)
					return std::wstring(s.begin(), s.end());
				else
				{
					std::wstring out;
					out.reserve(s.size());
					for (size_t i = 0; i < s.size(); ++i)
					{
						char32_t cp = s[i];
						if (cp >= 0xD800 && cp < 0xDC00 && i + 1 < s.size() && s[i + 1] >= 0xDC00 && s[i + 1] < 0xE000)
							cp = 0x10000 + ((cp - 0xD800) << 10) + (s[++i] - 0xDC00);
						out.push_back((wchar_t)cp);
					}
					return out;
				}
			}
		}

		template<class Archive>
		void serialize(Archive& ar, std::filesystem::path& p)
		{
			// libc++ on Android throws "locale not supported" converting path <-> wstring, so there
			// the path is bridged through UTF-8 (convert()) instead.
			std::u16string s;
			if (Archive::is_saving::value)
			{
				if constexpr (sizeof(wchar_t) == 2)
					s = wide_detail::to_utf16(p.wstring());
				else
					s = wide_detail::to_utf16(convert(p.string()));
			}
			ar& NP("string", s);

			if (Archive::is_loading::value)
			{
				if constexpr (sizeof(wchar_t) == 2)
					p = wide_detail::from_utf16(s);
				else
					p = std::filesystem::path(convert(wide_detail::from_utf16(s)));
			}
		}




		template<class Archive, typename clock>
		void serialize(Archive& ar, std::chrono::time_point<clock>& tp)
		{
			if constexpr (Archive::is_loading::value)
			{
				std::chrono::milliseconds::rep millis;
				ar& NVP(millis);
				tp = std::chrono::time_point<clock>(std::chrono::milliseconds(millis));
			}
			else
			{
				std::chrono::milliseconds::rep millis = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch()).count();
				ar& NVP(millis);
			}
		}


	}
}

