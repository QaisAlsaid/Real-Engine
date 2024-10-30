#ifndef EDITOR_CAMERA_H
#define EDITOR_CAMERA_H

#include "Real-Engine/Render/Camera.h"
#include "Real-Engine/Core/Events/MouseEvents.h"
#include "Real-Engine/Core/Math/math.h"
#include "Real-Engine/Core/Timestep.h"


namespace Real
{
  constexpr auto pi = glm::pi<float>();

  class EditorCamera : public Camera
  {
  public:
    void onUpdate(Timestep ts);
    Mat4 getView() const;
    void onResize(uint32_t width, uint32_t height);
    void onMouseScrolledEvent(MouseScrolledEvent& e);
  public:
    Vec3 world_up = { 0, 1, 0 };
    Vec3 up       = { 0, 1, 0 };
    Vec3 right    = { 1, 0, 0 };
    Vec3 front    = { 0, 0, 1 };
    Vec3 position = { 0, 0, 1 };

    float aspect_ratio = 16.0f/9.0f;
    float near = 0.1f, far = 1000.0f;
    float mouse_sensitivaty = 0.1f;
    float min_fov = glm::radians(0.1f), max_fov = pi / 2.0f;
    float yaw = -pi/2.0f, pitch = 0;
    float fov = pi/4.0f;
    bool fixed_aspect_ratio = false;
  private:
    Vec2 m_last_mouse = { 0.0f, 0.0f }; 
  };
}

#endif //EDITOR_CAMERA_H
