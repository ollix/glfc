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

#include "glfc/gaussian_blur_filter.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <string>

#include "glfc/framebuffer.h"
#include "glfc/opengl_hook.h"
#include "glfc/program.h"

namespace glfc {

GaussianBlurFilter::GaussianBlurFilter() : blur_radius_(4),
                                           sigma_(2),
                                           texel_height_offset_(0),
                                           texel_spacing_multiplier_(1),
                                           texel_width_offset_(0) {
}

GaussianBlurFilter::~GaussianBlurFilter() {
}

std::string GaussianBlurFilter::GetFragmentShader() const {
  // First, generate the normal Gaussian weights for a given `sigma_`.
  float* standard_gaussian_weights = reinterpret_cast<float*>(
      std::calloc(blur_radius_ + 1, sizeof(float)));
  float sum_of_weights = 0.0;
  for (int current_gaussian_weight_index = 0;
       current_gaussian_weight_index < blur_radius_ + 1;
       current_gaussian_weight_index++) {
    standard_gaussian_weights[current_gaussian_weight_index] = \
        (1.0 / std::sqrt(2.0 * M_PI * std::pow(sigma_, 2.0))) *
        std::exp(-std::pow(current_gaussian_weight_index, 2.0) /
                 (2.0 * std::pow(sigma_, 2.0)));

    if (current_gaussian_weight_index == 0) {
      sum_of_weights += \
          standard_gaussian_weights[current_gaussian_weight_index];
    } else {
      sum_of_weights += \
          2.0 * standard_gaussian_weights[current_gaussian_weight_index];
    }
  }

  // Next, normalize these weights to prevent the clipping of the Gaussian
  // curve at the end of the discrete samples from reducing luminance.
  for (int current_gaussian_weight_index = 0;
       current_gaussian_weight_index < blur_radius_ + 1;
       current_gaussian_weight_index++) {
    standard_gaussian_weights[current_gaussian_weight_index] = \
        standard_gaussian_weights[current_gaussian_weight_index] /
        sum_of_weights;
  }

  // From these weights we calculate the offsets to read interpolated values
  // from.
  const int kNumberOfOptimizedOffsets = \
      std::min(blur_radius_ / 2 + (blur_radius_ % 2), 7);
  const int kTruekNumberOfOptimizedOffsets = \
      blur_radius_ / 2 + (blur_radius_ % 2);

  std::string shader_string;
  // Header
  const char* kHeaderFormat =
#ifdef GLFC_IOS
R"(uniform sampler2D inputImageTexture;
uniform highp float texelWidthOffset;
uniform highp float texelHeightOffset;

varying highp vec2 blurCoordinates[%lu];

void main() {
  lowp vec4 sum = vec4(0.0);)";
#else
R"(uniform sampler2D inputImageTexture;
uniform float texelWidthOffset;
uniform float texelHeightOffset;

varying vec2 blurCoordinates[%lu];

void main() {
  vec4 sum = vec4(0.0);)";
#endif
  const int kNumberOfBlurCoordinates = 1 + (kNumberOfOptimizedOffsets * 2);
  const int kHeaderLength = snprintf(NULL, 0, kHeaderFormat,
                                     kNumberOfBlurCoordinates) + 1;
  char header[kHeaderLength];
  snprintf(header, kHeaderLength, kHeaderFormat, kNumberOfBlurCoordinates);
  shader_string.append(header);

  // Inner texture loop.
  const float kStandardGaussianWeight = standard_gaussian_weights[0];
  const char* kInnerTextureLoopFirstLineFormat = R"(
  sum += texture2D(inputImageTexture, blurCoordinates[0]) * %f;)";
  const int kInnerTextureLoopFirstLineLength = \
      snprintf(NULL, 0, kInnerTextureLoopFirstLineFormat,
      kStandardGaussianWeight) + 1;
  char inner_texture_loop_first_line[kInnerTextureLoopFirstLineLength];
  snprintf(inner_texture_loop_first_line, kInnerTextureLoopFirstLineLength,
           kInnerTextureLoopFirstLineFormat, kStandardGaussianWeight);
  shader_string.append(inner_texture_loop_first_line);

