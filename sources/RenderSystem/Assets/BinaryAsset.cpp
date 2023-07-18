module Graphics:BinaryAsset;

import <RenderSystem.h>;

import :MipMapGenerator;
import TextSystem;
import Core;
void removeme2() // TODO: VS issue - make dummy unused func to compile entire cpp =[
{
	auto res = Serializer::deserialize<std::string>("");
}
REGISTER_TYPE(BinaryAsset);

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
	auto file = FileSystem::get().get_file(to_path(file_name));

	if (file)
	{
		data = file->load_all();
		file_depends.add_depend(file);
	}

	this->file_name = file_name;
	name = file_name.substr(file_name.find_last_of(L"\\") + 1);
	mark_changed();
	FileSystem::get().get_provider<native_file_provider>()->on_change(to_path(file_name), [this]()
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
	auto file = FileSystem::get().get_file(to_path(file_name));

	if (file)
	{
		data = file->load_all();
		file_depends.add_depend((file));
		mark_changed();
	}

	mark_contents_changed();
}

BinaryAsset::BinaryAsset()
{
	tester = std::make_shared<bool>();

}

void BinaryAsset::update_preview(HAL::Texture::ptr preview)
{
//	if (!preview || !preview->is_rt())
//		preview.reset(new HAL::Texture(HAL::ResourceDesc::Tex2D(HAL::Format::R8G8B8A8_UNORM, { 256, 256 }, 1, 6, HAL::ResFlags::ShaderResource | HAL::ResFlags::RenderTarget | HAL::ResFlags::UnorderedAccess)));
//
//	auto list = (HAL::Device::get().get_frame_manager().begin_frame()->start_list("BinaryAsset"));
//
//	auto font = Fonts::FontSystem::get().get_font("Segoe UI Light");
//	auto 	geomerty = std::make_shared<Fonts::FontGeometry>();
//	sizer layout = { 0,0,256,256 };
//
//	geomerty->set(list, convert(data), font, 14, layout, float4(1, 1, 1, 1), FW1_LEFT);
//
//
//	{
//				RT::SingleColor rt;
//				rt.GetColor() =preview->texture_2d().renderTarget;
//				rt.set(list->get_graphics(), HAL::RTOptions::Default| HAL::RTOptions::ClearColor);
//	}
//
//
//
//	geomerty->draw(list, layout, 0, { 0,0 });
//
//
//	MipMapGenerator::get().generate(list->get_compute(), preview->texture_2d());
//	//TextureAssetRenderer::get().render(this, preview, list);
//	list->execute();
//	//
////   c.generate_mipmaps(preview->get_shader_view());
//	mark_changed();
}

