#include "kc_custom.hpp"

#include "lve_model.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>

namespace kc_custom {

std::vector<lve::LveModel::Vertex> generate_sierpinski(
    int level, const std::vector<lve::LveModel::Vertex>& current_triangles) {
  std::vector<lve::LveModel::Vertex> return_triangles;
  if (level == 0) {
    return_triangles = current_triangles;
  } else {
    std::vector<lve::LveModel::Vertex> prev_triangles;
    prev_triangles = generate_sierpinski(level - 1, current_triangles);

    int num_triangles = static_cast<int>(prev_triangles.size()) / 3;
    for (int i = 0; i < num_triangles; i++) {
      for (int j = 0; j < 3; j++) {
        int prev_j = (j + 3 - 1) % 3;
        int next_j = (j + 1) % 3;
        const lve::LveModel::Vertex& current_vertex = prev_triangles[3 * i + j];
        const lve::LveModel::Vertex& prev_vertex =
            prev_triangles[3 * i + prev_j];
        const lve::LveModel::Vertex& next_vertex =
            prev_triangles[3 * i + next_j];

        glm::vec2 p1 = current_vertex.position;
        glm::vec2 p2 =
            glm::mix(current_vertex.position, next_vertex.position, 0.5);
        glm::vec2 p3 =
            glm::mix(current_vertex.position, prev_vertex.position, 0.5);

        glm::vec3 c1 = current_vertex.color;
        glm::vec3 c2 = glm::mix(current_vertex.color, next_vertex.color, 0.5);
        glm::vec3 c3 = glm::mix(current_vertex.color, prev_vertex.color, 0.5);

        return_triangles.push_back({p1, c1});
        return_triangles.push_back({p2, c2});
        return_triangles.push_back({p3, c3});
      }
    }
  }
  return return_triangles;
}

}  // namespace kc_custom