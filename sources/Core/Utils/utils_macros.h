

#define STRINGIZE(x) #x

#define SPECTRUM_MERGE_(a,b)  a##b
#define SPECTRUM_LABEL_(a) SPECTRUM_MERGE_(__timer__, a)
#define SPECTRUM_UNIQUE_NAME SPECTRUM_LABEL_(__LINE__)

#define CACHE_ALIGN(x) __declspec(align(x))

#define GEN_DEF_COMP(x) \
	bool operator==(const x& r) const = default;\
	std::strong_ordering  operator<=>(const  x& r) const = default;
#define GENERATE_OPS __GENERATE_OPS__
