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

#ifndef GLFC_OPENGL_HOOK_H_
#define GLFC_OPENGL_HOOK_H_

#if defined GLFC_ANDROID
#include <GLES2/gl2.h>
// iOS
#elif defined GLFC_IOS && defined GLFC_GLES2
#include <OpenGLES/ES2/gl.h>
#elif defined GLFC_IOS && defined GLFC_GLES3
#include <OpenGLES/ES3/gl.h>
// Mac
#elif defined GLFC_MAC && defined GLFC_GL2
#include <OpenGL/gl.h>
#endif

#endif  // GLFC_OPENGL_HOOK_H_
