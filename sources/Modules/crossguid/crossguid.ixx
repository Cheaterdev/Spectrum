export module crossguid;

export import "crossguid.h";


export using Guid = xg::Guid;

#ifdef __ANDROID__
extern "C" long getrandom(void* buffer, unsigned long length, unsigned int flags);   // bionic, API 28+
#endif

export namespace xg
{
	// xg::newGuid() on Android goes through JNI (java.util.UUID) and needs a JNIEnv this engine never
	// hands over; there it is generated from getrandom() instead (a version 4 UUID).
	inline Guid new_guid()
	{
#ifdef __ANDROID__
		std::array<unsigned char, 16> bytes{};
		getrandom(bytes.data(), bytes.size(), 0);
		bytes[6] = (bytes[6] & 0x0F) | 0x40;
		bytes[8] = (bytes[8] & 0x3F) | 0x80;
		return Guid(bytes);
#else
		return newGuid();
#endif
	}
}
