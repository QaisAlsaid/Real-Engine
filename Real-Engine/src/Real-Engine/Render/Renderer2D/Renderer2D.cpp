#include "Real-Engine/Render/API/Shader.h"
#include "pch.h"
#include "Renderer2D.h"
#include "Real-Engine/Core/Log.h"
#include "Real-Engine/Render/API/IndexBuffer.h"
#include "Real-Engine/Render/API/RenderCommands.h"
#include "Real-Engine/Render/API/VertexArray.h"
#include "Real-Engine/Render/API/VertexBuffer.h"
#include "glm/ext/matrix_transform.hpp"
#include <cstdint>

namespace Real
{
  Renderer2D::Data* Renderer2D::s_data = new Renderer2D::Data;

  void Renderer2D::init(const std::string& shaders_2d_config_path)
  {
    //Shaders
    s_data->shaders.LoadConfig(shaders_2d_config_path);
    
    //Default White Texture 
    uint32_t wh_data = 0xffffffff;
    s_data->wh_tux = Texture2D::create(1, 1, sizeof(wh_data), &wh_data);

    //Texture Slots
    s_data->texture_slots = new ARef<Texture>[s_data->MAX_TEXTURE_SLOTS];
    s_data->texture_slots[0] = s_data->wh_tux;

    //Quad
    s_data->quad_base = (Quad*)new QuadVertex[s_data->MAX_VERTS];
    uint32_t* quad_inds = new uint32_t[s_data->MAX_INDES];
    uint32_t offset = 0;
    //Same IndexBuffer for both quad and circle
    for(uint32_t i = 0; i < s_data->MAX_INDES; i += 6)
    {
      quad_inds[i + 0] = offset + 0;
      quad_inds[i + 1] = offset + 1;
      quad_inds[i + 2] = offset + 2;

      quad_inds[i + 3] = offset + 2;
      quad_inds[i + 4] = offset + 3;
      quad_inds[i + 5] = offset + 0;

      offset += 4;
    }

    s_data->quad_vertex_buff = VertexBuffer::create(sizeof(QuadVertex) * s_data->MAX_VERTS);
    ARef<IndexBuffer> ib  = IndexBuffer::create(s_data->MAX_INDES, quad_inds, 5);
    Real::BufferLayout bl = 
    {
      {"color", Real::ShaderDataType::Float4},
      {"pos", Real::ShaderDataType::Float3},
      {"tux_coord", Real::ShaderDataType::Float2},
      {"tux_idx", Real::ShaderDataType::Float}, 
      {"entity_id", Real::ShaderDataType::Int}
    };
    s_data->quad_vertex_buff->setLayout(bl);
    s_data->quad_vertex_arr = VertexArray::create();
    s_data->quad_vertex_arr->setIndexBuffer(ib);
    s_data->quad_vertex_arr->addVertexBuffer(s_data->quad_vertex_buff);

    int samplers[8];
    for(uint8_t i = 0; i < sizeof(samplers)/sizeof(samplers[0]); ++i)
    {
      samplers[i] = i;
    }
    s_data->shaders.get("tux_shader")->bind();
    s_data->shaders.get("tux_shader")->setUniform("u_textures", samplers, sizeof(samplers)/sizeof(samplers[0]));
    
    s_data->quad_vertex_pos[0] = {-0.5f, -0.5f, 0.0f, 1.0f};
    s_data->quad_vertex_pos[1] = { 0.5f, -0.5f, 0.0f, 1.0f};
    s_data->quad_vertex_pos[2] = { 0.5f,  0.5f, 0.0f, 1.0f};
    s_data->quad_vertex_pos[3] = {-0.5f,  0.5f, 0.0f, 1.0f};
    
    //Circle 
    s_data->circle_ptr = new Circle[s_data->MAX_QUADS]();
    s_data->circle_base = s_data->circle_ptr;
    
    s_data->circle_vertex_buff = VertexBuffer::create(sizeof(CircleVertex) * s_data->MAX_VERTS);
    ARef<IndexBuffer> cib  = IndexBuffer::create(s_data->MAX_INDES, quad_inds, 5);
    Real::BufferLayout cbl = 
    {
      {"color", Real::ShaderDataType::Float4},
      {"pos", Real::ShaderDataType::Float3},
      {"local_pos", Real::ShaderDataType::Float2}, 
      {"thickness", Real::ShaderDataType::Float}, 
      {"blur", Real::ShaderDataType::Float}, 
      {"entity_id", Real::ShaderDataType::Int}
    };
    
    s_data->circle_vertex_buff->setLayout(cbl);
    s_data->circle_vertex_array = VertexArray::create();
    s_data->circle_vertex_array->setIndexBuffer(cib);
    s_data->circle_vertex_array->addVertexBuffer(s_data->circle_vertex_buff);


    resetStats();

    delete[] quad_inds;
  }

