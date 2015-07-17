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

#ifndef GLFC_PROGRAM_H_
#define GLFC_PROGRAM_H_

#include <string>

#include "glfc/base.h"
#include "glfc/opengl_hook.h"

namespace glfc {

// This class manages an OpenGL program object for rendering an input texture
// with configured shaders to the currently binded framebuffer.
//
// The shaders passed to this class must declare some attributes and uniforms
// described as follow so this class knows how to deal with them properly.
// For vertex shader, both `vec4 position` and `vec2 inputTextureCoordinate`
// attributes must be declared. As for fragement shader, only the
// `sampler2D inputImageTexture` uniform is required. If you declare other
// attributes or uniforms, you must set the values for them between the
// `Init()` and `Render()` calls.
class Program {
 public:
  Program();
  ~Program();

  // Initializes the program. The `Finalize()` method must be called before
  // another `Init()` call.
  bool Init(const std::string vertex_shader, const std::string fragment_shader);

  // Finalizes the program. The `Init()` method must be called before calling
  // this method.
  void Finalize();

  // Renders the `input_texture` with configured shaders to the currently
  // binded framebuffer.
  void Render(const GLuint input_texture);

  // Accessors.
  GLuint program() const { return program_; }

 private:
  // Binds both array buffer and index buffer objects.
  void BindBufferObjects();

  // Resets the states.
  void Reset();

  // The array buffer object name.
  GLuint array_buffer_;

  // The fragment shader name.
  GLuint fragment_shader_;

  // The index buffer object name.
  GLuint index_buffer_;

  // The location of the `position` attribute defined in the vertex shader.
  GLint position_attribute_;

  // The program name.
  GLuint program_;

  // The location of the `inputTextureCoordinate` attribute defined in the
  // vertex shader.
  GLint texture_coordinate_attribute_;

  // The vertex shader name.
  GLuint vertex_shader_;

  DISALLOW_COPY_AND_ASSIGN(Program);
};

}  // namespace glfc

#endif  // GLFC_PROGRAM_H_
