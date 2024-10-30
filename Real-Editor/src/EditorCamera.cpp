#include "EditorCamera.h"
#include "Real-Engine/Core/ButtonsAndKeyCodes.h"
#include "Real-Engine/Core/Input.h"
#include "glm/ext/matrix_clip_space.hpp"
#include <algorithm>


namespace Real
{
  void EditorCamera::onUpdate(Timestep ts)
  {

    //vectors from Euler angles
    {
      front.x = cos(yaw) * cos(pitch);
      front.y = sin(pitch);
      front.z = sin(yaw) * cos(pitch);
      front = glm::normalize(front);
    }
    //find camera vectors
    {
      right = glm::normalize(glm::cross(front, world_up));
      up    = glm::normalize(glm::cross(right, front));
    }
    m_projection = glm::perspectiveRH(fov, aspect_ratio, near, far);

    //Mouse Movment
    {
      float s = std::min(aspect_ratio, 2.4f);
      float speed = 0.03f * (s * s) - 0.2f * s + 0.3f;
      auto mouse = Input::getMousePos();
      Vec2 dmouse = m_last_mouse - mouse;
      Vec2 scaled_dmouse = dmouse * speed * mouse_sensitivaty;
      scaled_dmouse.x *= -1;
      if(Input::isMouseButtonPressed(Mouse::ButtonRight))
      {
        yaw += scaled_dmouse.x;
        pitch += scaled_dmouse.y;
        if(pitch >= pi) pitch = pi - 0.001;
        else if(pitch <= -pi) pitch = -(pi - 0.001);
      }
      m_last_mouse = mouse;
    }

    //freelook
    {
      if(Input::isKeyPressed(Keyboard::D))
      {
        position += 5 * ts * right;
      }
      else if(Input::isKeyPressed(Keyboard::A))
      {
        position -= 5 * ts * right;
      }
      if(Input::isKeyPressed(Keyboard::W))
      {
        position += 5 * ts * front;
      }
      else if(Input::isKeyPressed(Keyboard::S))
      {
        position -= 5 * ts * front;
      }
    }
  }

  Mat4 EditorCamera::getView() const
  {
    return glm::lookAtRH(position, position + front, world_up);
  }

  void EditorCamera::onResize(uint32_t w, uint32_t h)
  {
    if(!fixed_aspect_ratio)
      aspect_ratio = (float)w/(float)h;
  }

  void EditorCamera::onMouseScrolledEvent(MouseScrolledEvent& e)
  {
    auto offset = e.getYoffset();
    //fov = glm::radians((glm::degrees(fov) - offset));
    //if(fov > max_fov) fov = max_fov;
    //else if(fov < min_fov) fov = min_fov;
    position += offset * 0.1f * front * glm::abs(glm::distance(position, Vec3{0}));
  }
}
