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

#include "glfc/framebuffer.h"
#include "glfc/opengl_hook.h"
#include "glfc/program.h"

namespace glfc {

Filter::Filter() {
}

Filter::~Filter() {
}

void Filter::Render(const GLuint input_texture, const int width,
                    const int height) {
  Framebuffer framebuffer(width, height);
  if (!framebuffer.Init()) {
#ifdef DEBUG
    fprintf(stderr, "!! Failed to initialize framebuffer.\n");
#endif
    return;
  }

  Program program;
  if (!program.Init(GetVertexShader(), GetFragmentShader())) {
#ifdef DEBUG
    fprintf(stderr, "!! Failed to initialize program.\n");
#endif
    return;
  }

  // Applies the filter to the framebuffer object.
  framebuffer.Bind();
  SetUniforms(program.program());
  program.Render(input_texture);
  program.Finalize();
  framebuffer.Unbind();

  // Renders the current framebuffer object.
  framebuffer.Render();
}

}  // namespace glfc
