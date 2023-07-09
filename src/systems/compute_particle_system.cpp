#include "compute_particle_system.hpp"

#include "lve_buffer.hpp"
#include "lve_swap_chain.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

// std
#include <random>
#include <stdexcept>
#include <vector>

namespace tut {
ComputeParticleSystem::ComputeParticleSystem(lve::LveDevice& device,
                                             VkRenderPass renderPass,
                                             lve::LveDescriptorPool& pool)
    : lveDevice{device} {
  createUniformBuffers();
  createShaderStorageBuffers();

  createGraphicsDescriptorSetLayout();
  createGraphicsDescriptorSets(pool);
  createGraphicsPipelineLayout();
  createGraphicsPipeline(renderPass);

  createComputeDescriptorSetLayout();
  createComputeDescriptorSets(pool);
  createComputePipelineLayout();
  createComputePipeline();
}
ComputeParticleSystem::~ComputeParticleSystem() {
  vkDestroyPipelineLayout(lveDevice.device(), graphicsPipelineLayout, nullptr);
  vkDestroyPipelineLayout(lveDevice.device(), computePipelineLayout, nullptr);
}

void ComputeParticleSystem::createUniformBuffers() {
  uniformBuffers.resize(lve::LveSwapChain::MAX_FRAMES_IN_FLIGHT);
  for (int i = 0; i < uniformBuffers.size(); i++) {
    // NOTE: need to flush since non-coherent
    uniformBuffers[i] = std::make_unique<lve::LveBuffer>(
        lveDevice, sizeof(ParticleUbo), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    uniformBuffers[i]->map();
  }
}

void ComputeParticleSystem::createShaderStorageBuffers() {
  // initial particle data
  std::default_random_engine randomEngine;
  randomEngine.seed(1111);
  std::uniform_real_distribution<float> randomDist(0.f, 1.f);

  std::vector<Particle> particles(PARTICLE_COUNT);
  for (auto& particle : particles) {
    float r = 0.25f * randomDist(randomEngine);
    float theta = glm::two_pi<float>() * randomDist(randomEngine);
    float x = r * cos(theta);
    float y = r * sin(theta);
    particle.position = glm::vec2(x, y);
    particle.velocity = glm::normalize(glm::vec2(x, y)) * 0.00025f;
    particle.color =
        glm::vec4(randomDist(randomEngine), randomDist(randomEngine),
                  randomDist(randomEngine), 1.f);
  }

  // transfer using staging buffer
  VkDeviceSize bufferSize = sizeof(Particle) * PARTICLE_COUNT;
  uint32_t particleSize = sizeof(Particle);
  lve::LveBuffer stagingBuffer{
      lveDevice,
      particleSize,
      PARTICLE_COUNT,
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
  };

  stagingBuffer.map();
  stagingBuffer.writeToBuffer((void*)particles.data());

  // copy buffer
  shaderStorageBuffers.resize(lve::LveSwapChain::MAX_FRAMES_IN_FLIGHT);
  for (int i = 0; i < shaderStorageBuffers.size(); i++) {
    shaderStorageBuffers[i] = std::make_unique<lve::LveBuffer>(
        lveDevice, particleSize, PARTICLE_COUNT,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
            VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    lveDevice.copyBuffer(stagingBuffer.getBuffer(),
                         shaderStorageBuffers[i]->getBuffer(), bufferSize);
  }
}

void ComputeParticleSystem::createGraphicsDescriptorSetLayout() {
  graphicsDescriptorSetLayout =
      lve::LveDescriptorSetLayout::Builder(lveDevice)
          .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                      VK_SHADER_STAGE_ALL_GRAPHICS)
          .build();
}

void ComputeParticleSystem::createComputeDescriptorSetLayout() {
  computeDescriptorSetLayout =
      lve::LveDescriptorSetLayout::Builder(lveDevice)
          .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                      VK_SHADER_STAGE_COMPUTE_BIT)
          .addBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                      VK_SHADER_STAGE_COMPUTE_BIT)
          .addBinding(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                      VK_SHADER_STAGE_COMPUTE_BIT)
          .build();
}

