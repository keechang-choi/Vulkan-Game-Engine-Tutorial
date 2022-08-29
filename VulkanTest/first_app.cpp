#include "first_app.hpp"

#include "kc_bonus.hpp"
#include "simple_render_system.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <iostream>
#include <memory>
#include <stdexcept>

namespace lve {

FirstApp::FirstApp() { loadGameObjects(); }
FirstApp::~FirstApp() {}

void FirstApp::runGravity() {
  // create some models
  std::shared_ptr<LveModel> squareModel = kc_bonus::createSquareModel(
      lveDevice, {.5f, .0f});  // offset model by .5 so rotation occurs at edge
                               // rather than center of square
  std::shared_ptr<LveModel> circleModel =
      kc_bonus::createCircleModel(lveDevice, 64);

  // https://evgenii.com/blog/three-body-problem-simulator/
  glm::vec2 figure_8_position{0.97000436, -0.24308753};
  glm::vec2 figure_8_velocity{-0.93240737, -0.86473146};

  // create physics objects
  std::vector<LveGameObject> physicsObjects{};
  auto red = LveGameObject::createGameObject();
  red.transform2d.scale = glm::vec2{.05f};
  red.transform2d.translation = {.0f, .0f};
  red.color = {1.f, 0.f, 0.f};
  red.rigidBody2d.velocity = figure_8_velocity;
  red.rigidBody2d.mass = 1.02f;
  red.model = circleModel;
  physicsObjects.push_back(std::move(red));

  auto blue = LveGameObject::createGameObject();
  blue.transform2d.scale = glm::vec2{.05f};
  blue.transform2d.translation = figure_8_position;
  blue.color = {0.f, 0.f, 1.f};
  blue.rigidBody2d.velocity = figure_8_velocity * -0.5f;
  blue.rigidBody2d.mass = 1.0f;
  blue.model = circleModel;
  physicsObjects.push_back(std::move(blue));

  auto green = LveGameObject::createGameObject();
  green.transform2d.scale = glm::vec2{.05f};
  green.transform2d.translation = -figure_8_position;
  green.color = {0.f, 1.f, 0.f};
  green.rigidBody2d.velocity = figure_8_velocity * -0.5f;
  green.rigidBody2d.mass = 1.0f;
  green.model = circleModel;
  physicsObjects.push_back(std::move(green));

  // create vector field
  std::vector<LveGameObject> vectorField{};
  int gridCount = 40;
  for (int i = 0; i < gridCount; i++) {
    for (int j = 0; j < gridCount; j++) {
      auto vf = LveGameObject::createGameObject();
      vf.transform2d.scale = glm::vec2(0.005f);
      vf.transform2d.translation = {-1.0f + (i + 0.5f) * 2.0f / gridCount,
                                    -1.0f + (j + 0.5f) * 2.0f / gridCount};
      vf.color = glm::vec3(1.0f);
      vf.model = squareModel;
      vectorField.push_back(std::move(vf));
    }
  }

  kc_bonus::GravityPhysicsSystem gravitySystem{1.0f};
  kc_bonus::Vec2FieldSystem vecFieldSystem{};

  SimpleRenderSystem simpleRenderSystem{lveDevice,
                                        lveRenderer.getSwapChainRenderPass()};

  while (!lveWindow.shouldClose()) {
    glfwPollEvents();

    if (auto commandBuffer = lveRenderer.beginFrame()) {
      // update systems
      gravitySystem.update(physicsObjects, 1.f / 180, 5);
      vecFieldSystem.update(gravitySystem, physicsObjects, vectorField);

      // render system
      lveRenderer.beginSwapChainRenderPass(commandBuffer);
      simpleRenderSystem.renderGameObjects(commandBuffer, physicsObjects);
      simpleRenderSystem.renderGameObjects(commandBuffer, vectorField);
      lveRenderer.endSwapChainRenderPass(commandBuffer);
      lveRenderer.endFrame();
    }
  }

  vkDeviceWaitIdle(lveDevice.device());
}

void FirstApp::run() {
  SimpleRenderSystem simpleRenderSystem{lveDevice,
                                        lveRenderer.getSwapChainRenderPass()};
  while (!lveWindow.shouldClose()) {
    glfwPollEvents();
    if (auto commandBuffer = lveRenderer.beginFrame()) {
      // NOTE: separate frame and renderpass, since we need to control
      // multiple render passes.
      lveRenderer.beginSwapChainRenderPass(commandBuffer);
      simpleRenderSystem.renderGameObjects(commandBuffer, gameObjects);
      lveRenderer.endSwapChainRenderPass(commandBuffer);
      lveRenderer.endFrame();
    }
  }
  vkDeviceWaitIdle(lveDevice.device());
}

void FirstApp::loadGameObjects() {
  std::vector<LveModel::Vertex> vertices{{{0.0f, -0.6f}, {1.0f, 0.0f, 0.0f}},
                                         {{0.5f, 0.3f}, {0.0f, 1.0f, 0.0f}},
                                         {{-0.5f, 0.3f}, {0.0f, 0.0f, 1.0f}}};
  std::vector<LveModel::Vertex> sierpinski_vertices =
      kc_bonus::generate_sierpinski(2, vertices);
  // NOTE(kc): a model can be shared by multiple game objects.
  auto lveModel = std::make_shared<LveModel>(lveDevice, sierpinski_vertices);

  // https://www.color-hex.com/color-palette/5361
  std::vector<glm::vec3> colors{{1.f, .7f, .73f}, {1.f, .87f, .73f},
                                {1.f, 1.f, .73f}, {.73f, 1.f, .8f},
                                {.73, .88f, 1.f}, {1.f, 1.f, 1.f}};
  for (auto& color : colors) {
    color = glm::pow(color, glm::vec3{4.0f});
    std::cout << color[0] << " " << color[1] << " " << color[2] << std::endl;
  }

  const int n = 60;
  for (int i = 0; i < n; i++) {
    auto triangle = LveGameObject::createGameObject();
    triangle.model = lveModel;
    triangle.color = colors[i % colors.size()];
    triangle.transform2d.scale = glm::vec2(.5f) + i / static_cast<float>(n);
    triangle.transform2d.rotation = i * .025f * glm::pi<float>();

    gameObjects.push_back(std::move(triangle));
  }
}

}  // namespace lve
