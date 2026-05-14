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
        static bind_to_archives<__VA_ARGS__> const & b;                  \
    };                                                                   \
    inline bind_to_archives<__VA_ARGS__> const &                         \
        init_binding<__VA_ARGS__>::b =                                   \
        ::cereal::detail::StaticObject<                                  \
            bind_to_archives<__VA_ARGS__>                                \
        >::getInstance().bind();                                         \
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

// Forces cereal polymorphic binding-creator singletons to initialise in any linked TU.
// Put this alongside REGISTER_TYPE in each type's .cpp so the InputBindingCreator /
// OutputBindingCreator constructors run at startup regardless of static-lib linker quirks.
#ifndef CEREAL_CONCAT_
#define CEREAL_CONCAT_(a, b) a##b
#define CEREAL_CONCAT(a, b) CEREAL_CONCAT_(a, b)
#endif
#define CEREAL_FORCE_REGISTER(...)                                                            \
    namespace {                                                                               \
        [[maybe_unused]] auto const & CEREAL_CONCAT(_freg_in_,  __COUNTER__) =              \
            cereal::detail::StaticObject<                                                     \
                cereal::detail::InputBindingCreator<                                          \
                    cereal::BinaryInputArchive, __VA_ARGS__>                                  \
            >::getInstance();                                                                  \
        [[maybe_unused]] auto const & CEREAL_CONCAT(_freg_out_, __COUNTER__) =              \
            cereal::detail::StaticObject<                                                     \
                cereal::detail::OutputBindingCreator<                                         \
                    cereal::BinaryOutputArchive, __VA_ARGS__>                                 \
            >::getInstance();                                                                  \
    }

#define IF_LOAD() if constexpr(Archive::is_loading::value)
#define IF_SAVE() if constexpr(Archive::is_saving::value)

// Force-link support for cereal polymorphic registration across static lib boundaries.
// CEREAL_REGISTER_DYNAMIC_INIT goes alongside REGISTER_TYPE in the .cpp that owns the type.
// CEREAL_FORCE_DYNAMIC_INIT goes in any always-linked TU to ODR-use the .cpp's dummy symbol.
#ifndef CEREAL_REGISTER_DYNAMIC_INIT
#define CEREAL_REGISTER_DYNAMIC_INIT(LibName)  \
  namespace cereal { namespace detail {        \
    void dynamic_init_dummy_##LibName() {}     \
  } }
#endif

#ifndef CEREAL_FORCE_DYNAMIC_INIT
#define CEREAL_FORCE_DYNAMIC_INIT(LibName)                 \
  namespace cereal { namespace detail {                    \
    void dynamic_init_dummy_##LibName();                   \
  } }                                                      \
  namespace {                                              \
    struct dynamic_init_##LibName {                        \
      dynamic_init_##LibName() {                           \
        ::cereal::detail::dynamic_init_dummy_##LibName();  \
      }                                                    \
    } dynamic_init_instance_##LibName;                     \
  }
#endif