void ComputeParticleSystem::createGraphicsDescriptorSets(
    lve::LveDescriptorPool& pool) {
  graphicsDescriptorSets.resize(lve::LveSwapChain::MAX_FRAMES_IN_FLIGHT);
  for (int i = 0; i < graphicsDescriptorSets.size(); i++) {
    auto bufferInfo = uniformBuffers[i]->descriptorInfo();
    lve::LveDescriptorWriter(*graphicsDescriptorSetLayout, pool)
        .writeBuffer(0, &bufferInfo)
        .build(graphicsDescriptorSets[i]);
  }
}

void ComputeParticleSystem::createComputeDescriptorSets(
    lve::LveDescriptorPool& pool) {
  // allocate descriptor sets를 한번에 여러개 가능한데 일단 기본 구조대로
  // 하나씩.
  // https://github.com/Overv/VulkanTutorial/blob/main/code/31_compute_shader.cpp#L862
  computeDescriptorSets.resize(lve::LveSwapChain::MAX_FRAMES_IN_FLIGHT);
  for (int i = 0; i < computeDescriptorSets.size(); i++) {
    auto uniformBufferInfo = uniformBuffers[i]->descriptorInfo();
    int prevFrameIdx = (i - 1 + lve::LveSwapChain::MAX_FRAMES_IN_FLIGHT) %
                       lve::LveSwapChain::MAX_FRAMES_IN_FLIGHT;
    auto storageBufferInfoLastFrame =
        shaderStorageBuffers[prevFrameIdx]->descriptorInfo();
    auto storageBufferInfoCurrentFrame =
        shaderStorageBuffers[i]->descriptorInfo();
    lve::LveDescriptorWriter(*computeDescriptorSetLayout, pool)
        .writeBuffer(0, &uniformBufferInfo)
        .writeBuffer(1, &storageBufferInfoLastFrame)
        .writeBuffer(2, &storageBufferInfoCurrentFrame)
        .build(computeDescriptorSets[i]);
  }
}

void ComputeParticleSystem::createGraphicsPipelineLayout() {
  // TODO: create multiple pipelines at once to optimize.
  std::vector<VkDescriptorSetLayout> descriptorSetLayouts{
      graphicsDescriptorSetLayout->getDescriptorSetLayout()};

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount =
      static_cast<size_t>(descriptorSetLayouts.size());
  pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
  if (vkCreatePipelineLayout(lveDevice.device(), &pipelineLayoutInfo, nullptr,
                             &graphicsPipelineLayout) != VK_SUCCESS) {
    throw std::runtime_error("failed to create graphics pipeline layout!");
  }
}

void ComputeParticleSystem::createComputePipelineLayout() {
  // TODO: create multiple pipelines at once to optimize.
  std::vector<VkDescriptorSetLayout> descriptorSetLayouts{
      computeDescriptorSetLayout->getDescriptorSetLayout()};

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount =
      static_cast<size_t>(descriptorSetLayouts.size());
  pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
  if (vkCreatePipelineLayout(lveDevice.device(), &pipelineLayoutInfo, nullptr,
                             &computePipelineLayout) != VK_SUCCESS) {
    throw std::runtime_error("failed to create compute pipeline layout!");
  }
}

void ComputeParticleSystem::createGraphicsPipeline(VkRenderPass renderPass) {
  assert(graphicsPipelineLayout != nullptr &&
         "Cannot create pipeline before pipeline layout.");

  lve::PipelineConfigInfo pipelineConfig{};
  lve::LvePipeline::defaultPipelineConfigInfo(pipelineConfig);
  pipelineConfig.renderPass = renderPass;
  pipelineConfig.pipelineLayout = graphicsPipelineLayout;
  pipelineConfig.multisampleInfo.rasterizationSamples =
      lveDevice.getSampleCount();
  pipelineConfig.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;

  lveGraphicsPipeline = std::make_unique<lve::LvePipeline>(lveDevice);
  lveGraphicsPipeline->createGraphicsPipeline(
      "./shaders/compute_particle.vert.spv",
      "./shaders/compute_particle.frag.spv", pipelineConfig);
}

void ComputeParticleSystem::createComputePipeline() {
  assert(computePipelineLayout != nullptr &&
         "Cannot create pipeline before pipeline layout.");

  lve::PipelineConfigInfo pipelineConfig{};
  pipelineConfig.pipelineLayout = computePipelineLayout;

  lveComputePipeline = std::make_unique<lve::LvePipeline>(lveDevice);
  lveComputePipeline->createComputePipeline(
      "./shaders/compute_particle.comp.spv", pipelineConfig);
}
// TODO: compute command buffer and dispatch part

}  // namespace tut