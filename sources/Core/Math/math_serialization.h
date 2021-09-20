namespace boost
{
    namespace serialization
    {
		template<class Archive>
		void serialize(Archive& ar, std::_Big_uint128& a, const unsigned int)
		{
            ar& boost::serialization::make_array((char*)(void*)(&a), sizeof(a));
		}
    } // namespace serialization
} // namespace boost
