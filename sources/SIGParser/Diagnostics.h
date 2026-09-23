#pragma once
#include "Parsed.h"

// Errors collected across the whole run. Nothing is written to autogen/ unless
// this is empty at the end of parsing + validation: a partially regenerated
// tree is internally inconsistent and worse than a stale one.
class Diagnostics
{
public:
	struct Entry
	{
		SourceLocation loc;
		std::string message;
	};

private:
	std::vector<Entry> errors;

public:
	const std::vector<Entry>& entries() const
	{
		return errors;
	}

	// The language server revalidates in one long-lived process.
	void clear()
	{
		errors.clear();
	}

	void error(const SourceLocation& loc, std::string message)
	{
		errors.push_back({ loc, std::move(message) });
	}

	void error(const parsed_type& at, std::string message)
	{
		error(at.loc, std::move(message));
	}

	bool empty() const
	{
		return errors.empty();
	}

	size_t count() const
	{
		return errors.size();
	}

	// MSVC's file(line,col) shape, so the VS output window can jump to it.
	void print() const
	{
		for (const auto& e : errors)
		{
			if (e.loc.file.empty())
				std::cout << "error: " << e.message << "\n";
			else
				std::cout << e.loc.file << "(" << e.loc.line << "," << e.loc.column << "): error: " << e.message << "\n";
		}
		std::cout << std::flush;
	}
};

inline Diagnostics& diagnostics()
{
	static Diagnostics d;
	return d;
}
