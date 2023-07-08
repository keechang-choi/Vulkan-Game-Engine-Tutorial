#pragma once

#include "lve_buffer.hpp"
#include "lve_descriptors.hpp"
#include "lve_device.hpp"
#include "lve_frame_info.hpp"
#include "lve_pipeline.hpp"

// std
#include <memory>
#include <vector>

namespace tut {
struct Particle {
  glm::vec2 position;
  glm::vec2 velocity;
  glm::vec4 color;
};

struct ParticleUbo {
  float deltaTime = 1.0;
};

class ComputeParticleSystem {
 public:
  static constexpr uint32_t PARTICLE_COUNT = 64u;

  ComputeParticleSystem(lve::LveDevice &device, VkRenderPass renderPass);
  ~ComputeParticleSystem();

  ComputeParticleSystem(const ComputeParticleSystem &) = delete;
  ComputeParticleSystem &operator=(const ComputeParticleSystem &) = delete;

  // TODO: need only compute command buffer of the frame idx.
  // recording dispatch cmd
  void computeParticles(lve::FrameInfo &frameInfo);
  // render particles
  void renderParticles(lve::FrameInfo &frameInfo);

 private:
  void createGraphicsPipelineLayout();
  void createGraphicsPipeline(VkRenderPass renderPass);

  void createComputePipelineLayout();
  void createComputePipeline();

  void createUniformBuffers();
  void createShaderStorageBuffers();
  void createGraphicsDescriptorSetLayout();
  void createComputeDescriptorSetLayout();
  void createGraphicsDescriptorSets();
  void createComputeDescriptorSets();

  lve::LveDevice &lveDevice;
  // TODO: pipeline layout 및 생성
  std::unique_ptr<lve::LvePipeline> lveGraphicsPipeline;
  VkPipelineLayout graphicsPipelineLayout;
  std::unique_ptr<lve::LvePipeline> lveComputePipeline;
  VkPipelineLayout computePipelineLayout;

  // TODO: ubo ssbo 생성,
  std::vector<std::unique_ptr<lve::LveBuffer>> uniformBuffers;
  std::vector<std::unique_ptr<lve::LveBuffer>> shaderStorageBuffers;

  // TODO: descriptor set 생성, compute-graphics 분리
  std::unique_ptr<lve::LveDescriptorSetLayout> graphicsDescriptorSetLayout;
  std::vector<VkDescriptorSet> graphicsDescriptorSets;

  std::unique_ptr<lve::LveDescriptorSetLayout> computeDescriptorSetLayout;
  std::vector<VkDescriptorSet> computeDescriptorSets;
};
}  // namespace tut