  void Renderer2D::beginScene(const Camera& camera, const Mat4& transform)
  {
    beginScene(camera.getProjection(), glm::inverse(transform));
  }

  void Renderer2D::beginScene(const Mat4& projection, const Mat4& view)
  {
    const auto proj_view = projection * view;
    s_data->proj_view = proj_view;
    s_data->camera_view = view;
    reset();
  }


  void Renderer2D::reset()
  {
    //Quad
    s_data->quad_ptr = s_data->quad_base;
    s_data->quad_index_count = 0;
    s_data->texture_slot_index = 1;

    //Circle
    s_data->circle_ptr = s_data->circle_base;
    s_data->circle_index_count = 0;
  }
   
  void Renderer2D::drawCircle(const Mat4& trans, float thickness, float blur, const Vec4& color, int entity_id)
  {
    if(s_data->circle_index_count >= s_data->MAX_INDES)
    {
      flushCircle();
      reset();
    }

    for(uint8_t i = 0; i < 4; ++i)
    {
      s_data->circle_ptr->vertices[i].position = trans * s_data->quad_vertex_pos[i];
      s_data->circle_ptr->vertices[i].color = color;
      s_data->circle_ptr->vertices[i].local_position = s_data->quad_vertex_pos[i] * 2;
      s_data->circle_ptr->vertices[i].thickness = thickness;
      s_data->circle_ptr->vertices[i].blur = blur;
      s_data->circle_ptr->vertices[i].entity_id = entity_id;
    } s_data->circle_ptr++;
    
    s_data->circle_index_count += 6;

    s_data->stats.quad_count++;
  }
 
  void Renderer2D::drawQuad(const Mat4& trans, const Vec4& color, int entity_id)
  {
   if(s_data->quad_index_count >= s_data->MAX_INDES)
    {
      flushQuad();
      reset();
    }

    constexpr const Vec2 tux_coords[4] = 
    {
      {0.0f, 0.0f},
      {1.0f, 0.0f},
      {1.0f, 1.0f},
      {0.0f, 1.0f}
    };
    for(uint8_t i = 0; i < sizeof(tux_coords)/sizeof(tux_coords[0]); ++i)
    {
      s_data->quad_ptr->vertices[i].position = trans * s_data->quad_vertex_pos[i];
      s_data->quad_ptr->vertices[i].color = color;
      s_data->quad_ptr->vertices[i].tux_coord = tux_coords[i];
      s_data->quad_ptr->vertices[i].tux_idx = 0.0f;
      s_data->quad_ptr->vertices[i].entity_id = entity_id;
    } s_data->quad_ptr++;
    
    s_data->quad_index_count += 6;

    s_data->stats.quad_count++;

  }


  void Renderer2D::drawQuad(const Mat4& trans, const ARef<Texture2D>& tux, const Vec4& color, int entity_id)
  {
    if(s_data->quad_index_count >= s_data->MAX_INDES || s_data->texture_slot_index >= s_data->MAX_TEXTURE_SLOTS)
    {
      flushQuad();
      reset();
    }

    float c_tux_slot = 0.0f;
    for(uint8_t i = 1; i < s_data->texture_slot_index; ++i)
    {
      if(isEqual(s_data->texture_slots[i], tux))
      {
        c_tux_slot = (float)i;
        break;
      }
    }

    if(c_tux_slot == 0.0f)
    {
      c_tux_slot = (float)s_data->texture_slot_index;
      s_data->texture_slots[s_data->texture_slot_index] = tux;
      s_data->texture_slot_index++;
    }
   
    //TODO: in future when animations is present you should take the coords from the animation;
    constexpr const Vec2 tux_coords[4] =
    {
      {0.0f, 0.0f},
      {1.0f, 0.0f},
      {1.0f, 1.0f},
      {0.0f, 1.0f}
    };
    
    for(uint8_t i = 0; i < sizeof(tux_coords)/sizeof(tux_coords[0]); ++i)
    {
      s_data->quad_ptr->vertices[i].position = trans * s_data->quad_vertex_pos[i];
      s_data->quad_ptr->vertices[i].color = color;
      s_data->quad_ptr->vertices[i].tux_coord = tux_coords[i];
      s_data->quad_ptr->vertices[i].tux_idx = c_tux_slot;
      s_data->quad_ptr->vertices[i].entity_id = entity_id;
    } s_data->quad_ptr++;
    s_data->quad_index_count += 6;

    s_data->stats.quad_count++;

  }

