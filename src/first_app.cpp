#include "first_app.hpp"

#include "kc_bonus.hpp"
#include "keyboard_movement_controller.hpp"
#include "lve_buffer.hpp"
#include "lve_camera.hpp"
#include "lve_descriptors.hpp"
#include "lve_model.hpp"
#include "systems/point_light_system.hpp"
#include "systems/simple_render_system.hpp"
#include "tut_texture.hpp"

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

FirstApp::FirstApp() {
  globalPool =
      LveDescriptorPool::Builder(lveDevice)
          .setMaxSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT + maxObjectNum)
          .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                       LveSwapChain::MAX_FRAMES_IN_FLIGHT)
          .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, maxObjectNum)
          .build();

  loadGameObjects();
  int texture_obj_num = 0;
  for (const auto& kv : gameObjects) {
    auto& obj = kv.second;
    if (obj.model == nullptr) {
      continue;
    }
    texture_obj_num++;
  }
  std::cout << texture_obj_num << " < " << maxObjectNum << std::endl;

  assert(texture_obj_num < maxObjectNum);
}
FirstApp::~FirstApp() {}

void FirstApp::run() {
  std::vector<std::unique_ptr<LveBuffer>> uboBuffers(
      LveSwapChain::MAX_FRAMES_IN_FLIGHT);
  for (int i = 0; i < uboBuffers.size(); i++) {
    // VK_MEMORY_PROPERTY_HOST_COHERENT_BIT 를 쓰면 flush 신경 안써도 됨.
    uboBuffers[i] = std::make_unique<LveBuffer>(
        lveDevice, sizeof(GlobalUbo), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    uboBuffers[i]->map();
  }

  auto globalSetLayout = LveDescriptorSetLayout::Builder(lveDevice)
                             .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                         VK_SHADER_STAGE_ALL_GRAPHICS)
                             .build();
  auto objectSetLayout =
      LveDescriptorSetLayout::Builder(lveDevice)
          .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                      VK_SHADER_STAGE_FRAGMENT_BIT)
          .build();

  std::vector<VkDescriptorSet> globalDescriptorSets(
      LveSwapChain::MAX_FRAMES_IN_FLIGHT);
  for (int i = 0; i < globalDescriptorSets.size(); i++) {
    auto bufferInfo = uboBuffers[i]->descriptorInfo();
    LveDescriptorWriter(*globalSetLayout, *globalPool)
        .writeBuffer(0, &bufferInfo)
        .build(globalDescriptorSets[i]);
  }

  tut::TutTexture tutTexture{lveDevice};

  for (auto& kv : gameObjects) {
    auto& obj = kv.second;
    // TODO: system - entity sepration
    // to skip for objects w/o model(point lights)
    if (obj.model == nullptr) continue;
    auto& model = obj.model;
    if (model->textureDescriptorSet != VK_NULL_HANDLE) {
      continue;
    }

    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = model->getTextureImageView();
    imageInfo.sampler = tutTexture.getTextureSampler();
    LveDescriptorWriter(*objectSetLayout, *globalPool)
        .writeImage(0, &imageInfo)
        .build(model->textureDescriptorSet);
  }

  SimpleRenderSystem simpleRenderSystem{
      lveDevice,
      lveRenderer.getSwapChainRenderPass(),
      globalSetLayout->getDescriptorSetLayout(),
      objectSetLayout->getDescriptorSetLayout(),
  };
  PointLightSystem pointLightSystem{
      lveDevice,
      lveRenderer.getSwapChainRenderPass(),
      globalSetLayout->getDescriptorSetLayout(),
  };
  LveCamera camera{};
  // camera.setViewDirection(glm::vec3{0.f}, glm::vec3{0.5f, 0.f, 1.f});
  camera.setViewTarget(glm::vec3{-1.f, -2.f, 2.f}, glm::vec3{0.f, 0.f, 2.5f});

  auto viewerObject = LveGameObject::createGameObject();
  viewerObject.transform.translation.z = -10.5f;
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
    camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 100.f);

    if (auto commandBuffer = lveRenderer.beginFrame()) {
      int frameIndex = lveRenderer.getFrameIndex();

      FrameInfo frameInfo{
          frameIndex,
          frameTime,
          commandBuffer,
          camera,
          globalDescriptorSets[frameIndex],
          gameObjects,
      };

      // update
      GlobalUbo ubo{};
      ubo.projection = camera.getProjection();
      ubo.view = camera.getView();
      ubo.inverseView = camera.getInverseView();
      pointLightSystem.update(frameInfo, ubo);

      uboBuffers[frameIndex]->writeToBuffer(&ubo);
      // since not coherent.
      uboBuffers[frameIndex]->flush();

      // render
      // NOTE: separate frame and renderpass, since we need to control
      // multiple render passes.
      lveRenderer.beginSwapChainRenderPass(commandBuffer);

      // order matters
      simpleRenderSystem.renderGameObjects(frameInfo);
      pointLightSystem.render(frameInfo);

      lveRenderer.endSwapChainRenderPass(commandBuffer);
      lveRenderer.endFrame();
    }
  }
  vkDeviceWaitIdle(lveDevice.device());
}

