export module GUI:Separator;
import :Base;

export namespace GUI
{
    namespace Elements
    {
        class separator : public base
        {
        public:
            using ptr = s_ptr<separator>;
            using wptr = w_ptr<separator>;

            enum class orientation { horizontal, vertical };

            float4      color       = {1, 1, 1, 0.15f};
            float       thickness   = 1.0f;
            orientation orient      = orientation::horizontal;

            separator(orientation o = orientation::horizontal);

            virtual void draw(Context& c) override;
        };
    }
}
