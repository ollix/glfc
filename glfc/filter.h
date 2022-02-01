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

#ifndef GLFC_FILTER_H_
#define GLFC_FILTER_H_

#include <string>

#include "glfc/base.h"
#include "glfc/opengl_hook.h"

namespace glfc {

class Framebuffer;
class Program;

// This is the base class of all supported filters.
class Filter {
 public:
  Filter();
  virtual ~Filter();

  // Renders the filter with `input_texture` and its dimension to the
  // framebuffer that is currently binded to OpenGL. This method is designed
  // specifically for one pass rendering. A `Filter` subclass can override
  // this method to implement two pass rendering or combine multiple filters.
  bool Render(const GLuint input_texture, const float width,
              const float height, const float device_pixel_ratio);

 protected:
  // Applies the filter to the specified `framebuffer`.
  virtual void ApplyFilterToFramebuffer(const GLuint input_texture,
                                        Program* program,
                                        Framebuffer* framebuffer);

  // Returns `true` if the corresponded shaders should update.
  virtual bool ShouldUpdateShaders() const { return false; }

  // Setters and accessors.
  float device_pixel_ratio() const { return device_pixel_ratio_; }
  virtual void set_device_pixel_ratio(const float device_pixel_ratio) {
    device_pixel_ratio_ = device_pixel_ratio;
  }

 private:
  // Returns the fragment shader string. The returned shader must declare the
  // `sampler2D inputImageTexture` uniform.
  virtual std::string GetFragmentShader() const = 0;

  // Returns the vertex shader string. The returned shader must declare both
  // `vec4 position` and `vec2 inputTextureCoordinate` attributes.
  virtual std::string GetVertexShader() const = 0;

  // Sets uniforms used in shaders except the `inputImageTexture` one.
  virtual void SetUniforms(Program* program) const {}

  // Indicates the ratio between physical pixels and logical pixels. This value
  // will be updated whenever `Render()` is called. The default value is 1.
  float device_pixel_ratio_;

  // The strong reference to the framebuffer that holds the result.
  Framebuffer* framebuffer_;

  // The strong reference to the program that utilizing filter shaders.
  Program* program_;

  GLFC_DISALLOW_COPY_AND_ASSIGN(Filter);
};

}  // namespace glfc

#endif  // GLFC_FILTER_H_
