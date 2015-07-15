# Copyright 2015 Ollix
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
#
# ---
# Author: olliwang@ollix.com (Olli Wang)

{
  'includes': [
    'common.gypi',
  ],
  'targets': [
    {
      'target_name': 'libglfc',
      'type': 'static_library',
      'sources': [
         'filter.cc',
      ],
      'include_dirs': [
        '..',
      ],
      'conditions': [
        ['OS == "android"', {
          'defines': [
            'GLFC_GLES2',
            'GLFC_ANDROID',
          ],
          'ldflags': [
            '-lGLESv2',
            '-llog',
          ],
          'direct_dependent_settings': {
            'ldflags': [
              '-lGLESv2',
              '-llog',
            ],
          },
        }],
        ['OS == "ios"', {
          'defines': [
            'GLFC_GLES2',
            'GLFC_IOS',
          ],
          'link_settings': {
            'libraries': [
              '$(SDKROOT)/System/Library/Frameworks/OpenGLES.framework',
            ],
          },
        }],
        ['OS == "mac"', {
          'defines': [
            'GLFC_GL2',
            'GLFC_MAC',
          ],
          'link_settings': {
            'libraries': [
              '$(SDKROOT)/System/Library/Frameworks/OpenGL.framework',
            ],
          },
        }],
        ['OS == "win"', {
          'defines': [ 'GLFC_WINDOWS' ],
        }],
      ],  # conditions
      'direct_dependent_settings': {
        'include_dirs': [
          '..',
        ],
        'conditions': [
          ['OS == "android"', {
            'defines': [
              'GLFC_GLES2',
              'GLFC_ANDROID',
            ],
          }],
          ['OS == "ios"', {
            'defines': [
              'GLFC_GLES2',
              'GLFC_IOS',
            ],
          }],
          ['OS == "mac"', {
            'defines': [
              'GLFC_GL2',
              'GLFC_MAC',
            ],
          }],
          ['OS == "win"', {
            'defines': [
              'GLFC_WINDOWS',
            ],
          }],
        ],  # conditions
      },  # direct_dependent_settings
    },  # libglfc target
  ],  # targets
}
