typedef ID3D11Resource* native_resource;

class Resource
{
public:
	virtual native_resource get_native() = 0;
};