void FirstApp::loadGameObjects() {
  std::shared_ptr<LveModel> lveModel = LveModel::createModelFromFile(
      lveDevice, "models/flat_vase.obj", "textures/statue-512.jpg");
  auto flatVase = LveGameObject::createGameObject();
  flatVase.model = lveModel;
  flatVase.transform.translation = {.5f, .5f, 0.f};
  flatVase.transform.scale = {3.f, 1.5f, 3.f};
  gameObjects.emplace(flatVase.getId(), std::move(flatVase));

  lveModel = LveModel::createModelFromFile(lveDevice, "models/smooth_vase.obj",
                                           "textures/statue-512.jpg");
  auto smoothVase = LveGameObject::createGameObject();
  smoothVase.model = lveModel;
  smoothVase.transform.translation = {-.5f, .5f, 0.f};
  smoothVase.transform.scale = {3.f, 1.5f, 3.f};
  gameObjects.emplace(smoothVase.getId(), std::move(smoothVase));

  lveModel = LveModel::createModelFromFile(lveDevice, "models/quad.obj",
                                           "textures/statue-512.jpg");
  auto floor = LveGameObject::createGameObject();
  floor.model = lveModel;
  floor.transform.translation = {0.f, .5f, 0.f};
  floor.transform.scale = {3.f, 1.f, 3.f};
  gameObjects.emplace(floor.getId(), std::move(floor));

  lveModel = LveModel::createModelFromFile(lveDevice, "models/quad.obj",
                                           "textures/statue-512.jpg");
  auto wallWithTexture = LveGameObject::createGameObject();
  wallWithTexture.model = lveModel;
  wallWithTexture.transform.rotation = {glm::half_pi<float>(), 0.f, 0.f};
  wallWithTexture.transform.translation = {6.f, .5f, 3.f};
  wallWithTexture.transform.scale = {3.f, 1.f, 3.f};
  gameObjects.emplace(wallWithTexture.getId(), std::move(wallWithTexture));

  std::vector<glm::vec3> lightColors{
      {1.f, .1f, .1f}, {.1f, .1f, 1.f}, {.1f, 1.f, .1f},
      {1.f, 1.f, .1f}, {.1f, 1.f, 1.f}, {1.f, 1.f, 1.f}  //
  };
  // NOTE: move 된 unique_ptr은 brace 안으로 넣어서 더이상 접근 못하게 명시.
  for (int i = 0; i < lightColors.size(); i++) {
    auto pointLight = LveGameObject::makePointLight(0.2f, 0.2f);
    pointLight.color = lightColors[i];
    auto rotateLight = glm::rotate(
        glm::mat4(1.f), (i * glm::two_pi<float>() / lightColors.size()),
        {0.f, -1.f, 0.f});
    pointLight.transform.translation =
        glm::vec3(rotateLight * glm::vec4(-1.f, -1.f, -1.f, 1.f));
    gameObjects.emplace(pointLight.getId(), std::move(pointLight));
  }
}

}  // namespace lve
