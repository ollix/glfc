#include <cassert>
#include <algorithm>
#include <iostream>
#include <string>

#include "glfc/gaussian_blur_filter.h"

namespace glfc {
class GaussianBlurShaderTest {
 public:
  static std::string Fragment(const GaussianBlurFilter& filter) {
    return filter.GetFragmentShader();
  }
  static std::string Vertex(const GaussianBlurFilter& filter) {
    return filter.GetVertexShader();
  }
};
}

int main(int argc, char **argv) {
  const bool dump = argc == 2 && std::string(argv[1]) == "--dump";
  glfc::GaussianBlurFilter filter;
  for (int radius : {0, 1, 2, 7, 14, 15, 30, 64}) {
    filter.set_blur_radius(radius);
    const auto fragment = glfc::GaussianBlurShaderTest::Fragment(filter);
    const auto vertex = glfc::GaussianBlurShaderTest::Vertex(filter);
    if (dump) std::cout << radius << '\n' << fragment << '\n' << vertex << '\n';
    if (radius == 0) {
      assert(fragment.empty() && vertex.empty());
      continue;
    }
    const int coordinates = 1 + 2 * std::min((radius + 1) / 2, 7);
    const auto declaration = "blurCoordinates[" + std::to_string(coordinates) + "];";
    assert(fragment.find(declaration) != std::string::npos);
    assert(vertex.find(declaration) != std::string::npos);
    assert(fragment.find("gl_FragColor = sum;") != std::string::npos);
    assert(vertex.find("gl_Position = position;") != std::string::npos);
    assert(fragment.find('\0') == std::string::npos);
    assert(vertex.find('\0') == std::string::npos);
    assert(fragment == glfc::GaussianBlurShaderTest::Fragment(filter));
    assert(vertex == glfc::GaussianBlurShaderTest::Vertex(filter));
  }
}
