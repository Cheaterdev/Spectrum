#include "Parsed.h"
Parsed parse(std::wstring filename);

// Parses in-memory text (an unsaved editor buffer); `file` is only used for
// diagnostic locations.
Parsed parse_text(const std::string& text, const std::string& file);
