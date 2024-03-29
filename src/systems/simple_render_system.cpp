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

namespace {
// temp code
struct SimplePushConstantData {
  glm::mat4 modelMatrix{1.f};
  glm::mat4 normalMatrix{1.f};
};
}  // namespace

namespace lve {

SimpleRenderSystem::SimpleRenderSystem(LveDevice& device,
                                       VkRenderPass renderPass,
                                       VkDescriptorSetLayout globalSetLayout,
                                       VkDescriptorSetLayout objectSetLayout)
    : lveDevice{device} {
  createPipelineLayout(globalSetLayout, objectSetLayout);
  createPipeline(renderPass);
}
SimpleRenderSystem::~SimpleRenderSystem() {
  vkDestroyPipelineLayout(lveDevice.device(), pipelineLayout, nullptr);
}

void SimpleRenderSystem::createPipelineLayout(
    VkDescriptorSetLayout globalSetLayout,
    VkDescriptorSetLayout objectSetLayout) {
  VkPushConstantRange pushConstantRange{};
  pushConstantRange.stageFlags =
      VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
  pushConstantRange.offset = 0;
  pushConstantRange.size = sizeof(SimplePushConstantData);

  // only one for now.
  std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout,
                                                          objectSetLayout};

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
void SimpleRenderSystem::createPipeline(VkRenderPass renderPass) {
  assert(pipelineLayout != nullptr &&
         "Cannot create pipeline before pipeline layout.");

  PipelineConfigInfo pipelineConfig{};
  LvePipeline::defaultPipelineConfigInfo(pipelineConfig);
  pipelineConfig.renderPass = renderPass;
  pipelineConfig.pipelineLayout = pipelineLayout;
  pipelineConfig.multisampleInfo.rasterizationSamples =
      lveDevice.getSampleCount();
  lvePipeline = std::make_unique<LvePipeline>(lveDevice);
  lvePipeline->createGraphicsPipeline("./shaders/simple_shader.vert.spv",
                                      "./shaders/simple_shader.frag.spv",
                                      pipelineConfig);
}

void SimpleRenderSystem::renderGameObjects(FrameInfo& frameInfo) {
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

  // render
  lvePipeline->bind(frameInfo.commandBuffer);

  vkCmdBindDescriptorSets(frameInfo.commandBuffer,
                          VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1,
                          &frameInfo.globalDescriptorSet, 0, nullptr);

  for (auto& kv : frameInfo.gameObjects) {
    auto& obj = kv.second;
    if (obj.model == nullptr) continue;
    SimplePushConstantData push{};
    push.modelMatrix = obj.transform.mat4();
    push.normalMatrix = obj.transform.normalMatrix();

    vkCmdPushConstants(
        frameInfo.commandBuffer, pipelineLayout,
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
        sizeof(SimplePushConstantData), &push);

    vkCmdBindDescriptorSets(frameInfo.commandBuffer,
                            VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1,
                            1, &obj.model->textureDescriptorSet, 0, nullptr);
    obj.model->bind(frameInfo.commandBuffer);
    obj.model->draw(frameInfo.commandBuffer);
  }
}

}  // namespace lve
