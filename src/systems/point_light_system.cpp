#include "point_light_system.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <iostream>
#include <map>
#include <memory>
#include <stdexcept>

namespace lve {

struct PointLightPushConstants {
  glm::vec4 position{};
  glm::vec4 color{};
  float radius;
};

PointLightSystem::PointLightSystem(LveDevice& device, VkRenderPass renderPass,
                                   VkDescriptorSetLayout globalSetLayout)
    : lveDevice{device} {
  createPipelineLayout(globalSetLayout);
  createPipeline(renderPass);
}
PointLightSystem::~PointLightSystem() {
  vkDestroyPipelineLayout(lveDevice.device(), pipelineLayout, nullptr);
}

void PointLightSystem::createPipelineLayout(
    VkDescriptorSetLayout globalSetLayout) {
  VkPushConstantRange pushConstantRange{};
  pushConstantRange.stageFlags =
      VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
  pushConstantRange.offset = 0;
  pushConstantRange.size = sizeof(PointLightPushConstants);

  // only one for now.
  std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout};

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount =
      static_cast<uint32_t>(descriptorSetLayouts.size());
  pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
  pipelineLayoutInfo.pushConstantRangeCount = 1;
  pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
  if (vkCreatePipelineLayout(lveDevice.device(), &pipelineLayoutInfo, nullptr,
                             &pipelineLayout) != VK_SUCCESS) {
    throw std::runtime_error("failed to create pipeline layout!");
  }
}
void PointLightSystem::createPipeline(VkRenderPass renderPass) {
  assert(pipelineLayout != nullptr &&
         "Cannot create pipeline before pipeline layout.");

  PipelineConfigInfo pipelineConfig{};
  LvePipeline::defaultPipelineConfigInfo(pipelineConfig);
  LvePipeline::enableAlphaBlending(pipelineConfig);

  pipelineConfig.attributeDescriptions.clear();
  pipelineConfig.bindingDescriptions.clear();
  pipelineConfig.renderPass = renderPass;
  pipelineConfig.pipelineLayout = pipelineLayout;
  pipelineConfig.multisampleInfo.rasterizationSamples =
      lveDevice.getSampleCount();

  lvePipeline = std::make_unique<LvePipeline>(lveDevice);
  lvePipeline->createGraphicsPipeline("./shaders/point_light.vert.spv",
                                      "./shaders/point_light.frag.spv",
                                      pipelineConfig);
}

void PointLightSystem::render(FrameInfo& frameInfo) {
  // update
  // int i = 0;
  // for (auto& obj : gameObjects) {
  //  i++;
  //  float v = 0.0001f;
  //  obj.transform.rotation.y =
  //      glm::mod<float>(obj.transform.rotation.y + v * i,
  //      glm::two_pi<float>());
  //  obj.transform.rotation.x = glm::mod<float>(
  //      obj.transform.rotation.x + v * 0.5 * i, glm::two_pi<float>());
  //}

  // sort lights
  std::map<float, LveGameObject::id_t> sorted;
  for (auto& kv : frameInfo.gameObjects) {
    auto& obj = kv.second;
    if (obj.pointLight == nullptr) continue;

    // calculate distance
    auto offset = frameInfo.camera.getPosition() - obj.transform.translation;
    float disSquared = glm::dot(offset, offset);
    sorted[disSquared] = obj.getId();
  }

  // render
  lvePipeline->bind(frameInfo.commandBuffer);

  vkCmdBindDescriptorSets(frameInfo.commandBuffer,
                          VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1,
                          &frameInfo.globalDescriptorSet, 0, nullptr);
  // iterate through sroted lights in reverse order
  for (auto it = sorted.rbegin(); it != sorted.rend(); ++it) {
    // use gmae obj id to find light object
    auto& obj = frameInfo.gameObjects.at(it->second);

    PointLightPushConstants push{};
    push.position = glm::vec4(obj.transform.translation, 1.f);
    push.color = glm::vec4(obj.color, obj.pointLight->lightIntensity);
    push.radius = obj.transform.scale.x;

    vkCmdPushConstants(
        frameInfo.commandBuffer, pipelineLayout,
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
        sizeof(PointLightPushConstants), &push);

    vkCmdDraw(frameInfo.commandBuffer, 6, 1, 0, 0);
  }
}
void PointLightSystem::update(FrameInfo& frameInfo, GlobalUbo& ubo) {
  int lightIndex = 0;
  for (auto& kv : frameInfo.gameObjects) {
    auto& obj = kv.second;
    if (obj.pointLight == nullptr) continue;

    assert(lightIndex < MAX_LIGHTS && "Point lights exceed maximum specified.");

    // update angle
    obj.pointLight->angle += frameInfo.frameTime;
    if (obj.pointLight->angle > glm::two_pi<float>()) {
      obj.pointLight->angle -= glm::two_pi<float>();
    }
    auto rotateLight =
        glm::rotate(glm::mat4(1.f), obj.pointLight->angle, {0.f, -1.f, 0.f});

    glm::vec4 radiusPos =
        glm::vec4(obj.pointLight->rotationRadius, 0.f, 0.f, 1.f);
    // update light position
    obj.transform.translation =
        obj.pointLight->rotationCenter + glm::vec3(rotateLight * radiusPos);

    // copy light to ubo
    ubo.pointLights[lightIndex].position =
        glm::vec4(obj.transform.translation, 1.f);
    ubo.pointLights[lightIndex].color =
        glm::vec4(obj.color, obj.pointLight->lightIntensity);

    lightIndex++;
  }
  ubo.numLights = lightIndex;
}

}  // namespace lve
