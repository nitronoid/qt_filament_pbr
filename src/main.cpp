#include <QApplication>
#include "app_window.h"
#include "filament_window_widget.h"

// filament::Texture* load_texture(filament::Engine* io_engine, const
// utils::Path& i_texture_path)
//{
//  if (i_texture_path.isEmpty() || !i_texture_path.exists())
//    return nullptr;
//  using namespace OIIO;
//  // unique_ptr with custom deleter to close file on exit
//  std::unique_ptr<ImageInput, void (*)(ImageInput*)> input(
//    ImageInput::create(i_texture_path.c_str())
//#if OIIO_VERSION >= 10900
//      .release()
//#endif
//      ,
//    [](auto ptr) {
//      ptr->close();
//      delete ptr;
//    });
//
//  const auto& spec = input->spec();
//  const auto n_data = spec.width * spec.height * spec.nchannels;
//  auto image_data = std::make_unique<uint8_t[]>(n_data);
//  input->read_image(TypeDesc::UINT8, image_data.get());
//
//  auto texture = filament::Texture::Builder()
//                    .width(spec.width)
//                    .height(spec.height)
//                    .levels(0xff)
//                    .format(filament::driver::TextureFormat::SRGB8)
//                    .build(*io_engine);
//
//
//  filament::Texture::PixelBufferDescriptor buffer(
//      image_data.release(),
//      n_data,
//      filament::Texture::Format::RGB,
//      filament::Texture::Type::UBYTE,
//      [](void* data, size_t, void*)
//      {
//        typename decltype(image_data)::deleter_type {}(
//          static_cast<uint8_t*>(data));
//      });
//
//  texture->setImage(*io_engine, 0, std::move(buffer));
//  texture->generateMipmaps(*io_engine);
//  return texture;
//}

int main(int argc, char* argv[])
{
  // Create the application
  QApplication app(argc, argv);
  // Create a new main window
  AppWindow window;
  // Set the back-end we want filament to use for rendering
  auto filament_backend = filament::Engine::Backend::OPENGL;
  // Create our filament engine
  std::shared_ptr<filament::Engine> filament_engine(
    filament::Engine::create(filament_backend),
    [](filament::Engine* i_engine) { i_engine->destroy(&i_engine); });
  // Create our filament window
  auto filament_widget =
    std::make_shared<FilamentWindowWidget>(&window, filament_engine);
  // Initialize the filament entities and set-up cameras
  filament_widget->init();
  // Initialize the main window using our filament scene
  window.init(filament_widget);
  // Show it
  window.show();
  // Hand control over to Qt framework
  return app.exec();
}

