#pragma once
// Serialization helpers for Core and all dependent projects.
// Included by Core/Defines.h via forced include.

// ---- Named-value-pair helpers ---
#ifndef SER_STR
#  define SER_STR(x)  #x
#  define NP(name, param)  cereal::make_nvp(name, param)
#  define NVP(name)        NP(SER_STR(name), name)
#  define NVPG(name)       NP(SER_STR(name), g.name)
#endif

// ---- Base-class serialization ---
#ifndef SAVE_PARENT
#  define SAVE_PARENT(type)         ar & NP("parent_" SER_STR(type), cereal::base_class<type>(this))
#  define SAVE_VPARENT(type)        ar & NP("parent_" SER_STR(type), cereal::virtual_base_class<type>(this))
#  define SAVE_PARENT_MERGED(type)  type::serialize(ar);
#endif

// ---- Boilerplate injectors ---
#ifndef SERIALIZE
#  define SERIALIZE() \
       friend class cereal::access; \
       template<class Archive> void serialize(Archive& ar)
#  define SERIALIZE_PRETTY() \
       friend class cereal::access; \
       template<PrettyArchive Archive> void serialize(Archive& ar)
#endif

// ---- Compile-time archive branch guards ---
#ifndef IF_LOAD
#  define IF_LOAD()  if constexpr(Archive::is_loading::value)
#  define IF_SAVE()  if constexpr(Archive::is_saving::value)
#endif

// ---- Polymorphic type registration (custom, shadows cereal's own versions) ---
// Pinned to the vcpkg cereal version used when this engine was built.
// Re-verify compatibility if cereal is updated.

#ifndef CEREAL_BIND_TO_ARCHIVES
#  define CEREAL_BIND_TO_ARCHIVES(...)                                         \
       namespace cereal { namespace detail {                                   \
       template<>                                                              \
       struct init_binding<__VA_ARGS__> {                                      \
           static bind_to_archives<__VA_ARGS__> const & b;                    \
       };                                                                      \
       inline bind_to_archives<__VA_ARGS__> const &                           \
           init_binding<__VA_ARGS__>::b =                                      \
           ::cereal::detail::StaticObject<                                     \
               bind_to_archives<__VA_ARGS__>                                   \
           >::getInstance().bind();                                            \
       }} /* end namespaces */
#endif

#ifndef CEREAL_REGISTER_TYPE
#  define CEREAL_REGISTER_TYPE(...)                                            \
       namespace cereal { namespace detail {                                   \
       template <>                                                             \
       struct binding_name<__VA_ARGS__> {                                      \
           static constexpr char const * name() { return #__VA_ARGS__; }      \
       };                                                                      \
       } } /* end namespaces */                                                \
       CEREAL_BIND_TO_ARCHIVES(__VA_ARGS__)
#  define REGISTER_TYPE(...)  CEREAL_REGISTER_TYPE(__VA_ARGS__)
#endif

// ---- CEREAL_FORCE_REGISTER ---
// Forces cereal polymorphic binding-creator singletons to initialise in any
// linked TU.  Place alongside REGISTER_TYPE in each type's .cpp.
#ifndef CEREAL_CONCAT_
#  define CEREAL_CONCAT_(a, b)  a##b
#  define CEREAL_CONCAT(a, b)   CEREAL_CONCAT_(a, b)
#endif

#ifndef CEREAL_FORCE_REGISTER
#  define CEREAL_FORCE_REGISTER(...)                                                          \
       namespace {                                                                            \
           [[maybe_unused]] auto const & CEREAL_CONCAT(_freg_in_,  __COUNTER__) =            \
               cereal::detail::StaticObject<                                                  \
                   cereal::detail::InputBindingCreator<                                       \
                       cereal::BinaryInputArchive, __VA_ARGS__>                               \
               >::getInstance();                                                              \
           [[maybe_unused]] auto const & CEREAL_CONCAT(_freg_out_, __COUNTER__) =            \
               cereal::detail::StaticObject<                                                  \
                   cereal::detail::OutputBindingCreator<                                      \
                       cereal::BinaryOutputArchive, __VA_ARGS__>                              \
               >::getInstance();                                                              \
       }
#endif

// ---- Force-link support across static lib boundaries ---
// CEREAL_REGISTER_DYNAMIC_INIT : place alongside REGISTER_TYPE in the owning .cpp.
// CEREAL_FORCE_DYNAMIC_INIT    : place in any always-linked TU to ODR-use the symbol.
#ifndef CEREAL_REGISTER_DYNAMIC_INIT
#  define CEREAL_REGISTER_DYNAMIC_INIT(LibName)  \
       namespace cereal { namespace detail {      \
         void dynamic_init_dummy_##LibName() {}   \
       } }
#endif

#ifndef CEREAL_FORCE_DYNAMIC_INIT
#  define CEREAL_FORCE_DYNAMIC_INIT(LibName)                  \
       namespace cereal { namespace detail {                   \
         void dynamic_init_dummy_##LibName();                  \
       } }                                                     \
       namespace {                                             \
         struct dynamic_init_##LibName {                       \
           dynamic_init_##LibName() {                          \
             ::cereal::detail::dynamic_init_dummy_##LibName(); \
           }                                                   \
         } dynamic_init_instance_##LibName;                    \
       }
#endif
