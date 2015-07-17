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
    fprintf(stderr, "!! Failed to compile shader: %s", message);
    std::free(message);
#endif
    glDeleteShader(shader_handle);
    shader_handle = 0;
  }
  return shader_handle;
}

}  // namespace

namespace glfc {

Program::Program() : fragment_shader_(0), program_(0), vertex_shader_(0) {
}

Program::~Program() {
  Reset();
}

bool Program::Init(const std::string vertex_shader_source,
                   const std::string fragment_shader_source) {
  program_ = glCreateProgram();
  if (program_ == 0) {
    Reset();
    return false;
  }

  vertex_shader_ = CompileShader(GL_VERTEX_SHADER, vertex_shader_source);
  if (vertex_shader_ == 0) {
    Reset();
#if DEBUG
    fprintf(stderr, "--- Vertex Shader Source ---\n%s\n--- END ---\n",
            vertex_shader_source.c_str());
#endif
    return false;
  }
  fragment_shader_ = CompileShader(GL_FRAGMENT_SHADER, fragment_shader_source);
  if (fragment_shader_ == 0) {
    Reset();
#if DEBUG
    fprintf(stderr, "--- Fragment Shader Source ---\n%s\n--- END ---\n",
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
    Reset();
#if DEBUG
    fprintf(stderr, "!! Failed to create program.\n");
#endif
    return false;
  }
  glUseProgram(program_);
  BindBufferObjects();
  return true;
}

void Program::BindBufferObjects() {
  // Binds the array buffer object.
  glGenBuffers(1, &array_buffer_);
  glBindBuffer(GL_ARRAY_BUFFER, array_buffer_);
  glBufferData(GL_ARRAY_BUFFER, sizeof(kArrayBuffer), kArrayBuffer,
               GL_STATIC_DRAW);

  // Binds the `position` attribute.
  position_attribute_ = glGetAttribLocation(program_, "position");
  glEnableVertexAttribArray(position_attribute_);
  glVertexAttribPointer(position_attribute_, 2, GL_FLOAT, GL_FALSE,
                        sizeof(Vertex), reinterpret_cast<GLvoid*>(0));

  // Binds the `inputTextureCoordinate` attribute.
  texture_coordinate_attribute_ = glGetAttribLocation(
      program_, "inputTextureCoordinate");
  glEnableVertexAttribArray(texture_coordinate_attribute_);
  glVertexAttribPointer(
      texture_coordinate_attribute_, 2, GL_FLOAT, GL_FLOAT, sizeof(Vertex),
      reinterpret_cast<GLvoid*>(sizeof(FramebufferCoordinate)));

  // Binds the index buffer object.
  glGenBuffers(1, &index_buffer_);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * kIndexBufferCount,
               kIndexBuffer, GL_STATIC_DRAW);
}

void Program::Finalize() {
  glDisableVertexAttribArray(position_attribute_);
  glDisableVertexAttribArray(texture_coordinate_attribute_);
  glDeleteBuffers(1, &array_buffer_);
  glDeleteBuffers(1, &index_buffer_);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glUseProgram(0);
  Reset();
}

void Program::Render(const GLuint input_texture) {
  // Sets the `texture` uniform.
  GLint texture_uniform = glGetUniformLocation(program_, "inputImageTexture");
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, input_texture);
  glUniform1i(texture_uniform, 0);

  glDrawElements(GL_TRIANGLES, static_cast<GLsizeiptr>(kIndexBufferCount),
                 GL_UNSIGNED_INT, reinterpret_cast<GLvoid*>(0));
}

void Program::Reset() {
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
}

}  // namespace glfc
