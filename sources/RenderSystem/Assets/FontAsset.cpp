module Graphics:FontAsset;
import RenderSystem;

import :MipMapGenerator;
import TextEngine;
import Core;

REGISTER_TYPE(FontAsset);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Asset, FontAsset);
CEREAL_REGISTER_DYNAMIC_INIT(FontAsset);
CEREAL_FORCE_REGISTER(FontAsset);
CEREAL_FORCE_REGISTER_RELATION(Asset, FontAsset);

Asset_Type FontAsset::get_type()
{
	return Asset_Type::FONT;
}

FontAsset::FontAsset()
{
	tester = std::make_shared<bool>();
}

FontAsset::FontAsset(std::wstring file_name)
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

	std::weak_ptr<bool> r = tester;
	FileSystem::get().get_provider<native_file_provider>()->on_change(to_path(file_name), [this, r]()
	{
		if (r.lock())
			reload_resource();
	});
}

FontAsset::~FontAsset()
{
	// The engine goes before the asset manager at shutdown and takes its fonts with it.
	if (Text::Engine::is_good())
		release_font();
}

void FontAsset::try_register()
{
	if (data.size())
		Asset::try_register();
}

void FontAsset::release_font()
{
	if (font)
		Text::Engine::get().remove_font(font);

	font = 0;
}

Text::FontId FontAsset::get_font()
{
	if (!font)
		font = Text::Engine::get().add_font(name.empty() ? "font" : convert(name), data);

	return font;
}

void FontAsset::reload_resource()
{
	Log::get() << "reloading " << file_name << Log::endl;

	if (!file_depends.need_update())
		return;

	file_depends.clear();
	auto file = FileSystem::get().get_file(to_path(file_name));

	if (file)
	{
		data = file->load_all();
		file_depends.add_depend(file);
		mark_changed();
	}

	release_font();          // re-added from the new data on the next get_font()
	mark_contents_changed(); // bumps version + notifies consumers
}

bool FontAsset::is_emoji()
{
	return Text::Engine::get().has_glyph(get_font(), 0x1F600);
}

const std::vector<FontAsset::Sample>& FontAsset::samples()
{
	// Emoji reach the engine as UTF-8; \U escapes keep the source independent of its encoding.
	static const auto utf8 = [](const char8_t* s) { return std::string(reinterpret_cast<const char*>(s)); };
	static const std::vector<Sample> emoji = {
		{ utf8(u8"\U0001F600\U0001F60E\U0001F680\U0001F525"), 56, 40 },
		{ utf8(u8"\U0001F431\U0001F436\U0001F98A\U0001F43C"), 56, 40 },
		{ utf8(u8"\U0001F44D\U0001F44F\U0001F64F\U0001F4AA"), 56, 40 },
		{ utf8(u8"\U0001F34E\U0001F355\U0001F354☕\U0001F389"), 40, 0 },
		// Sequences: a ZWJ family, a rainbow flag, two country flags, a heart with a variation selector.
		{ utf8(u8"\U0001F468‍\U0001F469‍\U0001F467 \U0001F3F3️‍\U0001F308 \U0001F1FA\U0001F1F8 \U0001F1EF\U0001F1F5 ❤️"), 40, 0 },
	};

	if (is_emoji())
		return emoji;

	static const std::vector<Sample> list = {
		{ "Aa Gg Rr",                                      72, 56 },
		{ "ABCDEFGHIJKLMNOPQRSTUVWXYZ",                    28, 11 },
		{ "abcdefghijklmnopqrstuvwxyz",                    28, 11 },
		{ "0123456789  !?@#$%&*()[]{}<>",                  28, 11 },
		{ "The quick brown fox jumps over the lazy dog",   22, 11 },
		{ "Sphinx of black quartz, judge my vow",          18, 0 },
		{ "Pack my box with five dozen liquor jugs",       14, 0 },
		{ "fi fl ffi  -> => != >= <=  x\xC2\xB2 H\xE2\x82\x82O  \xC2\xBD \xE2\x82\xAC \xC2\xA9", 20, 0 },
		{ "\xC3\x80\xC3\x89\xC3\x8E\xC3\x95\xC3\x9C \xC3\xB1 \xC3\xA7 \xC3\x9F  \xCE\xA9\xCE\xB4  \xD0\x96\xD1\x89  \xE2\x86\x92 \xE2\x89\xA0 \xE2\x89\x88 \xE2\x88\x9E", 20, 0 },
	};
	return list;
}

void FontAsset::update_preview(HAL::Texture::ptr preview)
{
	if (!preview || !preview->is_rt())
		preview.reset(new HAL::Texture(RenderSystem::get().device(), HAL::ResourceDesc::Tex2D(HAL::Format::R8G8B8A8_UNORM, { 256, 256 }, 1, 6, HAL::ResFlags::ShaderResource | HAL::ResFlags::RenderTarget | HAL::ResFlags::UnorderedAccess)));

	auto list = (RenderSystem::get().device().get_frame_manager().begin_frame()->start_list(L"FontAsset", HAL::CommandListType::DIRECT, true));

	// Every line is laid out first: the glyphs of all of them are uploaded in one go, ahead of the draws.
	const Text::FontId font_id = get_font();
	std::vector<Text::Layout> lines;
	std::vector<float>        tops;
	float top = 6;

	for (auto& sample : samples())
	{
		// The thumbnail's lines are all that fit in 256x256: sizes of 0 are window-only.
		if (sample.thumbnail_size <= 0)
			continue;

		Text::Layout layout;
		Text::Engine::get().build(sample.text, { sample.thumbnail_size, Text::Weight::Normal, Text::Family::Sans, font_id }, layout);
		tops.push_back(top);
		top += layout.size.y + 2;
		lines.push_back(std::move(layout));
	}

	Text::Engine::get().upload(list);

	{
		RT::Frame::SingleColor rt;
		rt.GetColor() = preview->texture_2d().renderTarget;
		list->get_graphics().set_rtv(rt, HAL::RTOptions::Default | HAL::RTOptions::ClearColor);
	}

	// raw_output: an R8G8B8A8_UNORM preview texture, not the scRGB swapchain.
	for (size_t i = 0; i < lines.size(); ++i)
		Text::Engine::get().draw(list, lines[i], { 8, tops[i] }, float4(1, 1, 1, 1), sizer{ 0, 0, 256, 256 }, { 256, 256 }, true);

	MipMapGenerator::get().generate(list->get_compute(), preview->texture_2d());
	list->execute();

	mark_changed();
}
