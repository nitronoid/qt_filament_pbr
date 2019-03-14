#include "trackball_camera.h"
#include <math/fast.h>
#include <math/quat.h>
#include <math/scalar.h>

namespace
{
template <typename T>
constexpr T pi() noexcept
{
  return std::atan(1.0) * 4.0;
}

filament::math::float2
trackball_coordinate(filament::math::float2 i_spherical_pos,
                     const filament::math::float2& i_track_velocity,
                     const float i_damping) noexcept
{
  i_spherical_pos += i_track_velocity * i_damping;
  i_spherical_pos.x = std::fmod(i_spherical_pos.x, 2.f * pi<float>());
  static constexpr float epsilon = 0.1f;
  static constexpr float half_pi = pi<float>() * 0.5f - epsilon;
  i_spherical_pos.y =
    filament::math::clamp(i_spherical_pos.y, -half_pi, half_pi);
  return i_spherical_pos;
}

filament::math::quatf
fast_trackball_rotation(filament::math::float2 i_spherical_position)
{
  // Half the angles
  i_spherical_position *= 0.5f;
  namespace flm = filament::math;
  // We know that yaw will rotate around the Y axis, and pitch around the Z
  // So we can store simply two floats rather than an entire quaternion for
  // each of the two#
  // a.x === q.y, a.y === q.w
  // b.x === p.x, b.y === p.w
  const flm::float2 a{flm::fast::sin(i_spherical_position.x),
                      flm::fast::cos(i_spherical_position.x)};
  const flm::float2 b{flm::fast::sin(i_spherical_position.y),
                      flm::fast::cos(i_spherical_position.y)};
  // Using the above mapping, we can simplify the hammilton product to
  // -(q.y * p.x) + i(q.w * p.w) + j(q.w * p.x) + k(q.y * p.w)
  // \_________/    \_______________________________________/
  //      |                             |
  //     real                       imaginary
  // We now only have 4 fast trig calls and 4 multiplies so roughly ~40 cycles
  return {a.y * b.y, a.y * b.x, a.x * b.y, -(a.x) * b.x};
}
}  // namespace

struct TrackballCamera::TrackballCameraImpl
{
  filament::math::quatf rotation{0.f, 0.f, 0.f, 1.f};
  const filament::math::float3 arm_direction{0.f, 0.f, 1.f};
  float arm_length = 4.f;
  filament::math::float3 target{0.f};
  filament::math::float2 spherical_position{0.f};
  filament::math::float2 mouse_position{0.f};
  float sensitivity = 0.005f;
  ACTION action = ORBIT;
};

// Declare constructors and assignment operators after the definition for
// TrackballCameraImpl is visible
TrackballCamera::TrackballCamera() : m_impl(TrackballCameraImpl{})
{
}
TrackballCamera::TrackballCamera(const TrackballCamera&) = default;
TrackballCamera& TrackballCamera::operator=(const TrackballCamera&) = default;
TrackballCamera::TrackballCamera(TrackballCamera&&) = default;
TrackballCamera& TrackballCamera::operator=(TrackballCamera&&) = default;
TrackballCamera::~TrackballCamera() = default;


void TrackballCamera::orbit(filament::math::float2 i_mouse_position) noexcept
{
  // Calculate the new spherical coordinate of the camera, from the mouse
  // velocity vector, the current position and a damping factor
  m_impl->spherical_position =
    trackball_coordinate(m_impl->spherical_position,
                         m_impl->mouse_position - i_mouse_position,
                         m_impl->sensitivity);
  // Store the last mouse position
  m_impl->mouse_position = std::move(i_mouse_position);
  // Calculate the rotation from our cameras origin, around the target to the
  // new position
  m_impl->rotation = fast_trackball_rotation(m_impl->spherical_position);
}

void TrackballCamera::zoom(filament::math::float2 i_mouse_position) noexcept
{
  // Extend or contract the camera arm
  m_impl->arm_length +=
    (m_impl->mouse_position.y - i_mouse_position.y) * m_impl->sensitivity;
  // Store the last mouse position
  m_impl->mouse_position = std::move(i_mouse_position);
}

void TrackballCamera::set_mouse_position(
  filament::math::float2 i_mouse_position) noexcept
{
  // Store the last mouse position
  m_impl->mouse_position = std::move(i_mouse_position);
}

void TrackballCamera::set_action(ACTION i_new_action) noexcept
{
  m_impl->action = i_new_action;
}

void TrackballCamera::pan(filament::math::float2 i_mouse_position) noexcept
{
}

void TrackballCamera::act(filament::math::float2 i_mouse_position) noexcept
{
  switch (m_impl->action)
  {
  case ORBIT: orbit(i_mouse_position); break;
  case ZOOM: zoom(i_mouse_position); break;
  case PAN: break;
  default: break;
  };
}

filament::math::float3 TrackballCamera::eye() const noexcept
{
  // Calculate the camera arm
  const auto arm = m_impl->arm_direction * m_impl->arm_length;
  // Calculate the new position by rotating the camera origin position vector
  return m_impl->target - (m_impl->rotation * (m_impl->target - arm));
}

filament::math::float3 TrackballCamera::target() const noexcept
{
  return m_impl->target;
}

filament::math::float3 TrackballCamera::up() const noexcept
{
  // We use Y as the up direction
  constexpr filament::math::float3 up(0.f, 1.f, 0.f);
  return up;
}

