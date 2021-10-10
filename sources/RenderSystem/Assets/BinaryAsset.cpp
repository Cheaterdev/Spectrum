#include "pch_render.h"
#include "BinaryAsset.h"

BOOST_CLASS_EXPORT(BinaryAsset);
BOOST_CLASS_EXPORT_IMPLEMENT(AssetReference<BinaryAsset>);


template void AssetReference<BinaryAsset>::serialize(serialization_oarchive& arch, const unsigned int version);
template void AssetReference<BinaryAsset>::serialize(serialization_iarchive& arch, const unsigned int version);

//////////////////////////////////////////////////////////////////////////
Asset_Type BinaryAsset::get_type()
{
    return Asset_Type::BINARY;
}

std::string BinaryAsset::get_data()
{
    return data;
}

BinaryAsset::BinaryAsset(std::wstring file_name)
{
	tester = std::make_shared<bool>();
    auto file = FileSystem::get().get_file(file_name);

    if (file)
    {
        data = file->load_all();
        file_depends.add_depend(file);
    }

    this->file_name = file_name;
    name = file_name.substr(file_name.find_last_of(L"\\") + 1);
    mark_changed();
    FileSystem::get().get_provider<native_file_provider>()->on_change(file_name, [this]()
    {
        reload_resource();
    });
}
void BinaryAsset::try_register()
{
    if (data.size())
        Asset::try_register();
}

 void BinaryAsset::reload_resource()
{
	Log::get() << "reloading " << file_name << Log::endl;

	if (!file_depends.need_update())
		return;

	file_depends.clear();
	auto file = FileSystem::get().get_file(file_name);

	if (file)
	{
		data = file->load_all();
		file_depends.add_depend(file);
		mark_changed();
	}

	mark_contents_changed();
}

BinaryAsset::BinaryAsset()
{
	tester = std::make_shared<bool>();

}

void BinaryAsset::update_preview(Render::Texture::ptr preview)
{
    /* Render::CommandList::ptr list(new Render::CommandList(Render::CommandListType::DIRECT));
     list->begin();
     TextureAssetRenderer::get().render(this, preview, list);
     list->end();
     list->execute();
     //
     //   c.generate_mipmaps(preview->get_shader_view());
     mark_changed();*/
}