  void Renderer2D::drawQuad(const Vec3& pos, const Vec2& size, float rotation, const Vec4& color)
  {
    //clock wise starting from bottom left
    //the position is defined from the middle of the quad
    Mat4 trans = glm::translate(Mat4(1.0f), pos);
    trans = glm::rotate(trans, rotation, {0.0f, 0.0f, 1.0f});
    trans = glm::scale(trans, {size.x, size.y, 1.0f});
  
    drawQuad(trans, color);
  }
  
  void Renderer2D::drawQuad(const Vec2& pos, const Vec2& size, float rotation, const Vec4& color)
  {
    drawQuad(Vec3(pos.x, pos.y, 0.0f), size, rotation, color);
  }

  void Renderer2D::drawQuad(const Vec3& pos, const Vec2& size, float rotation, const ARef<Texture2D>& tux, const Vec4& color)
  {
    Mat4 trans = glm::translate(Mat4(1.0f), pos);
    trans = glm::rotate(trans, rotation, {0.0f, 0.0f, 1.0f});
    trans = glm::scale(trans, {size.x, size.y, 1.0f});

    drawQuad(trans, tux, color);
  }
  
  void Renderer2D::drawQuad(const Vec2& pos, const Vec2& size, float rotation, const ARef<Texture2D>& tux, const Vec4& color)
  {
    drawQuad(Vec3(pos.x, pos.y, 0.0f), size, rotation, tux, color);
  }

  void Renderer2D::drawQuad(const Vec3& pos, const Vec2& size, const Vec4& color)
  {
    if(s_data->quad_index_count >= s_data->MAX_INDES || s_data->texture_slot_index >= s_data->MAX_TEXTURE_SLOTS)
    {
      flushQuad();
      reset();
    }

    s_data->quad_ptr->vertices[0].position = pos + Vec3(-size.x/2.0f, -size.y/2.0f, 0.0f);
    s_data->quad_ptr->vertices[0].color = color;
    s_data->quad_ptr->vertices[0].tux_coord = {0.0f, 0.0f};
    s_data->quad_ptr->vertices[0].tux_idx = 0.0f;

    s_data->quad_ptr->vertices[1].position = pos + /*Vec3(size.x, 0.0f, 0.0f);*/Vec3(size.x/2.0f, -size.y/2.0f, 0.0f);
    s_data->quad_ptr->vertices[1].color = color;
    s_data->quad_ptr->vertices[1].tux_coord = {1.0f, 0.0f};
    s_data->quad_ptr->vertices[1].tux_idx = 0.0f;

    s_data->quad_ptr->vertices[2].position = pos + /*Vec3(size.x, size.y, 0.0f);*/Vec3(size.x/2.0f, size.y/2.0f, 0.0f);
    s_data->quad_ptr->vertices[2].color = color;
    s_data->quad_ptr->vertices[2].tux_coord = {1.0f, 1.0f};
    s_data->quad_ptr->vertices[2].tux_idx = 0.0f;

    s_data->quad_ptr->vertices[3].position = pos + /*Vec3(0.0f, size.y, 0.0f);*/Vec3(-size.x/2.0f, size.y/2.0f, 0.0f);
    s_data->quad_ptr->vertices[3].color = color;
    s_data->quad_ptr->vertices[3].tux_coord = {0.0f, 1.0f};
    s_data->quad_ptr->vertices[3].tux_idx = 0.0f;
    
    s_data->quad_ptr++;

    s_data->quad_index_count += 6;

    s_data->stats.quad_count++;
  }

  void Renderer2D::drawQuad(const Vec2& pos, const Vec2& size, const Vec4& color)
  {
    drawQuad(Vec3(pos.x, pos.y, 0.0f), size, color);
  }

