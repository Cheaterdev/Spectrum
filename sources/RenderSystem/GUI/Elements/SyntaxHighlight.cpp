module GUI:SyntaxHighlight;

namespace
{
    constexpr uint32_t rgb(uint32_t r, uint32_t g, uint32_t b)
    {
        return r | (g << 8) | (b << 16) | (0xFFu << 24);
    }

    constexpr uint32_t color_keyword   = rgb(0, 0, 255);
    constexpr uint32_t color_type      = rgb(43, 145, 175);
    constexpr uint32_t color_comment   = rgb(0, 128, 0);
    constexpr uint32_t color_string    = rgb(163, 21, 21);
    constexpr uint32_t color_number    = rgb(9, 134, 88);
    constexpr uint32_t color_directive = rgb(128, 128, 128);
    constexpr uint32_t color_semantic  = rgb(0, 112, 193);
    constexpr uint32_t color_intrinsic = rgb(121, 94, 38);

    bool is_digit(char32_t c) { return c >= '0' && c <= '9'; }
    bool is_alpha(char32_t c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_'; }
    bool is_ident(char32_t c) { return is_alpha(c) || is_digit(c); }
    bool is_hex(char32_t c)   { return is_digit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'); }

    const std::unordered_set<std::string_view> keywords = {
        "if", "else", "for", "while", "do", "switch", "case", "default", "break", "continue", "return", "discard",
        "struct", "cbuffer", "tbuffer", "class", "interface", "namespace", "typedef", "template", "typename",
        "static", "const", "uniform", "volatile", "extern", "inline", "precise", "groupshared", "shared",
        "in", "out", "inout", "linear", "centroid", "nointerpolation", "noperspective", "sample",
        "register", "packoffset", "true", "false", "row_major", "column_major", "export",
    };

    const std::unordered_set<std::string_view> object_types = {
        "void", "matrix", "vector", "string", "SamplerState", "SamplerComparisonState",
        "Texture1D", "Texture1DArray", "Texture2D", "Texture2DArray", "Texture2DMS", "Texture2DMSArray",
        "Texture3D", "TextureCube", "TextureCubeArray",
        "RWTexture1D", "RWTexture1DArray", "RWTexture2D", "RWTexture2DArray", "RWTexture3D",
        "Buffer", "RWBuffer", "StructuredBuffer", "RWStructuredBuffer", "AppendStructuredBuffer",
        "ConsumeStructuredBuffer", "ByteAddressBuffer", "RWByteAddressBuffer", "ConstantBuffer",
        "RaytracingAccelerationStructure", "RayDesc", "RayQuery",
    };

    const std::unordered_set<std::string_view> intrinsics = {
        "abs", "acos", "all", "any", "asfloat", "asin", "asint", "asuint", "atan", "atan2", "ceil", "clamp",
        "clip", "cos", "cosh", "countbits", "cross", "ddx", "ddy", "degrees", "determinant", "distance",
        "dot", "exp", "exp2", "f16tof32", "f32tof16", "firstbithigh", "firstbitlow", "floor", "fmod", "frac",
        "fwidth", "isinf", "isnan", "ldexp", "length", "lerp", "log", "log2", "mad", "max", "min", "modf",
        "mul", "normalize", "pow", "radians", "rcp", "reflect", "refract", "reversebits", "round", "rsqrt",
        "saturate", "sign", "sin", "sincos", "sinh", "smoothstep", "sqrt", "step", "tan", "tanh", "transpose",
        "trunc", "Sample", "SampleLevel", "SampleGrad", "SampleBias", "SampleCmp", "SampleCmpLevelZero",
        "Load", "Store", "Gather", "GetDimensions", "InterlockedAdd", "InterlockedMax", "InterlockedMin",
        "InterlockedOr", "InterlockedAnd", "InterlockedExchange", "InterlockedCompareExchange",
        "GroupMemoryBarrierWithGroupSync", "DeviceMemoryBarrier", "AllMemoryBarrier", "NonUniformResourceIndex",
        "WaveActiveSum", "WaveActiveMax", "WaveActiveMin", "WaveReadLaneFirst", "WaveGetLaneIndex",
        "TraceRay", "ReportHit", "AcceptHitAndEndSearch", "IgnoreHit", "DispatchRaysIndex", "DispatchRaysDimensions",
    };

    // Scalar, vector and matrix types: float, float3, float4x4, uint2, min16float3 ...
    bool is_numeric_type(std::string_view w)
    {
        static constexpr std::string_view bases[] = {
            "min16float", "min16uint", "min16int", "float16_t", "float32_t", "int16_t", "uint16_t",
            "int64_t", "uint64_t", "double", "float", "half", "uint", "bool", "dword", "int",
        };

        for (auto base : bases)
        {
            if (!w.starts_with(base)) continue;

            const auto rest = w.substr(base.size());
            if (rest.empty()) return true;
            if (rest.size() == 1 && rest[0] >= '1' && rest[0] <= '4') return true;
            if (rest.size() == 3 && rest[0] >= '1' && rest[0] <= '4' && rest[1] == 'x' && rest[2] >= '1' && rest[2] <= '4') return true;
            return false;
        }
        return false;
    }
}

namespace GUI::Syntax
{
    void highlight_hlsl(std::u32string_view s, std::vector<uint32_t>& colors)
    {
        const size_t n = s.size();
        auto paint = [&](size_t begin, size_t end, uint32_t color)
            {
                for (size_t i = begin; i < end && i < colors.size(); ++i)
                    colors[i] = color;
            };

        bool   line_start = true;   // only whitespace so far on this line
        size_t i = 0;

        while (i < n)
        {
            const char32_t c = s[i];

            if (c == '\n')
            {
                line_start = true;
                ++i;
                continue;
            }
            if (c == ' ' || c == '\t' || c == '\r')
            {
                ++i;
                continue;
            }

            // Line comment.
            if (c == '/' && i + 1 < n && s[i + 1] == '/')
            {
                size_t e = i;
                while (e < n && s[e] != '\n') ++e;
                paint(i, e, color_comment);
                i = e;
                continue;
            }

            // Block comment; spans lines, so it runs over the whole text at once.
            if (c == '/' && i + 1 < n && s[i + 1] == '*')
            {
                size_t e = i + 2;
                while (e + 1 < n && !(s[e] == '*' && s[e + 1] == '/')) ++e;
                e = std::min(n, e + 2);
                paint(i, e, color_comment);
                i = e;
                line_start = false;
                continue;
            }

            // Preprocessor directive: to the end of the line or a trailing comment.
            if (c == '#' && line_start)
            {
                size_t e = i;
                while (e < n && s[e] != '\n' && !(s[e] == '/' && e + 1 < n && (s[e + 1] == '/' || s[e + 1] == '*'))) ++e;
                paint(i, e, color_directive);
                i = e;
                line_start = false;
                continue;
            }

            line_start = false;

            if (c == '"')
            {
                size_t e = i + 1;
                while (e < n && s[e] != '"' && s[e] != '\n')
                    e += (s[e] == '\\') ? 2 : 1;
                if (e < n && s[e] == '"') ++e;
                e = std::min(e, n);
                paint(i, e, color_string);
                i = e;
                continue;
            }

            if (is_digit(c) || (c == '.' && i + 1 < n && is_digit(s[i + 1])))
            {
                size_t e = i;
                if (c == '0' && i + 1 < n && (s[i + 1] == 'x' || s[i + 1] == 'X'))
                {
                    e += 2;
                    while (e < n && is_hex(s[e])) ++e;
                }
                else
                {
                    while (e < n && (is_digit(s[e]) || s[e] == '.')) ++e;
                    if (e < n && (s[e] == 'e' || s[e] == 'E'))
                    {
                        ++e;
                        if (e < n && (s[e] == '+' || s[e] == '-')) ++e;
                        while (e < n && is_digit(s[e])) ++e;
                    }
                }
                while (e < n && is_alpha(s[e])) ++e;   // f, h, u, l suffixes
                paint(i, e, color_number);
                i = e;
                continue;
            }

            if (is_alpha(c))
            {
                size_t e = i;
                while (e < n && is_ident(s[e])) ++e;

                std::string word;
                word.reserve(e - i);
                for (size_t k = i; k < e; ++k)
                    word.push_back(char(s[k]));

                // A semantic is an identifier right after ':' (skipping spaces).
                size_t p = i;
                while (p > 0 && (s[p - 1] == ' ' || s[p - 1] == '\t')) --p;
                const bool after_colon = p > 0 && s[p - 1] == ':' && !(p > 1 && s[p - 2] == ':');

                uint32_t color = 0;
                if (keywords.contains(word))                                  color = color_keyword;
                else if (is_numeric_type(word) || object_types.contains(word)) color = color_type;
                else if (after_colon && (word.starts_with("SV_") || std::all_of(word.begin(), word.end(),
                         [](char ch) { return !(ch >= 'a' && ch <= 'z'); })))  color = color_semantic;
                else if (intrinsics.contains(word))                            color = color_intrinsic;

                if (color)
                    paint(i, e, color);
                i = e;
                continue;
            }

            ++i;
        }
    }
}
