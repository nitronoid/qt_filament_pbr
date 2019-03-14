#include "filament_window_widget.h"
#include "filament_raii.h"
#include "trackball_camera.h"
#include <QMouseEvent>
#include <array>
#include <filament/Material.h>
#include <filament/MaterialInstance.h>
#include <filament/RenderableManager.h>
#include <filament/Scene.h>
#include <filament/View.h>
#include <filament/LightManager.h>
#include <filament/TransformManager.h>
#include <filament/IndirectLight.h>
#include <filament/Skybox.h>


// Private state of the filament window widget
struct FilamentWindowWidget::FilamentWindowWidgetImpl
{
  FilamentWindowWidgetImpl(std::shared_ptr<filament::Engine> i_engine);
  // Store a shared pointer to the engine, all of our entities will also store
  std::shared_ptr<filament::Engine> engine;

  // Scoped unique pointers to all engine registered objects
  FilamentScopedPointer<filament::SwapChain> swap_chain;
  FilamentScopedPointer<filament::Renderer> renderer;
  FilamentScopedPointer<filament::Camera> camera;
  FilamentScopedPointer<filament::View> view;
  FilamentScopedPointer<filament::Scene> scene;
  FilamentScopedPointer<filament::Material> material;
  FilamentScopedPointer<filament::MaterialInstance> material_instance;

  // Scoped entity for our light
  FilamentScopedEntity light;
  FilamentScopedEntity mesh;
  filamesh::MeshReader::MaterialRegistry material_registry;

  // Implements the trackball camera state
  TrackballCamera camera_manager;
  // Implements image based lighting and environment map backdrop
  EnvironmentLight ibl_skybox;
};

// Construct our private state using the supplied filament engine
FilamentWindowWidget::FilamentWindowWidgetImpl::FilamentWindowWidgetImpl(
  std::shared_ptr<filament::Engine> i_engine)
  : engine(std::move(i_engine))
  , swap_chain(nullptr, {engine})
  , renderer(engine->createRenderer(), {engine})
  , camera(engine->createCamera(), {engine})
  , view(engine->createView(), {engine})
  , scene(engine->createScene(), {engine})
  , material(nullptr, {engine})
  , material_instance(nullptr, {engine})
  , light(utils::EntityManager::get().create(), engine)
  , mesh(engine)
  , ibl_skybox(engine)
{
}

// This needs to be generated from the sample bakedColor.mat
// $>  matc -o bakedColor.inc -f header bakedColor.mat
static constexpr uint8_t AIDEFAULTMAT_PACKAGE[] = {
#include "assets/materials/aiDefaultMat.inc"
};

// Call the parent constructor, and construct the private state
FilamentWindowWidget::FilamentWindowWidget(
  QWidget* i_parent, std::shared_ptr<filament::Engine> i_engine)
  : NativeWindowWidget(i_parent), m_impl(std::move(i_engine))
{
}

// Define the destructor once the definition of FilamentWindowWidgetImpl is 
// visible
FilamentWindowWidget::~FilamentWindowWidget() = default;

// Handle user mouse presses by setting the state of our camera
void FilamentWindowWidget::mousePressEvent(QMouseEvent* i_mouse_event)
{
  QWidget::mousePressEvent(i_mouse_event);
  // Could replace this with command pattern to allow re-mapping of controls
  switch (i_mouse_event->button())
  {
  case Qt::LeftButton:
    m_impl->camera_manager.set_action(TrackballCamera::ORBIT);
    break;
  case Qt::RightButton:
    m_impl->camera_manager.set_action(TrackballCamera::ZOOM);
    break;
  default: break;
  };
  // Update our camera manager with the new mouse position
  m_impl->camera_manager.set_mouse_position(
    {i_mouse_event->x(), i_mouse_event->y()});
}

// Update the camera view matrix using the camera manager
void FilamentWindowWidget::mouseMoveEvent(QMouseEvent* i_mouse_event)
{
  QWidget::mouseMoveEvent(i_mouse_event);
  // Get the new mouse position
  filament::math::float2 new_mouse_position(i_mouse_event->x(),
                                            i_mouse_event->y());
  // Let the camera respond to mouse movement
  m_impl->camera_manager.act(std::move(new_mouse_position));
  // Recalculate the camera view matrix
  calculate_camera_view();
  // Redraw the scene now that we've moved the camera
  request_draw();
}

// Load and link our materials here
void FilamentWindowWidget::init_materials()
{
  // Load the material for our mesh
  m_impl->material.reset(
    filament::Material::Builder()
      .package((void*)AIDEFAULTMAT_PACKAGE, sizeof(AIDEFAULTMAT_PACKAGE))
      .build(*m_impl->engine));
  // Create an instance of the material to set params
  m_impl->material_instance.reset(m_impl->material->createInstance());
  // Set material parameters
  m_impl->material_instance->setParameter(
    "baseColor",
    filament::RgbType::LINEAR,
    {0.1f, 0.4f, 0.9f});
  m_impl->material_instance->setParameter("metallic", 1.0f);
  m_impl->material_instance->setParameter("roughness", 0.3f);
  m_impl->material_instance->setParameter("reflectance", 0.5f);
  m_impl->material_registry["DefaultMaterial"] =
    m_impl->material_instance.get();
}

