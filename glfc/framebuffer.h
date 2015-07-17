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

#ifndef GLFC_FRAMEBUFFER_H_
#define GLFC_FRAMEBUFFER_H_

#include "glfc/base.h"
#include "glfc/opengl_hook.h"

namespace glfc {

// Forward declaration.
class Program;

// This class manages the life cycle of an OpenGL framebuffer object that is
// designed to render to a texutre.
class Framebuffer {
 public:
  Framebuffer(const int width, const int height);
  ~Framebuffer();

  // Initializes the framebuffer object and corresponded renderbuffer and
  // texture objects. Returns `false` on failure.
  bool Init();

  // Binds the framebuffer object.
  void Bind();

  // Renders the internal texture to the framebuffer that is currently binded
  // to OpenGL. This method should be called when the framebuffer instance
  // itself is not binded.
  void Render() const;

  // Unbinds the framebuffer object and resotres the original one.
  void Unbind() const;

  // Updates the internal texture by applying `program`'s shaders.
  void UpdateTexture(Program* program);

  // Accessors.
  GLuint texture() const { return texture_; }

 private:
  // Resets the states.
  void Reset();

  // Records whether `GL_BLEND` is enabled when binding `framebuffer_`.
  GLboolean blend_is_enabled_;

  // The framebuffer object name.
  GLuint framebuffer_;

  // The height of the framebuffer.
  const int height_;

  // The renderbuffer object name.
  GLuint renderbuffer_;

  // The texture name.
  GLuint texture_;

  // The width of the framebuffer.
  const int width_;

  // The name of the original framebuffer before binding.
  GLint original_framebuffer_;

  DISALLOW_COPY_AND_ASSIGN(Framebuffer);
};

}  // namespace glfc

#endif  // GLFC_FRAMEBUFFER_H_
