#ifndef OPENGL_SHADER_H
#define OPENGL_SHADER_H

#include "Render/API/Shader.h"

namespace Karen
{
  class OpenGlShader : public Shader
  {
  public:
    OpenGlShader();
    OpenGlShader(const std::string& vp, const std::string& fp);
    ~OpenGlShader();

    void bind()   const override;
    void unbind() const override;
    void loadFromFile(const std::string& vp, const std::string& fp) override;
  private:
    void compileShaders(const std::string& vs, const std::string& fs);
    void createProgram();
  private:
    std::string vertex_src;
    std::string fragment_src;
    uint32_t    m_program_id, m_vs_id, m_fs_id;
  };
}
#endif //OPENGL_SHADER_H