  void Renderer2D::drawQuad(const Vec3& pos, const Vec2& size, const ARef<Texture2D>& tux, const Vec4& color)
  {
    if(s_data->quad_index_count >= s_data->MAX_INDES || s_data->texture_slot_index >= s_data->MAX_TEXTURE_SLOTS)
    {
      flushQuad();
      reset();
    }

    float c_tux_slot = 0.0f;
    for(uint8_t i = 1; i < s_data->texture_slot_index; ++i)
    {
      if(isEqual(s_data->texture_slots[i], tux))
      {
        c_tux_slot = (float)i;
        break;
      }
    }

    if(c_tux_slot == 0.0f)
    {
      c_tux_slot = (float)s_data->texture_slot_index;
      s_data->texture_slots[s_data->texture_slot_index] = tux;
      s_data->texture_slot_index++;
    }

    s_data->quad_ptr->vertices[0].position = pos + Vec3(-size.x/2.0f, -size.y/2.0f, 0.0f);
    s_data->quad_ptr->vertices[0].color = color;
    s_data->quad_ptr->vertices[0].tux_coord = {0.0f, 0.0f};
    s_data->quad_ptr->vertices[0].tux_idx = c_tux_slot;

    s_data->quad_ptr->vertices[1].position = pos + /*Vec3(size.x, 0.0f, 0.0f);*/Vec3(size.x/2.0f, -size.y/2.0f, 0.0f);
    s_data->quad_ptr->vertices[1].color = color;
    s_data->quad_ptr->vertices[1].tux_coord = {1.0f, 0.0f};
    s_data->quad_ptr->vertices[1].tux_idx = c_tux_slot;

    s_data->quad_ptr->vertices[2].position = pos + /*Vec3(size.x, size.y, 0.0f);*/Vec3(size.x/2.0f, size.y/2.0f, 0.0f);
    s_data->quad_ptr->vertices[2].color = color;
    s_data->quad_ptr->vertices[2].tux_coord = {1.0f, 1.0f};
    s_data->quad_ptr->vertices[2].tux_idx = c_tux_slot;

    s_data->quad_ptr->vertices[3].position = pos + /*Vec3(0.0f, size.y, 0.0f);*/Vec3(-size.x/2.0f, size.y/2.0f, 0.0f);
    s_data->quad_ptr->vertices[3].color = color;
    s_data->quad_ptr->vertices[3].tux_coord = {0.0f, 1.0f};
    s_data->quad_ptr->vertices[3].tux_idx = c_tux_slot;
    
    s_data->quad_ptr++;

    s_data->quad_index_count += 6;

    s_data->stats.quad_count++;
  }

  void Renderer2D::drawQuad(const Vec2& pos, const Vec2& size, const ARef<Texture2D>& tux, const Vec4& color)
  {
    drawQuad(Vec3(pos.x, pos.y, 0.0f), size, tux, color);
  }

