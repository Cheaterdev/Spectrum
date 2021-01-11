#include "DX12/Buffer.h"
#include "DX12/PipelineState.h"
#include "InputLayouts.h"
#include "Context/Context.h"
namespace DX12
{
    class vertex_buffer_stage
    {
            std::vector <GPUBuffer::ptr > base_buffers;
            std::vector<D3D12_VERTEX_BUFFER_VIEW> native_handles;
        public:
            input_layout_header layout;
            //   std::shared_ptr<DX11::input_layout> layout;
            void create_layout(input_layout_header& header);
            void create_layout(std::vector<vertex_buffer_layout>& layouts, vertex_shader::ptr& shader);

            void add(GPUBuffer::ptr buffer, std::vector<vertex_buffer_layout>& layouts);
            void set(MeshRenderContext::ptr& list);
    };


    struct vertex_buffer_data
    {
            std::string data; unsigned int stride;
            std::vector<vertex_buffer_layout> layouts;

            template<class T>
            void init_buffer(T* data, unsigned int size)
            {
                this->data.assign(reinterpret_cast<char*>(data), reinterpret_cast<char*>(data + size));
                stride = sizeof(T);
            }
        private:
            friend class boost::serialization::access;

            template<class Archive>
            void serialize(Archive& ar, const unsigned int)
            {
                ar& NVP(data);
                ar& NVP(stride);
                ar& NVP(layouts);
            }

    };

    struct vertex_mesh_data
    {
            std::vector<vertex_buffer_data> buffers;
        private:
            friend class boost::serialization::access;

            template<class Archive>
            void serialize(Archive& ar, const unsigned int)
            {
                ar& NVP(buffers);
            }
    };

    struct vertex_buffer
    {
        //    Buffer<char>::ptr buffer;
        std::vector<vertex_buffer_layout> layouts;

        vertex_buffer() {}
        vertex_buffer(const vertex_buffer_data& data)
        {
            //    buffer.reset(new Buffer<char>(HeapType::DEFAULT, data.data.size(), data.stride));
            //    buffer->set_data(data.data.data(), data.data.size());
            layouts = data.layouts;
        }
    };

    struct vertex_mesh
    {
            std::vector<vertex_buffer> buffers;

            void set(MeshRenderContext::ptr& list);

            vertex_buffer_stage& get_layout(vertex_shader::ptr& shader);
            vertex_mesh() {}
            vertex_mesh(const vertex_mesh_data& data)
            {
                for (auto& t : data.buffers)
                    buffers.emplace_back(t);
            }
        protected:
            vertex_buffer_stage stage;
            std::map<std::shared_ptr<shader_inputs>, vertex_buffer_stage> resulted_layouts;
    };

    struct index_mesh_desc
    {
            unsigned int offset;
            unsigned int size;
            unsigned int vertex_offset;
        private:
            friend class boost::serialization::access;

            template<class Archive>
            void serialize(Archive& ar, const unsigned int)
            {
                ar& NVP(offset);
                ar& NVP(size);
                ar& NVP(vertex_offset);
            }

    };

    struct indices_data
    {
            std::vector<unsigned int> indices;
            std::vector<index_mesh_desc> meshes;
        private:
            friend class boost::serialization::access;

            template<class Archive>
            void serialize(Archive& ar, const unsigned int)
            {
                ar& NVP(indices);
                ar& NVP(meshes);
            }

    };

    class index_mesh
    {

            // IndexBuffer<unsigned int>::ptr buffer;
        public:
            index_mesh() {}
            index_mesh(const indices_data& data)
            {
                //    buffer.reset(new IndexBuffer<unsigned int>(DXGI_FORMAT::DXGI_FORMAT_R32_UINT, data.indices.size()));
                //  buffer->set_data(data.indices.data(), data.indices.size());
            }


            void set(MeshRenderContext::ptr& context)
            {
                //   context->list->get_graphics().set_index_buffer(buffer);
            }

    };

}