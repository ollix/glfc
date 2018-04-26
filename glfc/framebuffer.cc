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

#include "glfc/framebuffer.h"

#include <cstdio>

#include "glfc/base.h"
#include "glfc/opengl_hook.h"
#include "glfc/program.h"

namespace {

const char* kVertexShader = R"(
attribute vec4 position;
attribute vec2 inputTextureCoordinate;

varying vec2 textureCoordinate;

void main() {
  textureCoordinate = inputTextureCoordinate;
  gl_Position = position;
})";

const char* kFragmentShader = R"(
uniform sampler2D inputImageTexture;
varying mediump vec2 textureCoordinate;

void main() {
  gl_FragColor = texture2D(inputImageTexture, textureCoordinate);
})";

}  // namespace

namespace glfc {

Framebuffer::Framebuffer(const int width, const int height)
    : framebuffer_(0), height_(height), is_initialized_(false),
      program_(nullptr), renderbuffer_(0), texture_(0), width_(width) {
}

Framebuffer::~Framebuffer() {
  if (is_initialized_)
    Finalize();

  if (program_ != nullptr)
    delete program_;
}

bool Framebuffer::Init() {
  if (is_initialized_) {
    Finalize();
  }

  if (program_ == nullptr) {
    program_ = new Program;
    if (!program_->Init(kVertexShader, kFragmentShader)) {
      delete program_;
      program_ = nullptr;
  #ifdef DEBUG
      GLFC_LOG("!! Failed to initialize program for framebuffer.\n");
  #endif
      return false;
    }
  }

  // Remembers the current framebuffer and renderbuffer that will be restored
  // in the end of this method.
  GLint original_framebuffer;
  GLint original_renderbuffer;
  glGetIntegerv(GL_FRAMEBUFFER_BINDING, &original_framebuffer);
  glGetIntegerv(GL_RENDERBUFFER_BINDING, &original_renderbuffer);

  // Creates the texture.
  glGenTextures(1, &texture_);
  glBindTexture(GL_TEXTURE_2D, texture_);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
#ifndef GLFC_GLES2
	glPixelStorei(GL_UNPACK_ROW_LENGTH, width_);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
	glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
#endif
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_, height_, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
#ifndef GLFC_GLES2
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
	glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
#endif
  glBindTexture(GL_TEXTURE_2D, 0);

  // Creates the framebuffer object.
  glGenFramebuffers(1, &framebuffer_);
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);

  // Creates the renderbuffer object.
  glGenRenderbuffers(1, &renderbuffer_);
  glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer_);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, width_, height_);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         texture_, 0);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
                            GL_RENDERBUFFER, renderbuffer_);

  // Returns the result and restores the original framebuffer and renderbuffer.
  const bool kResult = glCheckFramebufferStatus(GL_FRAMEBUFFER) == \
                       GL_FRAMEBUFFER_COMPLETE;
  if (kResult) {
    is_initialized_ = true;
  } else {
    Finalize();
  }
  glBindFramebuffer(GL_FRAMEBUFFER, original_framebuffer);
  glBindRenderbuffer(GL_RENDERBUFFER, original_renderbuffer);
  return kResult;
}

void Framebuffer::Bind() {
  glGetIntegerv(GL_FRAMEBUFFER_BINDING, &original_framebuffer_);
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);
  blend_is_enabled_ = glIsEnabled(GL_BLEND);
  if (blend_is_enabled_) {
    glGetIntegerv(GL_BLEND_SRC_RGB, &blend_src_rgb_);
    glGetIntegerv(GL_BLEND_SRC_ALPHA, &blend_src_alpha_);
    glGetIntegerv(GL_BLEND_DST_RGB, &blend_dst_rgb_);
    glGetIntegerv(GL_BLEND_DST_ALPHA, &blend_dst_alpha_);
  } else {
    glEnable(GL_BLEND);
  }
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
}

void Framebuffer::Clear() {
  glViewport(0, 0, width_, height_);
  glClearColor(0, 0, 0, 0);
  glClear(GL_COLOR_BUFFER_BIT);
}

void Framebuffer::Finalize() {
  if (framebuffer_ > 0) {
    glDeleteFramebuffers(1, &framebuffer_);
    framebuffer_ = 0;
  }
  if (renderbuffer_ > 0) {
    glDeleteRenderbuffers(1, &renderbuffer_);
    renderbuffer_ = 0;
  }
  if (texture_ > 0) {
    glDeleteTextures(1, &texture_);
    texture_ = 0;
  }
  is_initialized_ = false;
}

void Framebuffer::Render() const {
  program_->Use();
  glBlendFunc(GL_ONE, GL_ZERO);
  program_->Render(texture_);
}

void Framebuffer::Unbind() const {
  glBindFramebuffer(GL_FRAMEBUFFER, original_framebuffer_);
  if (blend_is_enabled_) {
    glBlendFuncSeparate(blend_src_rgb_, blend_dst_rgb_, blend_src_alpha_,
                        blend_dst_alpha_);
  } else {
    glDisable(GL_BLEND);
  }
}

}  // namespace glfc
