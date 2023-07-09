#include "first_app.hpp"

#include "kc_bonus.hpp"
#include "keyboard_movement_controller.hpp"
#include "lve_buffer.hpp"
#include "lve_camera.hpp"
#include "lve_descriptors.hpp"
#include "lve_model.hpp"
#include "systems/compute_particle_system.hpp"
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
          .setMaxSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT * 5 + maxObjectNum)
          .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                       LveSwapChain::MAX_FRAMES_IN_FLIGHT * 3)
          .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, maxObjectNum)
          .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                       LveSwapChain::MAX_FRAMES_IN_FLIGHT * 2)
          .build();

  loadGameObjects();
  int texture_obj_num = 0;
  for (const auto &kv : gameObjects) {
    auto &obj = kv.second;
    if (obj.model == nullptr) {
      continue;
    }
    texture_obj_num++;
  }
  std::cout << "object num: " << texture_obj_num << " < " << maxObjectNum
            << std::endl;

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

  // User sampler that only dependent on mipLevels
  // to avoid move or copy constructor, use unique_ptr
  std::unordered_map<int, std::unique_ptr<tut::TutTexture>> mipMipSamplers;

  for (auto &kv : gameObjects) {
    auto &obj = kv.second;
    // TODO: system - entity sepration
    // to skip for objects w/o model(point lights)
    if (obj.model == nullptr) continue;
    auto &model = obj.model;

    // to avoid duplicated resource for shared models.
    if (model->textureDescriptorSet != VK_NULL_HANDLE) {
      continue;
    }
    // skip for game objects that not havine texture images.
    if (model->getTextureImagePtr() == nullptr) {
      continue;
    }

    uint32_t mipLevels = model->getTextureImagePtr()->getMipLevels();
    if (mipMipSamplers.find(mipLevels) == mipMipSamplers.end()) {
      mipMipSamplers[mipLevels] =
          std::make_unique<tut::TutTexture>(lveDevice, mipLevels);
    }

    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = model->getTextureImageView();
    imageInfo.sampler = mipMipSamplers[mipLevels]->getTextureSampler();
    LveDescriptorWriter(*objectSetLayout, *globalPool)
        .writeImage(0, &imageInfo)
        .build(model->textureDescriptorSet);
  }

  std::cout << "Mipmap Sampler Num : " << mipMipSamplers.size() << std::endl;

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

  tut::ComputeParticleSystem computeParticleSystem{
      lveDevice,
      lveRenderer.getSwapChainRenderPass(),
      *globalPool,
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
  {
    std::shared_ptr<LveModel> lveModel = LveModel::createModelFromFile(
        lveDevice, "models/flat_vase.obj", "textures/gray-1.jpg");
    auto flatVase = LveGameObject::createGameObject();
    flatVase.model = lveModel;
    flatVase.transform.translation = {.5f, .5f, 0.f};
    flatVase.transform.scale = {3.f, 1.5f, 3.f};
    gameObjects.emplace(flatVase.getId(), std::move(flatVase));
  }

  {
    std::shared_ptr<LveModel> lveModel = LveModel::createModelFromFile(
        lveDevice, "models/smooth_vase.obj", "textures/gray-1.jpg");
    auto smoothVase = LveGameObject::createGameObject();
    smoothVase.model = lveModel;
    smoothVase.transform.translation = {-.5f, .5f, 0.f};
    smoothVase.transform.scale = {3.f, 1.5f, 3.f};
    gameObjects.emplace(smoothVase.getId(), std::move(smoothVase));
  }

  {
    std::shared_ptr<LveModel> lveModel = LveModel::createModelFromFile(
        lveDevice, "models/quad.obj", "textures/gray-1.jpg");
    auto floor = LveGameObject::createGameObject();
    floor.model = lveModel;
    floor.transform.translation = {0.f, .5f, 0.f};
    floor.transform.scale = {3.f, 1.f, 3.f};
    gameObjects.emplace(floor.getId(), std::move(floor));
  }

  {
    std::shared_ptr<LveModel> lveModel = LveModel::createModelFromFile(
        lveDevice, "models/quad.obj", "textures/statue-512.jpg");
    auto wallWithTexture = LveGameObject::createGameObject();
    wallWithTexture.model = lveModel;
    wallWithTexture.transform.rotation = {
        0.f,
        glm::half_pi<float>(),
        glm::half_pi<float>(),
    };
    wallWithTexture.transform.translation = {0.f, -2.5f, 3.f};
    wallWithTexture.transform.scale = {-3.f, 1.f, 3.f};
    gameObjects.emplace(wallWithTexture.getId(), std::move(wallWithTexture));
  }

  {
    std::shared_ptr<LveModel> lveModel = LveModel::createModelFromFile(
        lveDevice, "models/food_apple_01_4k.obj",
        "textures/food_apple_01_diff_4k_blender.jpg");
    //"textures/gray-1.jpg"
    auto apple = LveGameObject::createGameObject();
    apple.model = lveModel;
    apple.transform.translation = {1.5f, 0.5f, 0.f};
    // apple.transform.rotation = {
    //     glm::pi<float>(),
    //     0.f,
    //     0.f,
    // };
    apple.transform.scale = {15.f, 15.f, 15.f};
    gameObjects.emplace(apple.getId(), std::move(apple));
  }

  {
    std::shared_ptr<LveModel> lveModel = LveModel::createModelFromFile(
        lveDevice, "models/viking_room.obj", "textures/viking_room.png");
    auto viking_room = LveGameObject::createGameObject();
    viking_room.model = lveModel;
    viking_room.transform.translation = {-6.0f, 0.5f, 0.f};
    viking_room.transform.rotation = {
        glm::half_pi<float>(),
        glm::half_pi<float>(),
        -glm::half_pi<float>(),
    };
    viking_room.transform.scale = {3.0f, 3.0f, 3.0f};
    gameObjects.emplace(viking_room.getId(), std::move(viking_room));
  }

  // viking room w/o mipmap
  {
    std::shared_ptr<LveModel> lveModel = LveModel::createModelFromFile(
        lveDevice, "models/viking_room.obj", "textures/viking_room.png", false);
    auto viking_room = LveGameObject::createGameObject();
    viking_room.model = lveModel;
    viking_room.transform.translation = {-9.0f, 0.5f, 0.f};
    viking_room.transform.rotation = {
        glm::half_pi<float>(),
        glm::half_pi<float>(),
        -glm::half_pi<float>(),
    };
    viking_room.transform.scale = {3.0f, 3.0f, 3.0f};
    gameObjects.emplace(viking_room.getId(), std::move(viking_room));
  }

  std::vector<glm::vec3> lightColors{
      {1.f, .1f, .1f}, {.1f, .1f, 1.f}, {.1f, 1.f, .1f},
      {1.f, 1.f, .1f}, {.1f, 1.f, 1.f}, {1.f, 1.f, 1.f}  //
  };
  // NOTE: move 된 unique_ptr은 brace 안으로 넣어서 더이상 접근 못하게 명시.
  for (int i = 0; i < lightColors.size(); i++) {
    float angle = (i * glm::two_pi<float>() / lightColors.size());
    auto pointLight = LveGameObject::makePointLight(
        2.f, 0.2f, {0.f, -1.f, -1.f}, 1.0f, angle, lightColors[i]);

    gameObjects.emplace(pointLight.getId(), std::move(pointLight));
  }
  {
    // fixed point light
    auto pointLight =
        LveGameObject::makePointLight(3.f, 0.2f, {-6.0f, -1.f, -1.f}, 1.0f);
    pointLight.color = {1.0f, 0.5, 0.0f};
    gameObjects.emplace(pointLight.getId(), std::move(pointLight));
  }
  {
    // fixed point light
    auto pointLight =
        LveGameObject::makePointLight(3.f, 0.2f, {-9.0f, -1.f, -1.f}, 1.0f);
    pointLight.color = {1.0f, 0.5, 0.0f};
    gameObjects.emplace(pointLight.getId(), std::move(pointLight));
  }
}

}  // namespace lve
