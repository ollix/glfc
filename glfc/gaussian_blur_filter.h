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

#ifndef GLFC_GAUSSIAN_BLUR_FILTER_H_
#define GLFC_GAUSSIAN_BLUR_FILTER_H_

#include <algorithm>
#include <string>

#include "glfc/base.h"
#include "glfc/filter.h"
#include "glfc/opengl_hook.h"

namespace glfc {

// This class implements the Gaussian blur effect. The shaders used in this
// class are ported from GPUImage's `GPUImageiOSBlurFilter` class. The source
// code can be found at http://git.io/vmKcw.
class GaussianBlurFilter : public Filter {
 public:
  GaussianBlurFilter();
  ~GaussianBlurFilter();

  // Inherited from `Filter` class.
  void Render(const GLuint input_texture, const int width,
              const int height) final;

  // Setters and accessors.
  int blur_radius() const { return blur_radius_; }
  void set_blur_radius(const int radius) { blur_radius_ = std::max(1, radius); }
  float sigma() const { return sigma_; }
  void set_sigma(const float sigma) { sigma_ = sigma; }
  float texel_spacing_multiplier() const { return texel_spacing_multiplier_; }
  void set_texel_spacing_multiplier(const float texel_spacing_multiplier) {
    texel_spacing_multiplier_ = texel_spacing_multiplier;
  }

 private:
  // Inherited from `Filter` class.
  std::string GetFragmentShader() const final;

  // Inherited from `Filter` class.
  std::string GetVertexShader() const final;

  // Inherited from `Filter` class.
  void SetUniforms(GLuint program) const final;

  // The radius in pixels to use for the blur, with a default of 2. This value
  // must be equal or greater than 1.
  int blur_radius_;

  // The sigma variable used in Gaussian distribution function for calculating
  // the Gaussian weights.
  float sigma_;

  // Indicates the vertical offset of a single step used in the vertex shader.
  float texel_height_offset_;

  // A multiplier for the spacing between texels, ranging from 0.0 on up, with
  // a default of 1.0. Adjusting this value may slightly increase the blur
  // strength but will introduce artifacs in the result.
  float texel_spacing_multiplier_;

  // Indicates the horizontal offset of a single step used in the vertex shader.
  float texel_width_offset_;

  DISALLOW_COPY_AND_ASSIGN(GaussianBlurFilter);
};

}  // namespace glfc

#endif  // GLFC_GAUSSIAN_BLUR_FILTER_H_