  const char* kInnerTextureLoopFormat = R"(
  sum += texture2D(inputImageTexture, blurCoordinates[%d]) * %f;
  sum += texture2D(inputImageTexture, blurCoordinates[%d]) * %f;)";
  for (int current_blur_coordinate_index = 0;
       current_blur_coordinate_index < kNumberOfOptimizedOffsets;
       current_blur_coordinate_index++) {
    const float kFirstWeight = \
        standard_gaussian_weights[current_blur_coordinate_index * 2 + 1];
    const float kSecondWeight = \
        standard_gaussian_weights[current_blur_coordinate_index * 2 + 2];
    const float kOptimizedWeight = kFirstWeight + kSecondWeight;

    const int kFirstIndex = current_blur_coordinate_index * 2 + 1;
    const int kSecondIndex = current_blur_coordinate_index * 2 + 2;

    const int kStringLength = snprintf(NULL, 0, kInnerTextureLoopFormat,
                                       kFirstIndex, kOptimizedWeight,
                                       kSecondIndex, kOptimizedWeight) + 1;
    char string[kStringLength];
    snprintf(string, kStringLength, kInnerTextureLoopFormat, kFirstIndex,
             kOptimizedWeight, kSecondIndex, kOptimizedWeight);
    shader_string.append(string);
  }

  // If the number of required samples exceeds the amount we can pass in via
  // varyings, we have to do dependent texture reads in the fragment shader.
  if (kTruekNumberOfOptimizedOffsets > kNumberOfOptimizedOffsets) {
#if GLFC_IOS
    shader_string.append(R"(
  highp vec2 singleStepOffset = vec2(texelWidthOffset, texelHeightOffset);)");
#else
    shader_string.append(R"(
  vec2 singleStepOffset = vec2(texelWidthOffset, texelHeightOffset);)");
#endif

    const char* kInnerTextureLoopFormat = R"(
  sum += texture2D(inputImageTexture, blurCoordinates[0] + singleStepOffset * %f) * %f;
  sum += texture2D(inputImageTexture, blurCoordinates[0] - singleStepOffset * %f) * %f;)";
    for (int current_overlow_texture_read = kNumberOfOptimizedOffsets;
         current_overlow_texture_read < kTruekNumberOfOptimizedOffsets;
         current_overlow_texture_read++) {
      const float kFirstWeight = \
          standard_gaussian_weights[current_overlow_texture_read * 2 + 1];
      const float kSecondWeight = \
          standard_gaussian_weights[current_overlow_texture_read * 2 + 2];

      const float kOptimizedWeight = kFirstWeight + kSecondWeight;
      const float kOptimizedOffset = \
          (kFirstWeight * (current_overlow_texture_read * 2 + 1) +
              kSecondWeight * (current_overlow_texture_read * 2 + 2))
          / kOptimizedWeight;

      const int kStringLength = \
          snprintf(NULL, 0, kInnerTextureLoopFormat, kOptimizedOffset,
                   kOptimizedWeight, kOptimizedOffset, kOptimizedWeight) + 1;
      char string[kStringLength];
      snprintf(string, kStringLength, kInnerTextureLoopFormat, kOptimizedOffset,
               kOptimizedWeight, kOptimizedOffset, kOptimizedWeight);
      shader_string.append(string);
    }
  }

  // Footer
  shader_string.append(R"(
  gl_FragColor = sum;
})");

  std::free(standard_gaussian_weights);
  return shader_string;
}