// Load and link our meshes here
void FilamentWindowWidget::init_meshes()
{
  // Load the Suzanne mesh from a file
  m_impl->mesh =
    filamesh::MeshReader::loadMeshFromFile(m_impl->engine.get(),
                                           "assets/models/suzanne.filamesh",
                                           m_impl->material_registry)
      .renderable;
  // Allow the mesh to cast shadows in the scene
  auto& renderable_manager = m_impl->engine->getRenderableManager();
  auto renderable_instance = renderable_manager.getInstance(m_impl->mesh);
  renderable_manager.setCastShadows(renderable_instance, true);
  m_impl->scene->addEntity(m_impl->mesh);
}

// Set-up the scene's lighting here
void FilamentWindowWidget::init_lighting()
{
  // Load the skybox and setup image based lighting
  m_impl->ibl_skybox.load_ibl("assets/env/pillars/pillars_ibl.ktx",
                              "assets/env/pillars/pillars_skybox.ktx");
  // Link the skybox as our backdrop, and set the image texture as a light
  m_impl->scene->setSkybox(m_impl->ibl_skybox.m_skybox.get());
  m_impl->scene->setIndirectLight(m_impl->ibl_skybox.m_indirect_light.get());

  // Create a simple sun light to compliment the image based lighting
  filament::LightManager::Builder(filament::LightManager::Type::SUN)
    .color(filament::Color::toLinear<filament::ACCURATE>(
      filament::sRGBColor(0.98f, 0.92f, 0.89f)))
    .intensity(110000)
    .direction({0.7, -1, -0.8})
    .sunAngularRadius(1.9f)
    .castShadows(false)
    .build(*m_impl->engine, m_impl->light);
  // Add the light to the scene
  m_impl->scene->addEntity(m_impl->light);
}

// Scene set-up, linking of filament components, creation of materials etc.
void FilamentWindowWidget::init_impl(void* io_native_window)
{
  NativeWindowWidget::init_impl(io_native_window);
  // Create our swap chain for displaying rendered frames
  m_impl->swap_chain.reset(m_impl->engine->createSwapChain(io_native_window));

  // Link the camera and scene to our view point
  m_impl->view->setCamera(m_impl->camera.get());
  m_impl->view->setScene(m_impl->scene.get());

  // Calculate the camera's view matrix
  calculate_camera_view();
  // Set up the render view point
  calculate_camera_projection();

  // Screen space effects
  m_impl->view->setClearColor({0.3f, 0.3f, 0.3f, 1.0f});
  m_impl->view->setPostProcessingEnabled(true);
  m_impl->view->setDepthPrepass(filament::View::DepthPrepass::ENABLED);
  m_impl->view->setAntiAliasing(filament::View::AntiAliasing::FXAA);
  m_impl->view->setRenderQuality({filament::View::QualityLevel::ULTRA});

  init_materials();
  init_meshes();
  init_lighting();
}
//
// Update the camera view matrix using the camera manager
void FilamentWindowWidget::calculate_camera_view()
{
  // Recalculate the view matrix
  m_impl->camera->lookAt(m_impl->camera_manager.eye(),
                         m_impl->camera_manager.target(),
                         m_impl->camera_manager.up());
}

// Update the camera projection matrix using the window size
void FilamentWindowWidget::calculate_camera_projection()
{
  // Get the width and height of our window, scaled by the pixel ratio
  const auto pixel_ratio = devicePixelRatio();
  const uint32_t w = static_cast<uint32_t>(width() * pixel_ratio);
  const uint32_t h = static_cast<uint32_t>(height() * pixel_ratio);

  // Set our view-port size
  m_impl->view->setViewport({0, 0, w, h});

  // setup projection matrix
  const float far = 50.f;
  const float near = 0.1f;
  const float aspect = float(w) / h;
  m_impl->camera->setProjection(
    45.0f, aspect, near, far, filament::Camera::Fov::VERTICAL);
}

void FilamentWindowWidget::resize_impl()
{
  NativeWindowWidget::resize_impl();
  // Recalculate our camera matrices
  calculate_camera_projection();
}

void FilamentWindowWidget::draw_impl()
{
  NativeWindowWidget::draw_impl();
  // beginFrame() returns false if we need to skip a frame
  if (m_impl->renderer->beginFrame(m_impl->swap_chain.get()))
  {
    m_impl->renderer->render(m_impl->view.get());
    m_impl->renderer->endFrame();
  }
}

void FilamentWindowWidget::closeEvent(QCloseEvent* i_event)
{
  QWidget::closeEvent(i_event);
  // We need to ensure all rendering operations have completed before we
  // destroy our engine registered objects.
  // Safe to assume we won't be issuing anymore render calls after this
  filament::Fence::waitAndDestroy(m_impl->engine->createFence());
}
