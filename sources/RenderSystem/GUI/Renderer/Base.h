namespace GUI
{



#define BUILD_BUFFERS(x)\
        Render::Buffer<v_c_b>::ptr cb_vertex;\
        Render::Buffer<p_c_b>::ptr cb_pixel;\
        Render::HandleTable vertex_table;\
        Render::HandleTable pixel_table;\
        x()\
        {\
            cb_vertex.reset(new Render::Buffer<v_c_b>());\
            cb_pixel.reset(new Render::Buffer<p_c_b>());\
            vertex_table = Render::DescriptorHeapManager::get().get_csu_static()->create_table(1);\
            pixel_table = Render::DescriptorHeapManager::get().get_csu_static()->create_table(1);\
            cb_vertex->place_const_buffer(vertex_table[0]);\
            cb_pixel->place_const_buffer(pixel_table[0]);\
        }\
void update(Render::CommandList::ptr& list)\
{\
cb_pixel->update(list);\
cb_vertex->update(list);\
}

    /*   struct SimpleData: public RenderData
       {
           struct v_c_b
           {
               vec2 p1;
               vec2 p2;
           };

           struct p_c_b
           {
               vec4 color1;
               vec4 color2;
               vec4 size;
               vec4 clipping;
           };

           BUILD_BUFFERS(SimpleData);
       };
    */

    template <class RENDERER, class OBJECT, class DATA>
    class renderer_base
    {
        protected:
            typedef DATA Data;
            typedef OBJECT Object;
            renderer_base()
            {
                /*      D3D11_SAMPLER_DESC sampDesc;
                      sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
                      sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
                      sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
                      sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
                      sampDesc.MipLODBias = 0.0f;
                      sampDesc.MaxAnisotropy = 16;
                      sampDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
                      sampDesc.BorderColor[0] = sampDesc.BorderColor[1] = sampDesc.BorderColor[2] = sampDesc.BorderColor[3] = 0;
                      sampDesc.MinLOD = 0;
                      sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
                      sampler = DX11::sampler_state::get_resource(sampDesc);*/
            }

            void set_sampler(Render::context& c)
            {
                //c.dx_context.get_shader_state<DX11::pixel_shader>().sampler_state[0] = sampler;
            }

        public:
            void draw(OBJECT* obj, Render::context& c)
            {
                if (!obj->render_data)
                    obj->render_data.reset(new DATA());

                DATA* data = static_cast<DATA*>(obj->render_data.get());
                render(obj, data, c);
            }

        protected:


            virtual void render(OBJECT* obj, DATA* data, Render::context& c) = 0;

    };
}