// Copyright (c) 2015 Ollix. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// ---
// Author: olliwang@ollix.com (Olli Wang)

#include "glfc/program.h"

#include <cstdlib>
#include <cstdio>

#include "glfc/base.h"
#include "glfc/opengl_hook.h"

namespace {

struct FramebufferCoordinate {
  float x;
  float y;
};

struct TextureCoordinate {
  float s;
  float t;
};

struct Vertex {
  FramebufferCoordinate framebuffer_coordinate;
  TextureCoordinate texture_coordinate;
};

const Vertex kArrayBuffer[4] = {{{-1.0, 1.0}, {0.0, 1.0}},  // top-left
                                {{-1.0, -1.0}, {0.0, 0.0}},  // bottom-left
                                {{1.0, -1.0}, {1.0, 0.0}},  // bottom-right
                                {{1.0, 1.0}, {1.0, 1.0}}};  // top-right

const int kIndexBufferCount = 6;
const int kIndexBuffer[kIndexBufferCount] = {0, 1, 2, 0, 2, 3};

GLuint CompileShader(const GLenum shader_type, std::string source) {
  GLuint shader_handle = glCreateShader(shader_type);
  if (shader_handle == 0)
    return 0;

  const GLchar* source_string = static_cast<const GLchar*>(source.c_str());
  glShaderSource(shader_handle, 1, &source_string, NULL);
  glCompileShader(shader_handle);
  GLint compile_result;
  glGetShaderiv(shader_handle, GL_COMPILE_STATUS, &compile_result);
  if (compile_result == GL_FALSE) {
#if DEBUG
    int info_log_length;
    glGetShaderiv(shader_handle, GL_INFO_LOG_LENGTH, &info_log_length);
    GLchar* message = reinterpret_cast<GLchar*>(std::malloc(info_log_length));
    glGetShaderInfoLog(shader_handle, static_cast<GLsizei>(info_log_length), 0,
                       message);
    GLFC_LOG("!! Failed to compile shader: %s", message);
    std::free(message);
#endif
    glDeleteShader(shader_handle);
    shader_handle = 0;
  }
  return shader_handle;
}

}  // namespace

namespace glfc {

Program::Program() : array_buffer_(0), fragment_shader_(0), index_buffer_(0),
                     is_initialized_(false), program_(0), vertex_shader_(0) {
}

Program::~Program() {
  if (is_initialized_)
    Finalize();
}

bool Program::Init(const std::string vertex_shader_source,
                   const std::string fragment_shader_source) {
  if (is_initialized_) {
    Finalize();
  }

  program_ = glCreateProgram();
  if (program_ == 0) {
    Finalize();
    return false;
  }

  vertex_shader_ = CompileShader(GL_VERTEX_SHADER, vertex_shader_source);
  if (vertex_shader_ == 0) {
    Finalize();
#ifdef DEBUG
    GLFC_LOG("--- Vertex Shader Source ---\n%s\n--- END ---\n",
            vertex_shader_source.c_str());
#endif
    return false;
  }
  fragment_shader_ = CompileShader(GL_FRAGMENT_SHADER, fragment_shader_source);
  if (fragment_shader_ == 0) {
    Finalize();
#ifdef DEBUG
    GLFC_LOG("--- Fragment Shader Source ---\n%s\n--- END ---\n",
            fragment_shader_source.c_str());
#endif
    return false;
  }

  glAttachShader(program_, vertex_shader_);
  glAttachShader(program_, fragment_shader_);
  glLinkProgram(program_);
  GLint status;
  glGetProgramiv(program_, GL_LINK_STATUS, &status);
  if (status != GL_TRUE) {
    Finalize();
#ifdef DEBUG
    GLFC_LOG("!! Failed to create program.\n");
#endif
    return false;
  }
  GLint current_program;
  glGetIntegerv(GL_CURRENT_PROGRAM, &current_program);
  glUseProgram(program_);
  position_attribute_ = glGetAttribLocation(program_, "position");
  texture_coordinate_attribute_ = glGetAttribLocation(
      program_, "inputTextureCoordinate");
  texture_uniform_ = glGetUniformLocation(program_, "inputImageTexture");
  glGenBuffers(1, &array_buffer_);
  glGenBuffers(1, &index_buffer_);
  glUseProgram(current_program);
  is_initialized_ = true;
  return true;
}

void Program::Finalize() {
  if (array_buffer_ > 0) {
    glDeleteBuffers(1, &array_buffer_);
  }
  if (index_buffer_ > 0) {
    glDeleteBuffers(1, &index_buffer_);
  }
  if (vertex_shader_ > 0) {
    glDeleteShader(vertex_shader_);
    vertex_shader_ = 0;
  }
  if (fragment_shader_ > 0) {
    glDeleteShader(fragment_shader_);
    fragment_shader_ = 0;
  }
  if (program_ > 0) {
    glDeleteProgram(program_);
    program_ = 0;
  }
  is_initialized_ = false;
}

void Program::Render(const GLuint input_texture) {
  // Sets the texture uniform.
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, input_texture);
  glUniform1i(texture_uniform_, 0);

  glDrawElements(GL_TRIANGLES, static_cast<GLsizeiptr>(kIndexBufferCount),
                 GL_UNSIGNED_INT, reinterpret_cast<GLvoid*>(0));

  glDisableVertexAttribArray(position_attribute_);
  glDisableVertexAttribArray(texture_coordinate_attribute_);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glUseProgram(0);
  glBindTexture(GL_TEXTURE_2D, 0);
  glFlush();
}

void Program::Use() {
  glUseProgram(program_);
  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
  glEnable(GL_BLEND);

  // Binds the array buffer object.
  glBindBuffer(GL_ARRAY_BUFFER, array_buffer_);
  glBufferData(GL_ARRAY_BUFFER, sizeof(kArrayBuffer), kArrayBuffer,
               GL_STATIC_DRAW);

  // Binds the `position` attribute.
  glEnableVertexAttribArray(position_attribute_);
  glVertexAttribPointer(position_attribute_, 2, GL_FLOAT, GL_FALSE,
                        sizeof(Vertex), reinterpret_cast<GLvoid*>(0));

  // Binds the `inputTextureCoordinate` attribute.
  glEnableVertexAttribArray(texture_coordinate_attribute_);
  glVertexAttribPointer(
      texture_coordinate_attribute_, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
      reinterpret_cast<GLvoid*>(sizeof(FramebufferCoordinate)));

  // Binds the index buffer object.
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * kIndexBufferCount,
               kIndexBuffer, GL_STATIC_DRAW);
}

}  // namespace glfc
