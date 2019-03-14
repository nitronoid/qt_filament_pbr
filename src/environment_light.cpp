#include "environment_light.h"
#include <image/KtxBundle.h>
#include <image/KtxUtility.h>
#include <fstream>
#include <array>
#include <sstream>
#include <filament/IndirectLight.h>
#include <filament/Skybox.h>
#include <filament/Material.h>


filament::Texture* load_ktx(filament::Engine* io_engine,
                            const utils::Path& i_texture_path,
                            image::KtxBundle** io_ktx_image = nullptr)
{
  if (i_texture_path.isEmpty() || !i_texture_path.exists())
    return nullptr;

  std::ifstream file(i_texture_path.getPath(), std::ios::binary);
  std::vector<uint8_t> contents((std::istreambuf_iterator<char>(file)), {});
  auto ktx_image = new image::KtxBundle(contents.data(), contents.size());
  if (io_ktx_image)
    *io_ktx_image = ktx_image;
  return image::KtxUtility::createTexture(io_engine, ktx_image, false, true);
}

EnvironmentLight::EnvironmentLight(const std::shared_ptr<filament::Engine>& i_engine)
  : m_engine(i_engine)
  , m_ibl_texture(nullptr, {i_engine})
  , m_indirect_light(nullptr, {i_engine})
  , m_skybox_texture(nullptr, {i_engine})
  , m_skybox(nullptr, {i_engine})
  , m_transparent_material(nullptr, {i_engine})
{
}

void EnvironmentLight::load_ibl(const utils::Path& i_ibl_path,
                   const utils::Path& i_skybox_path)
{
  image::KtxBundle* ibl_ktx = nullptr;
  m_ibl_texture.reset(load_ktx(m_engine.get(), i_ibl_path, &ibl_ktx));
  m_skybox_texture.reset(load_ktx(m_engine.get(), i_skybox_path));

  std::istringstream shstring(ibl_ktx->getMetadata("sh"));
  for (auto& band : m_ibl_bands)
  {
    shstring >> band.x >> band.y >> band.z;
  }

  m_indirect_light.reset(filament::IndirectLight::Builder()
                           .reflections(m_ibl_texture.get())
                           .irradiance(3, m_ibl_bands.data())
                           .intensity(30000.0f)
                           .build(*m_engine));

  m_skybox.reset(filament::Skybox::Builder()
                   .environment(m_skybox_texture.get())
                   .showSun(true)
                   .build(*m_engine));
}

