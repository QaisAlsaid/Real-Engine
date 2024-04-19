#include "pch.h"
#include "Karen/Render/API/BufferLayout.h"
#include "Platforms/OpenGl/OpenGlVertexBuffer.h"
#include "Platforms/OpenGl/OpenGlCore.h"


namespace Karen
{
  OpenGlVertexBuffer::OpenGlVertexBuffer(uint32_t size, const float* data, uint16_t usage)
  {
    KAREN_PROFILE_FUNCTION();
    glGenBuffers(1, &m_renderer_id);
    glBindBuffer(GL_ARRAY_BUFFER, m_renderer_id);
    setData(size, data, usage);
  }

  OpenGlVertexBuffer::~OpenGlVertexBuffer()
  {
    KAREN_PROFILE_FUNCTION();
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &m_renderer_id);
  }

  void OpenGlVertexBuffer::bind() const
  {
    KAREN_PROFILE_FUNCTION();
    glBindBuffer(GL_ARRAY_BUFFER, m_renderer_id);
  }

  void OpenGlVertexBuffer::unbind() const
  {
    KAREN_PROFILE_FUNCTION();
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }

  void OpenGlVertexBuffer::setData(uint32_t size, const float* data, uint16_t usage)
  {
    //TODO: make the usage work
    KAREN_PROFILE_FUNCTION();
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
  }
}
