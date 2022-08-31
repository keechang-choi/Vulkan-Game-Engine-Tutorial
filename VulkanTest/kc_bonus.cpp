#include "kc_bonus.hpp"

#include "lve_game_object.hpp"
#include "lve_model.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <cassert>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>

namespace kc_bonus {

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

        glm::vec3 p1 = current_vertex.position;
        glm::vec3 p2 =
            glm::mix(current_vertex.position, next_vertex.position, 0.5);
        glm::vec3 p3 =
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

GravityPhysicsSystem ::GravityPhysicsSystem(float strength)
    : strengthGravity{strength} {}

void GravityPhysicsSystem::update(std::vector<lve::LveGameObject>& objs,
                                  float dt, unsigned int substeps) {
  const float stepDelta = dt / substeps;
  for (int i = 0; i < substeps; i++) {
    stepSimulation(objs, stepDelta);
  }
}

void GravityPhysicsSystem::stepSimulation(
    std::vector<lve::LveGameObject>& physicsObjs, float dt) {
  // Loops through all pairs of objects and applies attractive force between
  // them
  for (auto iterA = physicsObjs.begin(); iterA != physicsObjs.end(); ++iterA) {
    auto& objA = *iterA;
    for (auto iterB = iterA; iterB != physicsObjs.end(); ++iterB) {
      if (iterA == iterB) continue;
      auto& objB = *iterB;

      auto force = computeForce(objA, objB);
      objA.rigidBody.velocity += dt * -force / objA.rigidBody.mass;
      objB.rigidBody.velocity += dt * force / objB.rigidBody.mass;
    }
  }

  // update each objects position based on its final velocity
  for (auto& obj : physicsObjs) {
    obj.transform.translation += dt * obj.rigidBody.velocity;
  }
}

glm::vec3 GravityPhysicsSystem::computeForce(lve::LveGameObject& fromObj,
                                             lve::LveGameObject& toObj) const {
  auto offset = fromObj.transform.translation - toObj.transform.translation;
  float distanceSquared = glm::dot(offset, offset);

  // clown town - just going to return 0 if objects are too close together...
  if (glm::abs(distanceSquared) < 1e-10f) {
    return {.0f, .0f, .0f};
  }

  // inversely proportional to the square of the distance.
  float force = strengthGravity * toObj.rigidBody.mass *
                fromObj.rigidBody.mass / distanceSquared;

  return force * offset / glm::sqrt(distanceSquared);
}

std::unique_ptr<lve::LveModel> createCircleModel(lve::LveDevice& device,
                                                 unsigned int numSides) {
  std::vector<lve::LveModel::Vertex> uniqueVertices{};
  for (int i = 0; i < numSides; i++) {
    float angle = i * glm::two_pi<float>() / numSides;
    uniqueVertices.push_back({{glm::cos(angle), glm::sin(angle), .0f}});
  }
  uniqueVertices.push_back({});  // adds center vertex at 0, 0

  std::vector<lve::LveModel::Vertex> vertices{};
  for (int i = 0; i < numSides; i++) {
    vertices.push_back(uniqueVertices[i]);
    vertices.push_back(uniqueVertices[(i + 1) % numSides]);
    vertices.push_back(uniqueVertices[numSides]);
  }
  return std::make_unique<lve::LveModel>(device, vertices);
}

}  // namespace kc_bonus