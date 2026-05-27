#pragma once
// ---- MSVC module COMDAT-key workaround (must come first) --------------------
// polymorphic_impl.hpp contains non-template inline functions (e.g.
// PolymorphicCasters::lookup_if_exists) that call StaticObject<T>::getInstance()
// in their bodies.  MSVC eagerly instantiates those calls when compiling this
// header unit, so an explicit specialisation declaration must appear BEFORE that
// header is included — otherwise C2908 fires ("already instantiated").
//
// Strategy:
//   1. Pull in static_object.hpp (defines StaticObject<T>) and
//      polymorphic_impl_fwd.hpp (forward-declares PolymorphicCasters) so the
//      template argument types are known without a complete definition.
//   2. Pull in binary.hpp (defines BinaryInputArchive / BinaryOutputArchive).
//      binary.hpp does NOT include types/polymorphic.hpp, so no instantiation
//      of the three singletons occurs here.
//   3. Forward-declare InputBindingMap / OutputBindingMap.
//   4. Declare the three explicit specialisations.  Every TU that sees these
//      declarations emits an external call instead of an inline template
//      instantiation — the definitions live in main.cpp (non-module TU, so they
//      get standard global-module COMDAT keys that link correctly from all
//      named modules).
//
// All subsequent includes of the real polymorphic headers will see the
// specialisation declarations in scope and will not produce implicit
// instantiations.
// -----------------------------------------------------------------------------
#include <cereal/details/static_object.hpp>
#include <cereal/details/polymorphic_impl_fwd.hpp>
#include <cereal/archives/binary.hpp>

namespace cereal { namespace detail {
    template <class Archive> struct InputBindingMap;
    template <class Archive> struct OutputBindingMap;
} }

namespace cereal {
  namespace detail {
    template<> CEREAL_DLL_EXPORT PolymorphicCasters&
        StaticObject<PolymorphicCasters>::getInstance();
    template<> CEREAL_DLL_EXPORT InputBindingMap<BinaryInputArchive>&
        StaticObject<InputBindingMap<BinaryInputArchive>>::getInstance();
    template<> CEREAL_DLL_EXPORT OutputBindingMap<BinaryOutputArchive>&
        StaticObject<OutputBindingMap<BinaryOutputArchive>>::getInstance();
  }
}
// ---- end workaround ---------------------------------------------------------

#include <cereal/cereal.hpp>
#include <cereal/types/polymorphic.hpp>

#include <cereal/types/map.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/complex.hpp>
#include <cereal/types/set.hpp>
#include <cereal/types/array.hpp>
#include <cereal/types/variant.hpp>
#include <cereal/types/optional.hpp>
#include <cereal/types/list.hpp>

//#include <cereal/archives/json.hpp>

// adapters.hpp requires CEREAL_FUTURE_EXPERIMENTAL (local to this wrapper).
#ifndef CEREAL_FUTURE_EXPERIMENTAL
#  define CEREAL_FUTURE_EXPERIMENTAL
#endif
#include <cereal/archives/adapters.hpp>

#include <cereal/details/helpers.hpp>
