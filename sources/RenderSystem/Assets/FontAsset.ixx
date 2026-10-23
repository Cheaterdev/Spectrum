export module Graphics:FontAsset;

import :Asset;
import :Texture;
import TextEngine;

// A TTF/OTF file the text engine can draw with. Its preview shows the font in use.
export class FontAsset : public Asset
{
        std::string data;
        std::wstring file_name;
        resource_file_depender file_depends;

        std::shared_ptr<bool> tester;
        Text::FontId font = 0;   // registered with the engine on first use, see get_font()

        FontAsset();
    public:

        virtual ~FontAsset();
        using ptr = s_ptr<FontAsset>;
        using ref = AssetReference<FontAsset>;

        FontAsset(std::wstring file_name);

        const std::string& get_data() const { return data; }

        // The font as a Text::Style::font. Changes when the file is reloaded.
        Text::FontId get_font();

        // What the preview (thumbnail and window) draws: size is for the window, thumbnail_size
        // for the 256x256 thumbnail (0 = not in the thumbnail).
        struct Sample
        {
            std::string text;
            float       size;
            float       thumbnail_size;
        };
        // An emoji font (it has U+1F600) gets emoji instead of letters: it has no Latin glyphs.
        bool is_emoji();
        const std::vector<Sample>& samples();

        virtual Asset_Type get_type();
        virtual void update_preview(HAL::Texture::ptr preview);
        virtual void try_register();
        virtual void reload_resource() override;
    private:

        void release_font();

        SERIALIZE()
        {
            SAVE_PARENT(Asset);
            ar& NVP(data);
            ar& NVP(file_depends);
            ar& NVP(file_name);

            IF_LOAD()
            {
                reload_resource();
                std::weak_ptr<bool> r = tester;
                FileSystem::get().get_provider<native_file_provider>()->on_change(to_path(file_name), [this, r]()
                {
                    if (r.lock())
                        reload_resource();
                });
            }
        }
};
