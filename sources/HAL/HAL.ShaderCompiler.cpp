module HAL:ShaderCompiler;

import Core;

namespace HAL
{
	void DebugLogStrings::register_format(uint32_t id, std::string fmt)
	{
		std::lock_guard<std::mutex> g(m);
		strings.try_emplace(id, std::move(fmt));
	}

	std::string DebugLogStrings::format(uint32_t id, uint4 args) const
	{
		std::string fmt;
		{
			std::lock_guard<std::mutex> g(m);
			auto it = strings.find(id);
			if (it == strings.end())
				return std::format("<unregistered log id {:#x}> {} {} {} {}", id, args.x, args.y, args.z, args.w);
			fmt = it->second;
		}

		uint32_t raw[4] = { args.x, args.y, args.z, args.w };
		int arg_index = 0;
		std::string out;
		out.reserve(fmt.size());

		for (size_t i = 0; i < fmt.size(); ++i)
		{
			if (fmt[i] != '%' || i + 1 >= fmt.size())
			{
				out += fmt[i];
				continue;
			}

			char spec = fmt[++i];
			uint32_t value = arg_index < 4 ? raw[arg_index] : 0;

			switch (spec)
			{
			case 'u': out += std::to_string(value); ++arg_index; break;
			case 'd': out += std::to_string(static_cast<int32_t>(value)); ++arg_index; break;
			case 'f':
			{
				float f;
				std::memcpy(&f, &value, sizeof(f));
				out += std::to_string(f);
				++arg_index;
				break;
			}
			case 'x': out += std::format("{:x}", value); ++arg_index; break;
			case '%': out += '%'; break;
			default:  out += '%'; out += spec; break; // unrecognized spec -- pass through literally
			}
		}

		return out;
	}
}
