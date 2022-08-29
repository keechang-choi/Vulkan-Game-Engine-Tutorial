#pragma once

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

#include "lve_model.hpp"

namespace kc_custom {

std::vector<lve::LveModel::Vertex> generate_sierpinski(
    int level, const std::vector<lve::LveModel::Vertex>& current_triangles);

}  // namespace kc_custom