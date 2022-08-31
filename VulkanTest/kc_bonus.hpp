#pragma once

#include "lve_game_object.hpp"
#include "lve_model.hpp"
#include "simple_render_system.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <vector>

namespace kc_bonus {

std::vector<lve::LveModel::Vertex> generate_sierpinski(
    int level, const std::vector<lve::LveModel::Vertex>& current_triangles);

// NOTE: RigidBody2dComponent

class GravityPhysicsSystem {
 public:
  GravityPhysicsSystem(float strength);
  const float strengthGravity;
  // dt: specific amout of time delta
  // substeps: intervals to divide time delta.
  // trade-off. stable simulation vs. computation.
  void update(std::vector<lve::LveGameObject>& objs, float dt,
              unsigned int substeps = 1);
  // fromObj attract toObj
  glm::vec3 computeForce(lve::LveGameObject& fromObj,
                         lve::LveGameObject& toObj) const;

 private:
  void stepSimulation(std::vector<lve::LveGameObject>& physicsObjs, float dt);
};

std::unique_ptr<lve::LveModel> createCircleModel(lve::LveDevice& device,
                                                 unsigned int numSides);

}  // namespace kc_bonus