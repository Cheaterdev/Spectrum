enum class vertex_types : int { simple, instanced, skeletal, count};

class vertex_transform
{
        std::vector<Render::vertex_shader::ptr> transformers = std::vector<Render::vertex_shader::ptr>(3, Render::vertex_shader::null);
    public:
        using ptr = s_ptr<vertex_transform>;

        void set(MeshRenderContext::ptr& context, vertex_types type)
        {
            context->pipeline.vertex = transformers[static_cast<int>(type)];
        }

        Render::vertex_shader::ptr& get(vertex_types type)
        {
            return transformers[static_cast<int>(type)];
        }

};
