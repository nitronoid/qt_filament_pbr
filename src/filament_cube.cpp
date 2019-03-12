#include "filament_cube.h"

#include <utils/EntityManager.h>
#include <filament/VertexBuffer.h>
#include <filament/IndexBuffer.h>
#include <filament/RenderableManager.h>
#include <filament/TransformManager.h>

using namespace filament::math;
using namespace filament;

const uint32_t Cube::m_indices[] = {
        // solid
        2,0,1, 2,1,3,  // far
        6,4,5, 6,5,7,  // near
        2,0,4, 2,4,6,  // left
        3,1,5, 3,5,7,  // right
        0,4,5, 0,5,1,  // bottom
        2,6,7, 2,7,3,  // top

        // wire-frame
        0,1, 1,3, 3,2, 2,0,     // far
        4,5, 5,7, 7,6, 6,4,     // near
        0,4, 1,5, 3,7, 2,6,
};

const filament::math::float3 Cube::m_vertices[] = {
        { -1, -1,  1},  // 0. left bottom far
        {  1, -1,  1},  // 1. right bottom far
        { -1,  1,  1},  // 2. left top far
        {  1,  1,  1},  // 3. right top far
        { -1, -1, -1},  // 4. left bottom near
        {  1, -1, -1},  // 5. right bottom near
        { -1,  1, -1},  // 6. left top near
        {  1,  1, -1}}; // 7. right top near


Cube::Cube(
    const std::shared_ptr<filament::Engine>& i_engine, 
    const filament::Material* i_material, 
    filament::math::float3 i_linear_color, 
    bool i_culling) 
  : m_engine(std::move(i_engine))
  , m_vertex_buffer(nullptr, {m_engine}) 
  , m_index_buffer(nullptr, {m_engine}) 
  , m_material(i_material) 
  , m_material_instance_solid(nullptr, {m_engine}) 
  , m_solid_renderable(m_engine) 
{
    m_vertex_buffer.reset(VertexBuffer::Builder()
            .vertexCount(8)
            .bufferCount(1)
            .attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT3)
            .build(*m_engine));

    m_index_buffer.reset(IndexBuffer::Builder()
            .indexCount(12*2 + 3*2*6)
            .build(*m_engine));

    if (m_material) 
    {
        m_material_instance_solid.reset(m_material->createInstance());
        m_material_instance_solid->setParameter("color", RgbaType::LINEAR,
                LinearColorA{i_linear_color.r, i_linear_color.g, i_linear_color.b, 0.05f});
    }

    m_vertex_buffer->setBufferAt(*m_engine, 0,
            VertexBuffer::BufferDescriptor(
                    m_vertices, m_vertex_buffer->getVertexCount() * sizeof(m_vertices[0])));

    m_index_buffer->setBuffer(*m_engine,
            IndexBuffer::BufferDescriptor(
                    m_indices, m_index_buffer->getIndexCount() * sizeof(uint32_t)));

    utils::EntityManager& em = utils::EntityManager::get();
    m_solid_renderable = em.create();
    RenderableManager::Builder(1)
            .boundingBox({{ 0, 0, 0 },
                          { 1, 1, 1 }})
            .material(0, m_material_instance_solid.get())
            .geometry(0, RenderableManager::PrimitiveType::TRIANGLES, m_vertex_buffer.get(), m_index_buffer.get(), 0, 3*2*6)
            .priority(7)
            .culling(i_culling)
            .build(*m_engine, m_solid_renderable);

}

void Cube::map_frustum(const Camera* i_camera)
{
    // the Camera far plane is at infinity, but we want it closer for display
    const mat4 vm(i_camera->getModelMatrix());
    mat4 p(vm * inverse(i_camera->getCullingProjectionMatrix()));
    return map_frustum(p);
}

void Cube::map_frustum(const filament::math::mat4& i_transform) 
{
    // the Camera far plane is at infinity, but we want it closer for display
    mat4f p(i_transform);
    auto& tcm = m_engine->getTransformManager();
    tcm.setTransform(tcm.getInstance(m_solid_renderable), p);
}


void Cube::map_aabb(const filament::Box& i_box)
{
    mat4 p = mat4::translate(i_box.center) * mat4::scale(i_box.halfExtent);
    return map_frustum(p);
}

