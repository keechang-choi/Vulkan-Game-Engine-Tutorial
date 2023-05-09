#pragma once

#include "lve_model.hpp"

// libs
#include <glm/gtc/matrix_transform.hpp>

// std
#include <memory>
#include <unordered_map>

namespace lve {

struct RigidBodyComponent {
  glm::vec3 velocity;
  float mass{1.0f};
};

struct TransformComponent {
  glm::vec3 translation{};
  glm::vec3 scale{1.f, 1.f, 1.f};
  glm::vec3 rotation{};

  // translation mat * Ry * Rx * Rz * scale mat
  // tait-bryan angles with YXZ
  // https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
  glm::mat4 mat4();
  glm::mat3 normalMatrix();
};

struct PointLightComponent {
  float lightIntensity = 1.0f;
  glm::vec3 rotationCenter{};
  float rotationRadius = 0.0f;
  float angle = 0.0f;
};

class LveGameObject {
 public:
  using id_t = unsigned int;
  using Map = std::unordered_map<id_t, LveGameObject>;

  static LveGameObject createGameObject() {
    static id_t currentId = 0;
    return LveGameObject{currentId++};
  }

  static LveGameObject makePointLight(float intensity = 10.0f,
                                      float radius = 0.1f,
                                      glm::vec3 rotationCenter = glm::vec3(0.f),
                                      float rotationRadius = 0.0f,
                                      float angle = 0.0f,
                                      glm::vec3 color = glm::vec3(1.f));

  LveGameObject(const LveGameObject &) = delete;
  LveGameObject &operator=(const LveGameObject &) = delete;
  LveGameObject(LveGameObject &&) = default;
  LveGameObject &operator=(LveGameObject &&) = default;

  id_t getId() const { return id; }

  glm::vec3 color{};
  TransformComponent transform{};
  RigidBodyComponent rigidBody{};

  // Optional pointer components
  std::shared_ptr<LveModel> model{};
  std::unique_ptr<PointLightComponent> pointLight = nullptr;

 private:
  LveGameObject(id_t objId) : id{objId} {}
  id_t id;
};
}  // namespace lve