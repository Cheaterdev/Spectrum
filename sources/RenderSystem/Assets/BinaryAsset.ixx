export module Graphics:BinaryAsset;

import :Asset;


export class BinaryAsset : public Asset
{
        std::string data;
        std::wstring file_name;
        resource_file_depender file_depends;

		std::shared_ptr<bool> tester;
        BinaryAsset();
        LEAK_TEST(BinaryAsset)
    public:

        virtual ~BinaryAsset()
        {
            //data.clear();
        }
        using ptr = s_ptr<BinaryAsset>;
        using ref = AssetReference<BinaryAsset>;

        BinaryAsset(std::wstring file_name);

        std::string get_data();

        virtual Asset_Type get_type();
        virtual void update_preview(HAL::Texture::ptr preview);
        virtual void try_register();
		virtual void reload_resource() override;
    private:



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
                FileSystem::get().get_provider<native_file_provider>()->on_change(to_path(file_name), [this,r]()
                {
					if(r.lock())
                    reload_resource();
                });
            }
        }

};
// REGISTER_TYPE(AssetReference<BinaryAsset>);