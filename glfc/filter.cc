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

#include "glfc/filter.h"

#include <cstdio>
#include <string>

#include "glfc/base.h"
#include "glfc/framebuffer.h"
#include "glfc/opengl_hook.h"
#include "glfc/program.h"

namespace glfc {

Filter::Filter() : device_pixel_ratio_(1), framebuffer_(nullptr),
                   program_(new Program) {
}

Filter::~Filter() {
  if (framebuffer_ != nullptr) {
    delete framebuffer_;
  }
  delete program_;
}

void Filter::ApplyFilterToFramebuffer(const GLuint input_texture,
                                      Program* program,
                                      Framebuffer* framebuffer) {
  program->Use();
  SetUniforms(program);
  program->Render(input_texture);
}

bool Filter::Render(const GLuint input_texture, const float width,
                    const float height, const float device_pixel_ratio) {
  const int kWidth = width * device_pixel_ratio;
  const int kHeight = height * device_pixel_ratio;
  set_device_pixel_ratio(device_pixel_ratio);
  if (framebuffer_ != nullptr &&
      (framebuffer_->width() != kWidth || framebuffer_->height() != kHeight)) {
    delete framebuffer_;
    framebuffer_ = nullptr;
  }
  if (framebuffer_ == nullptr) {
    framebuffer_ = new Framebuffer(kWidth, kHeight);
    if (!framebuffer_->Init()) {
#ifdef DEBUG
      GLFC_LOG("!! Failed to initialize framebuffer.\n");
#endif
      return false;
    }
  }

  if (!program_->is_initialized() || ShouldUpdateShaders()) {
    if (!program_->Init(GetVertexShader(), GetFragmentShader())) {
#ifdef DEBUG
      GLFC_LOG("!! Failed to initialize program.\n");
      return false;
#endif
    }
  }

  ApplyFilterToFramebuffer(input_texture, program_, framebuffer_);
  return true;
}

}  // namespace glfc