std::string GaussianBlurFilter::GetVertexShader() const {
  // First, generate the normal Gaussian weights for a given `sigma_`.
  float* standard_gaussian_weights = reinterpret_cast<float*>(
      std::calloc(blur_radius_ + 1, sizeof(float)));
  float sum_of_weights = 0.0;
  for (int current_gaussian_weight_index = 0;
       current_gaussian_weight_index < blur_radius_ + 1;
       current_gaussian_weight_index++) {
    standard_gaussian_weights[current_gaussian_weight_index] = \
        (1.0 / std::sqrt(2.0 * M_PI * std::pow(sigma_, 2.0)))
        * std::exp(-std::pow(current_gaussian_weight_index, 2.0)
                   / (2.0 * std::pow(sigma_, 2.0)));

    if (current_gaussian_weight_index == 0) {
      sum_of_weights += \
          standard_gaussian_weights[current_gaussian_weight_index];
    } else {
        sum_of_weights += \
            2.0 * standard_gaussian_weights[current_gaussian_weight_index];
    }
  }

  // Next, normalize these weights to prevent the clipping of the Gaussian
  // curve at the end of the discrete samples from reducing luminance.
  for (int current_gaussian_weight_index = 0;
       current_gaussian_weight_index < blur_radius_ + 1;
       current_gaussian_weight_index++) {
    standard_gaussian_weights[current_gaussian_weight_index] = \
        standard_gaussian_weights[current_gaussian_weight_index] /
        sum_of_weights;
  }

  // From these weights we calculate the offsets to read interpolated values
  // from.
  const int kNumberOfOptimizedOffsets = \
      std::min(blur_radius_ / 2 + (blur_radius_ % 2), 7);
  float* optimized_gaussian_offsets = reinterpret_cast<float*>(
      std::calloc(kNumberOfOptimizedOffsets, sizeof(float)));

  for (int current_optimized_offset = 0;
       current_optimized_offset < kNumberOfOptimizedOffsets;
       current_optimized_offset++) {
    const float kFirstWeight = \
        standard_gaussian_weights[current_optimized_offset * 2 + 1];
    const float kSecondWeight = \
        standard_gaussian_weights[current_optimized_offset * 2 + 2];
    const float kOptimizedWeight = kFirstWeight + kSecondWeight;

    optimized_gaussian_offsets[current_optimized_offset] = \
        (kFirstWeight * (current_optimized_offset * 2 + 1)
            + kSecondWeight * (current_optimized_offset * 2 + 2))
        / kOptimizedWeight;
  }

  std::string shader_string;
  // Header
  const char* kHeaderFormat = \
R"(attribute vec4 position;
attribute vec2 inputTextureCoordinate;

uniform float texelWidthOffset;
uniform float texelHeightOffset;

varying vec2 blurCoordinates[%d];

void main() {
  gl_Position = position;

  vec2 singleStepOffset = vec2(texelWidthOffset, texelHeightOffset);)";
  const int kNumberOfBlurCoordinates = 1 + (kNumberOfOptimizedOffsets * 2);
  const int kHeaderLength = snprintf(NULL, 0, kHeaderFormat,
                                     kNumberOfBlurCoordinates) + 1;
  char header[kHeaderLength];
  snprintf(header, kHeaderLength, kHeaderFormat, kNumberOfBlurCoordinates);
  shader_string.append(header);

  // Inner offset loop.
  shader_string.append(R"(
  blurCoordinates[0] = inputTextureCoordinate.xy;)");
  const char* kInnerOffsetLoopFormat = R"(
  blurCoordinates[%d] = inputTextureCoordinate.xy + singleStepOffset * %f;
  blurCoordinates[%d] = inputTextureCoordinate.xy - singleStepOffset * %f;)";    for (int current_optimized_offset = 0;
       current_optimized_offset < kNumberOfOptimizedOffsets;
       current_optimized_offset++) {
    const int kFirstIndex = (current_optimized_offset * 2) + 1;
    const int kSecondIndex = (current_optimized_offset * 2) + 2;
    const float kOptimizedGaussianOffset = \
        optimized_gaussian_offsets[current_optimized_offset];

    const int kStringLength = \
        snprintf(NULL, 0, kInnerOffsetLoopFormat, kFirstIndex,
                 kOptimizedGaussianOffset, kSecondIndex,
                 kOptimizedGaussianOffset) + 1;
    char string[kStringLength];
    snprintf(string, kStringLength, kInnerOffsetLoopFormat, kFirstIndex,
             kOptimizedGaussianOffset, kSecondIndex, kOptimizedGaussianOffset);
    shader_string.append(string);
  }

  // Footer
  shader_string.append(R"(
})");

  std::free(optimized_gaussian_offsets);
  std::free(standard_gaussian_weights);
  return shader_string;
}

void GaussianBlurFilter::Render(const GLuint input_texture, const int width,
                                const int height) {
  // Initializes the intermediate framebuffer.
  Framebuffer framebuffer(width, height);
  if (!framebuffer.Init()) {
#ifdef DEBUG
    fprintf(stderr, "!! Failed to initialize framebuffer.\n");
#endif
    return;
  }

  // Initializes the program with shaders.
  Program program;
  if (!program.Init(GetVertexShader(), GetFragmentShader())) {
#ifdef DEBUG
    fprintf(stderr, "!! Failed to initialize program.\n");
#endif
    return;
  }

  // First pass. Applies Gaussian blur to the input texture for horizontal
  // direction.
  framebuffer.Bind();
  texel_width_offset_ = texel_spacing_multiplier_ / width;
  texel_height_offset_ = 0;
  SetUniforms(program.program());
  program.Render(input_texture);

  // Second pass. Applies Gaussian blur to the `framebuffer`'s internal texture
  // for vertical direction.
  texel_width_offset_ = 0;
  texel_height_offset_ = texel_spacing_multiplier_ / height;
  SetUniforms(program.program());
  framebuffer.UpdateTexture(&program);
  program.Finalize();
  framebuffer.Unbind();

  // Renders the current framebuffer object.
  framebuffer.Render();
}

void GaussianBlurFilter::SetUniforms(GLuint program) const {
  GLint texel_width_offset_uniform = \
      glGetUniformLocation(program, "texelWidthOffset");
  glUniform1f(texel_width_offset_uniform, texel_width_offset_);

  GLint texel_height_offset_uniform = \
      glGetUniformLocation(program, "texelHeightOffset");
  glUniform1f(texel_height_offset_uniform, texel_height_offset_);
}

}  // namespace glfc
