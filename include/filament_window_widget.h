#ifndef FILAMENT_WINDOW_WIDGET
#define FILAMENT_WINDOW_WIDGET

#include "native_window_widget.h"
#include "environment_light.h"
#include <filameshio/MeshReader.h>
#include <nonstd/value_ptr.hpp>
#include <math/vec2.h>
#include <math/quat.h>

// Our filament rendering window
class FilamentWindowWidget final : public NativeWindowWidget
{
public:
  explicit FilamentWindowWidget(QWidget* i_parent,
                                std::shared_ptr<filament::Engine> i_engine);
  ~FilamentWindowWidget();

  virtual void mousePressEvent(QMouseEvent* i_mouse_event) override;

  virtual void mouseMoveEvent(QMouseEvent* i_mouse_event) override;

private:
  void calculate_camera_view();

  void calculate_camera_projection();

  void init_materials();

  void init_meshes();

  void init_lighting();

  virtual void init_impl(void* io_native_window) override;

  virtual void resize_impl() override;

  virtual void draw_impl() override;

  virtual void closeEvent(QCloseEvent* i_event) override;

private:
  struct FilamentWindowWidgetImpl;
  // Value semantics for a smart pointer, automatically deep copies
  nonstd::value_ptr<FilamentWindowWidgetImpl> m_impl;
};

#endif  // FILAMENT_WINDOW_WIDGET

