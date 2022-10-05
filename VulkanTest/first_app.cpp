#include "first_app.hpp"

#include "kc_bonus.hpp"
#include "keyboard_movement_controller.hpp"
#include "lve_camera.hpp"
#include "lve_model.hpp"
#include "simple_render_system.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <chrono>
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
  LveCamera camera{};
  camera.setOrthographicProjection(-1, 1, -1, 1, -1, 1);

  while (!lveWindow.shouldClose()) {
    glfwPollEvents();

    if (auto commandBuffer = lveRenderer.beginFrame()) {
      // update systems
      gravitySystem.update(physicsObjects, 1.f / 180, 5);

      // render system
      lveRenderer.beginSwapChainRenderPass(commandBuffer);
      simpleRenderSystem.renderGameObjects(commandBuffer, physicsObjects,
                                           camera);
      lveRenderer.endSwapChainRenderPass(commandBuffer);
      lveRenderer.endFrame();
    }
  }

  vkDeviceWaitIdle(lveDevice.device());
}

void FirstApp::run() {
  SimpleRenderSystem simpleRenderSystem{lveDevice,
                                        lveRenderer.getSwapChainRenderPass()};
  LveCamera camera{};
  // camera.setViewDirection(glm::vec3{0.f}, glm::vec3{0.5f, 0.f, 1.f});
  camera.setViewTarget(glm::vec3{-1.f, -2.f, 2.f}, glm::vec3{0.f, 0.f, 2.5f});

  auto viewerObject = LveGameObject::createGameObject();
  KeyBoardMovementController cameraController{};

  auto currentTime = std::chrono::high_resolution_clock::now();
  float MAX_FRAME_TIME = 0.1f;
  while (!lveWindow.shouldClose()) {
    glfwPollEvents();

    auto newTime = std::chrono::high_resolution_clock::now();
    float frameTime =
        std::chrono::duration<float, std::chrono::seconds::period>(newTime -
                                                                   currentTime)
            .count();

    currentTime = newTime;

    frameTime = glm::min(frameTime, MAX_FRAME_TIME);

    cameraController.moveInPlaneXZ(lveWindow.getGLFWwindow(), frameTime,
                                   viewerObject);
    camera.setViewYXZ(viewerObject.transform.translation,
                      viewerObject.transform.rotation);

    float aspect = lveRenderer.getAspectRatio();
    camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 10.f);

    if (auto commandBuffer = lveRenderer.beginFrame()) {
      // NOTE: separate frame and renderpass, since we need to control
      // multiple render passes.
      lveRenderer.beginSwapChainRenderPass(commandBuffer);
      simpleRenderSystem.renderGameObjects(commandBuffer, gameObjects, camera);
      lveRenderer.endSwapChainRenderPass(commandBuffer);
      lveRenderer.endFrame();
    }
  }
  vkDeviceWaitIdle(lveDevice.device());
}

void FirstApp::loadGameObjects() {
  std::shared_ptr<LveModel> lveModel =
      LveModel::createModelFromFile(lveDevice, "models/flat_vase.obj");
  auto flatVase = LveGameObject::createGameObject();
  flatVase.model = lveModel;
  flatVase.transform.translation = {.5f, .5f, 2.5f};
  flatVase.transform.scale = {3.f, 1.5f, 3.f};
  gameObjects.push_back(std::move(flatVase));

  lveModel = LveModel::createModelFromFile(lveDevice, "models/smooth_vase.obj");
  auto smoothVase = LveGameObject::createGameObject();
  smoothVase.model = lveModel;
  smoothVase.transform.translation = {-.5f, .5f, 2.5f};
  smoothVase.transform.scale = {3.f, 1.5f, 3.f};
  gameObjects.push_back(std::move(smoothVase));
}

}  // namespace lve
