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

  std::shared_ptr<LveModel> circleModel =
      kc_bonus::createCircleModel(lveDevice, 64);

  // https://evgenii.com/blog/three-body-problem-simulator/
  glm::vec3 figure_8_position{0.97000436, -0.24308753, 0.f};
  glm::vec3 figure_8_velocity{-0.93240737, -0.86473146, 0.f};

  // create physics objects
  std::vector<LveGameObject> physicsObjects{};
  auto red = LveGameObject::createGameObject();
  red.transform.scale = glm::vec3{.05f};
  red.transform.translation = {.0f, .0f, 0.f};
  red.color = {1.f, 0.f, 0.f};
  red.rigidBody.velocity = figure_8_velocity;
  red.rigidBody.mass = 1.02f;
  red.model = circleModel;
  physicsObjects.push_back(std::move(red));

  auto blue = LveGameObject::createGameObject();
  blue.transform.scale = glm::vec3{.05f};
  blue.transform.translation = figure_8_position;
  blue.color = {0.f, 0.f, 1.f};
  blue.rigidBody.velocity = figure_8_velocity * -0.5f;
  blue.rigidBody.mass = 1.0f;
  blue.model = circleModel;
  physicsObjects.push_back(std::move(blue));

  auto green = LveGameObject::createGameObject();
  green.transform.scale = glm::vec3{.05f};
  green.transform.translation = -figure_8_position;
  green.color = {0.f, 1.f, 0.f};
  green.rigidBody.velocity = figure_8_velocity * -0.5f;
  green.rigidBody.mass = 1.0f;
  green.model = circleModel;
  physicsObjects.push_back(std::move(green));

  kc_bonus::GravityPhysicsSystem gravitySystem{1.0f};

  SimpleRenderSystem simpleRenderSystem{lveDevice,
                                        lveRenderer.getSwapChainRenderPass()};

  while (!lveWindow.shouldClose()) {
    glfwPollEvents();

    if (auto commandBuffer = lveRenderer.beginFrame()) {
      // update systems
      gravitySystem.update(physicsObjects, 1.f / 180, 5);

      // render system
      lveRenderer.beginSwapChainRenderPass(commandBuffer);
      simpleRenderSystem.renderGameObjects(commandBuffer, physicsObjects);
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

// temporary helper function, creates a 1x1x1 cube centered at offset
std::unique_ptr<LveModel> createCubeModel(LveDevice& device, glm::vec3 offset) {
  std::vector<LveModel::Vertex> vertices{

      // left face (white)
      {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
      {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
      {{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
      {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
      {{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},
      {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},

      // right face (yellow)
      {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
      {{.5f, .5f, .5f}, {.8f, .8f, .1f}},
      {{.5f, -.5f, .5f}, {.8f, .8f, .1f}},
      {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
      {{.5f, .5f, -.5f}, {.8f, .8f, .1f}},
      {{.5f, .5f, .5f}, {.8f, .8f, .1f}},

      // top face (orange, remember y axis points down)
      {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
      {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
      {{-.5f, -.5f, .5f}, {.9f, .6f, .1f}},
      {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
      {{.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
      {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},

      // bottom face (red)
      {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
      {{.5f, .5f, .5f}, {.8f, .1f, .1f}},
      {{-.5f, .5f, .5f}, {.8f, .1f, .1f}},
      {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
      {{.5f, .5f, -.5f}, {.8f, .1f, .1f}},
      {{.5f, .5f, .5f}, {.8f, .1f, .1f}},

      // nose face (blue)
      {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
      {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
      {{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
      {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
      {{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
      {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},

      // tail face (green)
      {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
      {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
      {{-.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
      {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
      {{.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
      {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},

  };
  for (auto& v : vertices) {
    v.position += offset;
  }
  return std::make_unique<LveModel>(device, vertices);
}

void FirstApp::loadGameObjects() {
  std::shared_ptr<LveModel> lveModel =
      createCubeModel(lveDevice, {.0f, .0f, .0f});
  auto cube = LveGameObject::createGameObject();
  cube.model = lveModel;
  cube.transform.translation = {.0f, .0f, .5f};
  cube.transform.scale = {.5f, .5f, .5f};
  gameObjects.push_back(std::move(cube));
}

}  // namespace lve
