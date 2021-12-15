#define SER_STR(x) #x

#define NP(name, param) cereal::make_nvp(name, param)
#define NVP(name) NP(SER_STR(name), name)

#define SAVE_PARENT(type)  ar&NP("parent_" SER_STR(type), cereal::base_class<type>(this))
#define SAVE_VPARENT(type)  ar&NP("parent_" SER_STR(type), cereal::virtual_base_class<type>(this))

#define SERIALIZE()  friend class cereal::access; template<class Archive> void serialize(Archive& ar, const unsigned int version)
#define SERIALIZE_PRETTY()  friend class cereal::access; template<PrettyArchive Archive> void serialize(Archive& ar, const unsigned int version)


//! Helper macro to omit unused warning
//#if defined(__GNUC__)
  // GCC / clang don't want the function
//#define CEREAL_BIND_TO_ARCHIVES_UNUSED_FUNCTION
//#else
//#define CEREAL_BIND_TO_ARCHIVES_UNUSED_FUNCTION static void unused();// { (void)b; }
//#endif

//! Binds a polymorhic type to all registered archives
/*! This binds a polymorphic type to all compatible registered archives that
    have been registered with CEREAL_REGISTER_ARCHIVE.  This must be called
    after all archives are registered (usually after the archives themselves
    have been included). */
#define BIND_TO_ARCHIVES(...)                                     \
    namespace cereal {                                                   \
    namespace detail {                                                   \
    template<>                                                           \
    struct init_binding<__VA_ARGS__> {                                   \
        static bind_to_archives<__VA_ARGS__> const & b;                  \
    };             }}                                                   

#define BIND_TO_ARCHIVES_IMPL(...)                                     \
      namespace cereal {                                                   \
    namespace detail {     bind_to_archives<__VA_ARGS__> const & init_binding<__VA_ARGS__>::b = \
        ::cereal::detail::StaticObject<                                  \
            bind_to_archives<__VA_ARGS__>                                \
        >::getInstance().bind();                                         \
    }} /* end namespaces */



#define REGISTER_TYPE_KEY(...)                                        \
  namespace cereal {                                                     \
  namespace detail {                                                     \
  template <>                                                            \
  struct binding_name<__VA_ARGS__>                                       \
  {                                                                      \
    static char const * name() ; \
  };                                                                     \
  } } /* end namespaces */                                               \
  BIND_TO_ARCHIVES(__VA_ARGS__) 

#define REGISTER_TYPE_IMPLEMENT(...)                                        \
  namespace cereal {                                                     \
  namespace detail {                                                     \
     char const * binding_name<__VA_ARGS__>::name() { return #__VA_ARGS__; } \
  } } /* end namespaces */                                               \
BIND_TO_ARCHIVES_IMPL(__VA_ARGS__)


#define CEREAL_BIND_TO_ARCHIVES2(...)                                     \
    namespace cereal {                                                   \
    namespace detail {                                                   \
    template<>                                                           \
    struct init_binding<__VA_ARGS__> {                                   \
        static bind_to_archives<__VA_ARGS__> const & b;                  \
                       \
    };     }}        



#define CEREAL_BIND_TO_ARCHIVES3(...)                \
namespace cereal {\
        namespace detail {\
    bind_to_archives<__VA_ARGS__> const & init_binding<__VA_ARGS__>::b = \
        ::cereal::detail::StaticObject<                                  \
            bind_to_archives<__VA_ARGS__>                                \
        >::getInstance().bind();                                         \
    }} /* end namespaces */


#define CEREAL_REGISTER_TYPE2(...)                                        \
  namespace cereal {                                                     \
  namespace detail {                                                     \
  template <>                                                            \
  struct binding_name<__VA_ARGS__>                                       \
  {                                                                      \
    static constexpr char const * name() { return #__VA_ARGS__; } \
  };                                                                     \
  } } /* end namespaces */                                               \
  CEREAL_BIND_TO_ARCHIVES2(__VA_ARGS__)