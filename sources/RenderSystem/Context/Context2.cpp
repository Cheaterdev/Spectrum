module Graphics:Context;
import <RenderSystem.h>;
import Core;
import HAL;

MeshRenderContext::MeshRenderContext()
{
}

void MeshRenderContext::begin()
{
    draw_count = 0;
}

void GBuffer::SetTable(Table::GBuffer& table)
{
    table.GetAlbedo() = albedo.texture2D;
    table.GetNormals() = normals.texture2D;
    table.GetSpecular() = specular.texture2D;
    table.GetDepth() = depth_mips.texture2D;
    table.GetMotion() = speed.texture2D;
}
