#include <cereal/cereal.hpp>
#include <cereal/types/polymorphic.hpp>

#include <cereal/types/map.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/complex.hpp>
#include <cereal/types/set.hpp>
#include <cereal/types/array.hpp>

#include <cereal/archives/binary.hpp>

#define FORCE_DYNAMIC_INIT(LibName)                 \
  namespace cereal {                                       \
  namespace detail {                                       \
    void CEREAL_DLL_EXPORT dynamic_init_dummy_##LibName(); \
  } /* end detail */                                       \
  } /* end cereal */                                       \
                                              \
    struct dynamic_init_##LibName {                        \
      dynamic_init_##LibName() {                           \
        ::cereal::detail::dynamic_init_dummy_##LibName();  \
      }                                                    \
    } dynamic_init_instance_##LibName;


