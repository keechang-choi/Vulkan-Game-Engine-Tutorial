#include "first_app.hpp"

#include "kc_bonus.hpp"
#include "keyboard_movement_controller.hpp"
#include "lve_buffer.hpp"
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

struct GlobalUbo {
  glm::mat4 projectionView{1.f};
  glm::vec3 lightDirection = glm::normalize(glm::vec3{1.f, -3.f, -1.f});
};

FirstApp::FirstApp() { loadGameObjects(); }
FirstApp::~FirstApp() {}

void FirstApp::run() {
  LveBuffer globalUboBuffer{
      lveDevice,
      sizeof(GlobalUbo),
      LveSwapChain::MAX_FRAMES_IN_FLIGHT,
      VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
      lveDevice.properties.limits.minUniformBufferOffsetAlignment,
  };

  globalUboBuffer.map();

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
      int frameIndex = lveRenderer.getFrameIndex();

      FrameInfo frameInfo{
          frameIndex,
          frameTime,
          commandBuffer,
          camera,
      };

      // update
      GlobalUbo ubo{};
      ubo.projectionView = camera.getProjection() * camera.getView();
      globalUboBuffer.writeToIndex(&ubo, frameIndex);
      // since not coherent.
      globalUboBuffer.flushIndex(frameIndex);

      // render
      // NOTE: separate frame and renderpass, since we need to control
      // multiple render passes.
      lveRenderer.beginSwapChainRenderPass(commandBuffer);
      simpleRenderSystem.renderGameObjects(frameInfo, gameObjects);
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
