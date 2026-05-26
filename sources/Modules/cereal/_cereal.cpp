// No "module" declaration — this TU belongs to the global module.
// That gives the three getInstance() specialisations below standard COMDAT
// keys visible from every named module, so all cereal binding maps and
// PolymorphicCasters always resolve to one shared instance regardless of
// which module calls getInstance().
#include "_cereal.h"

namespace cereal { namespace detail {

    template<>
    PolymorphicCasters& StaticObject<PolymorphicCasters>::getInstance()
    {
        static PolymorphicCasters t;
        return t;
    }

    template<>
    InputBindingMap<BinaryInputArchive>&
        StaticObject<InputBindingMap<BinaryInputArchive>>::getInstance()
    {
        static InputBindingMap<BinaryInputArchive> t;
        return t;
    }

    template<>
    OutputBindingMap<BinaryOutputArchive>&
        StaticObject<OutputBindingMap<BinaryOutputArchive>>::getInstance()
    {
        static OutputBindingMap<BinaryOutputArchive> t;
        return t;
    }

} }
