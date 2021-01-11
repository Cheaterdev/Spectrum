#pragma once
#include "Asset.h"

class BinaryAsset : public Asset
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
        virtual void update_preview(DX12::Texture::ptr preview);
        virtual void try_register();
		virtual void reload_resource() override;
    private:




        friend class boost::serialization::access;

        template<class Archive>
        void serialize(Archive& ar, const unsigned int)
        {
            ar& NVP(boost::serialization::base_object<Asset>(*this));
            ar& NVP(data);
            ar& NVP(file_depends);
            ar& NVP(file_name);

            if (Archive::is_loading::value)
            {
                reload_resource();
				std::weak_ptr<bool> r = tester;
                FileSystem::get().get_provider<native_file_provider>()->on_change(file_name, [this,r]()
                {
					if(r.lock())
                    reload_resource();
                });
            }
        }

};
BOOST_CLASS_EXPORT_KEY(AssetReference<BinaryAsset>);