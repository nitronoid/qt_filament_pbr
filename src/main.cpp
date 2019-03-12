#include <QApplication>
#include <QSurfaceFormat>

#include <filament/Material.h>
#include <filament/MaterialInstance.h>
#include <filament/RenderableManager.h>
#include <filament/Scene.h>
#include <filament/View.h>
#include <filament/LightManager.h>
#include <filameshio/MeshReader.h>

#include "app_window.h"
#include "native_window_widget.h"
#include "filament_raii.h"

// To reduce the verbosity of filament code
namespace fl = filament;
namespace flm = filament::math;

// This needs to be generated from the sample bakedColor.mat
// $>  matc -o bakedColor.inc -f header bakedColor.mat
static constexpr uint8_t AIDEFAULTMAT_PACKAGE[] = {
#include "assets/materials/aiDefaultMat.inc"
};

// Our filament rendering window
class FilamentWindowWidget final : public NativeWindowWidget
{
public:
  explicit FilamentWindowWidget(QWidget* i_parent,
                                const filament::Engine::Backend i_backend)
    : NativeWindowWidget(i_parent)
    , m_engine(filament::Engine::create(i_backend),
               [](filament::Engine* i_engine) { i_engine->destroy(&i_engine); })
    , m_swap_chain(nullptr, {m_engine})
    , m_renderer(m_engine->createRenderer(), {m_engine})
    , m_camera(m_engine->createCamera(), {m_engine})
    , m_view(m_engine->createView(), {m_engine})
    , m_scene(m_engine->createScene(), {m_engine})
    , m_material(nullptr, {m_engine})
    , m_material_instance(nullptr, {m_engine})
    //, m_material(nullptr, {m_engine})
    , m_light(utils::EntityManager::get().create(), m_engine)
    , m_mesh(m_engine)
  {
  }

  virtual void init_impl(void* io_native_window) override
  {
    NativeWindowWidget::init_impl(io_native_window);
    // Create our swap chain for displaying rendered frames
    m_swap_chain.reset(m_engine->createSwapChain(io_native_window));

    // Link the camera and scene to our view point
    m_view->setCamera(m_camera.get());
    m_view->setScene(m_scene.get());

    // Set up the render view point
    setup_camera();

    // Screen space effects
    m_view->setClearColor({0.1f, 0.125f, 0.25f, 1.0f});
    m_view->setPostProcessingEnabled(false);
    m_view->setDepthPrepass(fl::View::DepthPrepass::DISABLED);

    m_material.reset(
      filament::Material::Builder()
        .package((void*)AIDEFAULTMAT_PACKAGE, sizeof(AIDEFAULTMAT_PACKAGE))
        .build(*m_engine));
    m_material_instance.reset(m_material->createInstance());
    m_material_instance->setParameter("baseColor", filament::RgbType::LINEAR, flm::float3{0.8f});
    m_material_instance->setParameter("metallic", 0.0f);
    m_material_instance->setParameter("roughness", 0.4f);
    m_material_instance->setParameter("reflectance", 0.1f);
    m_material_registry["DefaultMaterial"] = m_material_instance.get();

    // Load the Suzanne mesh from a file
    m_mesh =
      filamesh::MeshReader::loadMeshFromFile(
        m_engine.get(), "assets/models/suzanne.filamesh", m_material_registry)
        .renderable;
    // rcm.setCastShadows(rcm.getInstance(app.mesh.renderable), false);
    m_scene->addEntity(m_mesh);

    // Create a simple sun light
    fl::LightManager::Builder(fl::LightManager::Type::SUN)
      .color(
        fl::Color::toLinear<fl::ACCURATE>(fl::sRGBColor(0.98f, 0.92f, 0.89f)))
      .intensity(110000)
      .direction({0.7, -1, -0.8})
      .sunAngularRadius(1.9f)
      .castShadows(false)
      .build(*m_engine, m_light);
    // Add the light to the scene
    m_scene->addEntity(m_light);
  }

private:
  void setup_camera()
  {
    // Get the width and height of our window, scaled by the pixel ratio
    const auto pixel_ratio = devicePixelRatio();
    const uint32_t w = static_cast<uint32_t>(width() * pixel_ratio);
    const uint32_t h = static_cast<uint32_t>(height() * pixel_ratio);

    // Set our view-port size
    m_view->setViewport({0, 0, w, h});

    // setup view matrix
    const flm::float3 eye(0.f, 0.f, 1.f);
    const flm::float3 target(0.f);
    const flm::float3 up(0.f, 1.f, 0.f);
    m_camera->lookAt(eye, target, up);

    // setup projection matrix
    constexpr float k_zoom = 1.5f;
    const float aspect = float(w) / h;
    m_camera->setProjection(fl::Camera::Projection::ORTHO,
                            -aspect * k_zoom,
                            aspect * k_zoom,
                            -k_zoom,
                            k_zoom,
                            0,
                            1);
  }

  virtual void resize_impl() override
  {
    // Don't attempt to access any filament entities before we've initialized
    if (!m_is_init)
      return;
    NativeWindowWidget::resize_impl();
    // Recalculate our camera matrices
    setup_camera();
  }

  virtual void draw_impl() override
  {
    NativeWindowWidget::draw_impl();
    // beginFrame() returns false if we need to skip a frame
    if (m_renderer->beginFrame(m_swap_chain.get()))
    {
      m_renderer->render(m_view.get());
      m_renderer->endFrame();
    }
  }

  virtual void closeEvent(QCloseEvent* i_event) override
  {
    QWidget::closeEvent(i_event);
    // We need to ensure all rendering operations have completed before we
    // destroy our engine registered objects.
    // Safe to assume we won't be issuing anymore render calls after this
    filament::Fence::waitAndDestroy(m_engine->createFence());
  }

private:
  // Store a shared pointer to the engine, all of our entities will also store
  std::shared_ptr<filament::Engine> m_engine;

  // Scoped unique pointers to all engine registered objects
  FilamentScopedPointer<filament::SwapChain> m_swap_chain;
  FilamentScopedPointer<filament::Renderer> m_renderer;
  FilamentScopedPointer<filament::Camera> m_camera;
  FilamentScopedPointer<filament::View> m_view;
  FilamentScopedPointer<filament::Scene> m_scene;
  FilamentScopedPointer<filament::Material> m_material;
  FilamentScopedPointer<filament::MaterialInstance> m_material_instance;

  // Scoped entity for our light
  FilamentScopedEntity m_light;
  FilamentScopedEntity m_mesh;
  filamesh::MeshReader::MaterialRegistry m_material_registry;
};

int main(int argc, char* argv[])
{
  // Create the application
  QApplication app(argc, argv);
  // Create a new main window
  AppWindow window;
  // Create our filament window
  auto filament_widget = std::make_shared<FilamentWindowWidget>(
    &window, filament::Engine::Backend::OPENGL);
  // Initialize the filament entities and set-up cameras
  filament_widget->init();
  // Initialize the main window using our filament scene
  window.init(filament_widget);
  // Show it
  window.show();
  // Hand control over to Qt framework
  return app.exec();
}