  void Renderer2D::flushQuad()
  {
    uint32_t data_size = ((unsigned char*)s_data->quad_ptr - (unsigned char*)s_data->quad_base) * 4;
    
    //sort for Z position (for blind)
    
    //struct Quad
    //{
     // QuadVertex refs[4];
      /*inline bool operator < (const Quad& other) 
      {
        return this->refs[0]->position.z - s_data->camera_view[3][3] < other.refs[0]->position.z - s_data->camera_view[3][3]; 
      }*/
    //};

  auto* arr = s_data->quad_base;
  uint32_t count = data_size / sizeof(QuadVertex);
     
  //std::sort((Quad*)s_data->quad_vertex_base, (Quad*)s_data->quad_vertex_ptr, 
   //   [](const Quad& lhs, const Quad& rhs)
     // {
      //  return glm::length(s_data->camera_view[3][2] - lhs.refs[0].position.z) > glm::length(s_data->camera_view[3][2] - rhs.refs[0].position.z); 
     // });
     //
     std::ranges::sort(s_data->quad_base, s_data->quad_ptr, 
      [](const Quad& lhs, const Quad& rhs)
      {
        //auto f = s_data->camera_view[3][2] - lhs.vertices[0].position.z;
        //auto s = s_data->camera_view[3][2] - rhs.vertices[0].position.z; 
        //return std::abs(f) < std::abs(s);  
        auto f = s_data->camera_view[3][2] - lhs.vertices[0].position.z;
        auto s = s_data->camera_view[3][2] - rhs.vertices[0].position.z; 
        if((lhs.vertices[0].color.a > 250) && (rhs.vertices[0].color.a > 255)) // both not transparent
        { 
          return std::abs(f) < std::abs(s);
        }
        else if((lhs.vertices[0].color.a < 250) ^ (rhs.vertices[0].color.a < 250)) //one is transparent
        {
          if(lhs.vertices[0].color.a < 250) // lhs is transparent
          {
            return false;
          }
          else //if(rhs.vertices[0].color.a < 250) // rhs is transparent
          {
            return true;
          }
        }
        else //both are transparent
        {
          return std::abs(f) < std::abs(s);
        }

      });



    //std::sort(arr, arr);
//for(size_t i = 0; i < count; ++i)
  //  for(size_t j = 0; j < count - 1 - i; ++j)
    //  if(arr[j] > arr[j+1])
      //  std::swap(arr[j], arr[j+1]);  
      //  REAL_CORE_WARN("vertex count: {0}, quad_count: {1}, first: << pos: {2}, color: {3}, tux_coord: {4}, tux_idx: {5} >>", count, count/4, arr->vertices[0].position, arr->vertices[0].color, arr->vertices[0].tux_coord, arr->vertices[0].tux_idx);
    //std::sort(s_data->quad_vertex_base, s_data->quad_vertex_base + (data_size / sizeof(QuadVertex)));

      //s_data->quad_vertex_arr->getIndexBuffer();
    /* std::sort(s_data->quad_vertex_base, s_data->quad_vertex_ptr, [&](const QuadVertex& lhs, const QuadVertex& rhs)
    {
      return lhs.position.z - s_data->camera_view[3][3] < rhs.position.z - s_data->camera_view[3][3]; 
    });*/
    s_data->quad_vertex_buff->setData(data_size, s_data->quad_base);

    for(uint8_t i = 0; i < s_data->texture_slot_index; ++i)
    {
      s_data->texture_slots[i]->bind(i);
    }

    s_data->shaders.get("tux_shader")->bind();
    s_data->shaders.get("tux_shader")->setUniform("u_proj_view", s_data->proj_view);
    RenderCommands::drawIndexed(s_data->quad_vertex_arr, s_data->quad_index_count);
    s_data->stats.draw_calls++;
  }
   
  void Renderer2D::flushCircle()
  {
    uint32_t data_size = ((unsigned char*)s_data->circle_ptr - (unsigned char*)s_data->circle_base) * 4;
    
    auto* arr = s_data->circle_base;
    uint32_t count = data_size / sizeof(CircleVertex);
    std::ranges::sort(s_data->circle_base, s_data->circle_ptr, 
      [](const Circle& lhs, const Circle& rhs)
      {
        auto f = s_data->camera_view[3][2] - lhs.vertices[0].position.z;
        auto s = s_data->camera_view[3][2] - rhs.vertices[0].position.z; 
        if((lhs.vertices[0].color.a > 250) && (rhs.vertices[0].color.a > 255)) // both not transparent
        { 
          return std::abs(f) < std::abs(s);
        }
        else if((lhs.vertices[0].color.a < 250) ^ (rhs.vertices[0].color.a < 250)) //one is transparent
        {
          if(lhs.vertices[0].color.a < 250) // lhs is transparent
          {
            return true;
          }
          else //if(rhs.vertices[0].color.a < 250) // rhs is transparent
          {
            return false;
          }
        }
        else //both are transparent
        {
          return std::abs(f) < std::abs(s);
        }
      });

    //REAL_CORE_WARN("vertex count: {0}, quad_count: {1}, first: << pos: {2}, color: {3} >>", count, count/4, arr->vertices[0].position, arr->vertices[0].color);
    
    s_data->circle_vertex_buff->setData(data_size, s_data->circle_base);

    s_data->shaders.get("circle_shader")->bind();
    s_data->shaders.get("circle_shader")->setUniform("u_proj_view", s_data->proj_view);
    RenderCommands::drawIndexed(s_data->circle_vertex_array, s_data->circle_index_count);
    s_data->stats.draw_calls++;
  }

  void Renderer2D::endScene()
  {
    flushQuad();
    flushCircle();
    REAL_CORE_INFO("Scene end");
  }

  void Renderer2D::shutDown()
  {
    delete[] s_data->texture_slots;
    delete s_data;
  }

}
