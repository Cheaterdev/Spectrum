#pragma once
#include "GUI/GUI.h"
#include "Camera/Camera.h"
#include "Assets/MeshAsset.h"
#include "DX12/Buffer.h"
#include "Scene/Scene.h"

class stencil_renderer : public GUI::base, public FrameGraphGenerator, public Events::Runner
{
     //   RenderTargetTable table;
    //    DX12::StructuredBuffer<UINT>::ptr id_buffer;
        camera cam;
        camera axis_intersect_cam;


     //   DX12::StructuredBuffer<UINT>::ptr axis_id_buffer;

        MeshAssetInstance::ptr axis;
     //   PlacedAllocator allocator;
        camera axis_cam;



        //  bool need_update = false;
        int selected_axis = -1;
        // float3 added = 0;
        float3 center_pos;
        float3 pivot_pos;
        std::pair<MeshAssetInstance::ptr, int> current_selected;
        ////////////////////////////

        void select_current();
        virtual bool on_mouse_action(mouse_action action, mouse_button button, vec2 pos)override;

        virtual     bool on_mouse_move(vec2 pos) override;

        float3 mouse_pos;
        float3 prev_mouse_pos;
        int mouse_on_axis = -1;

        float3 get_axis(int axis);

        float3 get_current_pos();
        float3 get_normal(int axis);
        std::pair<MeshAssetInstance::ptr, int> mouse_on_object;

		DX12::StructuredBuffer<vec4>::ptr vertex_buffer;
		DX12::IndexBuffer::ptr index_buffer;

        virtual bool can_accept(GUI::drag_n_drop_package::ptr p) override;
		virtual void on_drop_move(GUI::drag_n_drop_package::ptr p, vec2)override;
        virtual bool on_drop(GUI::drag_n_drop_package::ptr p, vec2 m) override;

		Plane current_plane;

    public:
		virtual void generate(FrameGraph& graph) override;
		void generate_after(FrameGraph& graph);
		Scene::ptr debug_scene;

        Scene::ptr scene;
        camera* player_cam;

        using ptr = s_ptr<stencil_renderer>;
        std::vector<std::pair<MeshAssetInstance::ptr, int>> selected;
        vec3 direction;
        stencil_renderer();
};