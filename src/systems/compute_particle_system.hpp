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
  static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
  static std::vector<VkVertexInputAttributeDescription>
  getAttributeDescriptions();
};

struct ParticleUbo {
  float deltaTime = 1.0;
};

class ComputeParticleSystem {
 public:
  static constexpr uint32_t PARTICLE_COUNT = 256u;

  ComputeParticleSystem(lve::LveDevice &device, VkRenderPass renderPass,
                        lve::LveDescriptorPool &pool);
  ~ComputeParticleSystem();

  ComputeParticleSystem(const ComputeParticleSystem &) = delete;
  ComputeParticleSystem &operator=(const ComputeParticleSystem &) = delete;

  // TODO: need only compute command buffer of the frame idx.
  // recording dispatch cmd
  void computeParticles(lve::FrameInfo &frameInfo);
  // render particles
  void renderParticles(lve::FrameInfo &frameInfo);
  void updateUbo(lve::FrameInfo &frameInfo);

 private:
  void createGraphicsPipelineLayout();
  void createGraphicsPipeline(VkRenderPass renderPass);

  void createComputePipelineLayout();
  void createComputePipeline();

  void createUniformBuffers();
  void createShaderStorageBuffers();
  void createGraphicsDescriptorSetLayout();
  void createComputeDescriptorSetLayout();
  void createGraphicsDescriptorSets(lve::LveDescriptorPool &pool);
  void createComputeDescriptorSets(lve::LveDescriptorPool &pool);

  lve::LveDevice &lveDevice;

  std::unique_ptr<lve::LvePipeline> lveGraphicsPipeline;
  VkPipelineLayout graphicsPipelineLayout;
  std::unique_ptr<lve::LvePipeline> lveComputePipeline;
  VkPipelineLayout computePipelineLayout;

  std::vector<std::unique_ptr<lve::LveBuffer>> uniformBuffers;
  std::vector<std::unique_ptr<lve::LveBuffer>> shaderStorageBuffers;

  std::unique_ptr<lve::LveDescriptorSetLayout> graphicsDescriptorSetLayout;
  std::vector<VkDescriptorSet> graphicsDescriptorSets;

  std::unique_ptr<lve::LveDescriptorSetLayout> computeDescriptorSetLayout;
  std::vector<VkDescriptorSet> computeDescriptorSets;
};
}  // namespace tut