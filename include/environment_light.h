#ifndef ENVIRONMENT_LIGHT
#define ENVIRONMENT_LIGHT

#include "filament_raii.h"
#include <utils/Path.h>
#include <math/vec3.h>
#include <array>

struct EnvironmentLight
{
  EnvironmentLight(const std::shared_ptr<filament::Engine>& i_engine);

  void load_ibl(const utils::Path& i_ibl_path,
                const utils::Path& i_skybox_path);

  std::shared_ptr<filament::Engine> m_engine;
  std::array<filament::math::float3, 9> m_ibl_bands;
  FilamentScopedPointer<filament::Texture> m_ibl_texture;
  FilamentScopedPointer<filament::IndirectLight> m_indirect_light;
  FilamentScopedPointer<filament::Texture> m_skybox_texture;
  FilamentScopedPointer<filament::Skybox> m_skybox;
  FilamentScopedPointer<filament::Material> m_transparent_material;
};


#endif//ENVIRONMENT_LIGHT

