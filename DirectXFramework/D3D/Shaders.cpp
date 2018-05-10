#include "pch.h"

namespace D3D
{
    bool operator<(const shader_header& l, const shader_header& r)
    {
        if (l.file_name != r.file_name)
            return l.file_name < r.file_name;
        else
        {
            if (l.entry_point != r.entry_point)
                return l.entry_point < r.entry_point;
        }

		if (l.macros.size() == r.macros.size())
		{
			for (int i = 0; i < l.macros.size(); i++)
			{
				auto ll = l.macros[i];
				auto rr = r.macros[i];

				if (ll.name == rr.name)
				{
					if (ll.value == rr.value)
					{
						
					}
					else
						return	ll.value < rr.value;


				}else
					return	ll.name < rr.name;
			}
		}
        return l.macros.size() < r.macros.size();
    }


    shader_macro::shader_macro(std::string name, std::string value /*= "1"*/)
    {
        this->name = name;
        this->value = value;
    }




    STDMETHODIMP shader_include::Close(LPCVOID pData)
    {
        return S_OK;
    }

    STDMETHODIMP shader_include::Open(D3D10_INCLUDE_TYPE includeType, LPCSTR pFileName, LPCVOID /*pParentData*/, LPCVOID* ppData, UINT* pBytes)
    {
        std::string file_name = dir + pFileName;
        auto file = FileSystem::get().get_file(convert(file_name));

        if (!file)
        {
            //Make a message about not finding an included file.
            return E_FAIL;
        }

        data = file->load_all();
        depender.add_depend(file);
        *ppData = reinterpret_cast<const void*>(data.data());
        *pBytes = static_cast<unsigned int>(data.size());
        return S_OK;
    }

    shader_include::shader_include(std::string dir, resource_file_depender& _depender) : depender(_depender)
    {

		std::transform(dir.begin(), dir.end(), dir.begin(), []( char s) {
			if (s == '/') return '\\';
			return s;
		});
        this->dir = dir.substr(0, dir.find_last_of('\\')) + '\\';
    }

}