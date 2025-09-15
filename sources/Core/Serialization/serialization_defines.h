#define SER_STR(x) #x

#define NP(name, param) cereal::make_nvp(name, param)
#define NVP(name) NP(SER_STR(name), name)
#define NVPG(name) NP(SER_STR(name), g.name)

#define SAVE_PARENT(type)  ar&NP("parent_" SER_STR(type), cereal::base_class<type>(this))
#define SAVE_VPARENT(type)  ar&NP("parent_" SER_STR(type), cereal::virtual_base_class<type>(this))

#define SAVE_PARENT_MERGED(type)  type::serialize(ar);

#define SERIALIZE()  friend class cereal::access; template<class Archive> void serialize(Archive& ar)
#define SERIALIZE_PRETTY()  friend class cereal::access; template<PrettyArchive Archive> void serialize(Archive& ar)

				

#define CEREAL_BIND_TO_ARCHIVES(...)                                     \
    namespace cereal {                                                   \
    namespace detail {                                                   \
    template<>                                                           \
    struct init_binding<__VA_ARGS__> {                                   \
        static inline bind_to_archives<__VA_ARGS__> const & b=           \
        ::cereal::detail::StaticObject<                                  \
            bind_to_archives<__VA_ARGS__>                                \
        >::getInstance().bind();                                         \
    };                                                                   \
    }} /* end namespaces */

#define CEREAL_REGISTER_TYPE(...)                                        \
  namespace cereal {                                                     \
  namespace detail {                                                     \
  template <>                                                            \
  struct binding_name<__VA_ARGS__>                                       \
  {                                                                      \
    static constexpr char const * name() { return #__VA_ARGS__; } \
  };                                                                     \
  } } /* end namespaces */                                               \
  CEREAL_BIND_TO_ARCHIVES(__VA_ARGS__)

#define REGISTER_TYPE(...) CEREAL_REGISTER_TYPE(__VA_ARGS__)
#define IF_LOAD() if constexpr(Archive::is_loading::value)
#define IF_SAVE() if constexpr(Archive::is_saving::value)