#include <iostream>
#include <vector>
#include <sstream>
#include <string>
#include <iomanip>
#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>
export module guid;

import Singletons;
import Logs;

#define NVP(name) boost::serialization::make_nvp(BOOST_PP_STRINGIZE(name), name)

export
{


    namespace Spectrum
    {

        // Class to represent a GUID/UUID. Each instance acts as a wrapper around a
        // 16 byte value that can be passed around by value. It also supports
        // conversion to string (via the stream operator <<) and conversion from a
        // string via constructor.
        class Guid
        {
        public:

            // create a guid from vector of bytes
            Guid(const std::vector<unsigned char>& bytes);
            Guid(std::initializer_list<unsigned char>list);

            // create a guid from array of bytes
            Guid(const unsigned char* bytes);

            // create a guid from string
            Guid(const std::string& fromString);

            // create empty guid
            Guid();

            // copy constructor
            Guid(const Guid& other);

            // overload assignment operator
            Guid& operator=(const Guid& other);

            // overload equality and inequality operator
            bool operator==(const Guid& other) const;
            bool operator!=(const Guid& other) const;


            bool operator<(const Guid& r) const
            {
                if (_bytes.size() != r._bytes.size())
                    return _bytes.size() < r._bytes.size();

                for (unsigned int i = 0; i < _bytes.size(); i++)
                {
                    if (_bytes[i] != r._bytes[i])
                        return _bytes[i] < r._bytes[i];
                }

                return false;
            }
            bool is_good()
            {
                return !_bytes.empty();
            }
        private:

            // actual data
            std::vector<unsigned char> _bytes;

            // make the << operator a friend so it can access _bytes
            friend std::ostream& operator<<(std::ostream& s, const Guid& guid);
            friend std::wostream& operator<<(std::wostream& s, const Guid& guid);

            friend class boost::serialization::access;

            template<class Archive>
            void serialize(Archive& ar, const unsigned int)
            {
                ar& NVP(_bytes);
            }

        };


        template <>
        struct need_log_serialize<Guid>
        {
            static const bool value = false;
        };
    }


    // Class that can create new guids. The only reason this exists instead of
    // just a global "newGuid" function is because some platforms will require
    // that there is some attached context. In the case of android, we need to
    // know what JNIEnv is being used to call back to Java, but the newGuid()
    // function would no longer be cross-platform if we parameterized the android
    // version. Instead, construction of the GuidGenerator may be different on
    // each platform, but the use of newGuid is uniform.
    class GuidGenerator : public Singleton<GuidGenerator>
    {
    public:
        GuidGenerator() { }


        Spectrum::Guid newGuid();

    };

}

