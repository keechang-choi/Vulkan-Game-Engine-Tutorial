#pragma once

#include "lve_device.hpp"

// std
#include <string>
#include <vector>

namespace lve {

struct PipelineConfigInfo {
  PipelineConfigInfo() = default;
  PipelineConfigInfo(const PipelineConfigInfo&) = delete;
  PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;

  std::vector<VkVertexInputBindingDescription> bindingDescriptions{};
  std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
  VkPipelineViewportStateCreateInfo viewportInfo;
  VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
  VkPipelineRasterizationStateCreateInfo rasterizationInfo;
  // multi sample
  VkPipelineMultisampleStateCreateInfo multisampleInfo;
  // color blend attachment
  VkPipelineColorBlendAttachmentState colorBlendAttachment;
  // color blend
  VkPipelineColorBlendStateCreateInfo colorBlendInfo;
  VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
  std::vector<VkDynamicState> dynamicStateEnables;
  VkPipelineDynamicStateCreateInfo dynamicStateInfo;
  VkPipelineLayout pipelineLayout = nullptr;
  VkRenderPass renderPass = nullptr;
  uint32_t subpass = 0;
};
class LvePipeline {
 public:
  LvePipeline() = default;
  LvePipeline(LveDevice& device);

  ~LvePipeline();

  LvePipeline(const LvePipeline&) = delete;
  LvePipeline& operator=(const LvePipeline&) = delete;

  void bind(VkCommandBuffer commandBuffers);

  void createGraphicsPipeline(const std::string& vertFilepath,
                              const std::string& fragFilepath,
                              const PipelineConfigInfo& configInfo);
  void createComputePipeline(const std::string& compFilepath,
                             const PipelineConfigInfo& configInfo);

  static void defaultPipelineConfigInfo(PipelineConfigInfo& configInfo);
  static void enableAlphaBlending(PipelineConfigInfo& configInfo);

 private:
  static std::vector<char> readFile(const std::string& filepath);

  void createShaderModule(const std::vector<char>& code,
                          VkShaderModule* shaderModule);

  LveDevice& lveDevice;
  VkPipeline pipeline;
  VkPipelineBindPoint pipelineBindPoint;

  VkShaderModule vertShaderModule = VK_NULL_HANDLE;
  VkShaderModule fragShaderModule = VK_NULL_HANDLE;

  VkShaderModule compShaderModule = VK_NULL_HANDLE;
};
}  // namespace lve