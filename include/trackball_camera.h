#ifndef TRACKBALL_CAMERA
#define TRACKBALL_CAMERA

#include <nonstd/value_ptr.hpp>
#include <math/vec2.h>
#include <math/vec3.h>


class TrackballCamera
{
public:
  TrackballCamera();
  TrackballCamera(const TrackballCamera&);
  TrackballCamera& operator=(const TrackballCamera&);
  TrackballCamera(TrackballCamera&&);
  TrackballCamera& operator=(TrackballCamera&&);
  ~TrackballCamera();

  void orbit(filament::math::float2 i_mouse_position) noexcept;
  void zoom(filament::math::float2 i_mouse_position) noexcept;
  void pan(filament::math::float2 i_mouse_position) noexcept;
  void set_mouse_position(filament::math::float2 i_mouse_position) noexcept;
  enum ACTION { ORBIT, ZOOM, PAN, NUM_BEHAVIOURS };
  void set_action(ACTION i_new_action) noexcept;

  void act(filament::math::float2 i_mouse_position) noexcept;

  filament::math::float3 eye() const noexcept;
  filament::math::float3 target() const noexcept;
  filament::math::float3 up() const noexcept;

private:
  struct TrackballCameraImpl;
  // Value semantics for a smart pointer, automatically deep copies
  nonstd::value_ptr<TrackballCameraImpl> m_impl;

};


#endif//TRACKBALL_CAMERA


