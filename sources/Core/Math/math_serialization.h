#include "Log/simple_archive.h"

namespace boost
{
    namespace serialization
    {

        template<class T>
        void serialize(simple_log_archive& ar, Vector<vec2_t<T>>& a, const unsigned int)
        {
            ar& NP("x", a.x)
            & NP("y", a.y);
        }

        template<class T>
        void serialize(simple_log_archive& ar, Vector<vec3_t<T>>& a, const unsigned int)
        {
            ar& NP("x", a.x)
            & NP("y", a.y)
            & NP("z", a.z);
        }

        template<class T>
        void serialize(simple_log_archive& ar, Vector<vec4_t<T>>& a, const unsigned int)
        {
            ar& NP("x", a.x)
            & NP("y", a.y)
            & NP("z", a.z)
            &NP("w", a.w);
        }

        template<class Archive, class T>
        void serialize(Archive& ar, Vector<T>& a, const unsigned int)
        {
            ar& boost::serialization::make_array(a.values.data(), T::N);
        }

        template<class Archive, class T>
        void serialize(Archive& ar, matrix<T>& a, const unsigned int)
        {
            ar& boost::serialization::make_array(a.rows[0].values.data(), T::N * T::M);
        }

        template<class Archive>
        void serialize(Archive& ar, Sphere& s, const unsigned int)
        {
            ar& NVP(s.pos)&NVP(s.radius) & NVP(boost::serialization::base_object<Primitive>(s));
        }
        template<class Archive>
        void serialize(Archive& ar, AABB& s, const unsigned int)
        {
            ar& NVP(s.min)&NVP(s.max) & NVP(boost::serialization::base_object<Primitive>(s));
        }
        template<class Archive>
        void serialize(Archive&, Primitive&, const unsigned int)
        {
        }

		template<class Archive>
		void serialize(Archive& ar, std::_Big_uint128& a, const unsigned int)
		{
            ar& boost::serialization::make_array((char*)(void*)(&a), sizeof(a));
		}
    } // namespace serialization
} // namespace boost
