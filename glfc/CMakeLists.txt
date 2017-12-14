# Copyright 2017 Ollix
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required(VERSION 3.4.1)

if(APPLE)
    option(IOS "Build for iOS" NO)

    if(CMAKE_OSX_SYSROOT STREQUAL "iphoneos" OR
       CMAKE_OSX_SYSROOT MATCHES "iPhoneOS")
        set(IOS YES)
    elseif(IOS)
        set(CMAKE_OSX_SYSROOT "iphoneos")
    else()
        set(MAC YES)
    endif()
endif()

add_library(glfc
    STATIC
    "filter.cc"
    "framebuffer.cc"
    "gaussian_blur_filter.cc"
    "program.cc")

set_target_properties(glfc
    PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO)

target_include_directories(glfc PUBLIC "..")

if(ANDROID)
    target_compile_definitions(glfc PUBLIC "GLFC_ANDROID" "GLFC_GLES2")
    target_link_libraries(glfc PRIVATE "GLESv2" "log")
elseif(IOS)
    target_compile_definitions(glfc PUBLIC "GLFC_GLES2" "GLFC_IOS")
    target_link_libraries(glfc PRIVATE "-framework OpenGLES")
elseif(MAC)
    target_compile_definitions(glfc PUBLIC "GLFC_GL2" "GLFC_MAC")
    target_link_libraries(glfc PRIVATE "-framework OpenGL")
endif()
