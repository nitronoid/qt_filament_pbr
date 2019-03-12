#ifndef FILAMENT_CUBE_H
#define FILAMENT_CUBE_H

#include <vector>

#include <filament/Engine.h>
#include <filament/Camera.h>
#include <filament/Material.h>
#include <filament/MaterialInstance.h>
#include <utils/Entity.h>
#include "filament_raii.h"

class Cube 
{
public:
  Cube(const std::shared_ptr<filament::Engine>& i_engine, const filament::Material* i_material, filament::math::float3 i_linear_color, bool i_culling = true);

  void map_frustum(const filament::Camera* i_camera);
  void map_frustum(const filament::math::mat4& i_transform);
  void map_aabb(const filament::Box& i_box);

  static const uint32_t m_indices[];
  static const filament::math::float3 m_vertices[];

  std::shared_ptr<filament::Engine> m_engine;

  FilamentScopedPointer<filament::VertexBuffer> m_vertex_buffer;
  FilamentScopedPointer<filament::IndexBuffer> m_index_buffer;
  filament::Material const* m_material;
  FilamentScopedPointer<filament::MaterialInstance> m_material_instance_solid;
  FilamentScopedEntity m_solid_renderable;
};


#endif // TNT_FILAMENT_SAMPLE_CUBE_H
