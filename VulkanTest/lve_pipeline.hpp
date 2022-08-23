#pragma once

#include "lve_device.hpp"

// std
#include <string>
#include <vector>

namespace lve {

struct PipelineConfigInfo {
  PipelineConfigInfo(const PipelineConfigInfo&) = delete;
  PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;

  VkPipelineViewportStateCreateInfo viewportInfo;
  VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
  VkPipelineRasterizationStateCreateInfo rasterizationInfo;
  // multi sample
  VkPipelineMultisampleStateCreateInfo multisampleInfo;
  // color blend attachment
  // color blend
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
  LvePipeline(LveDevice& device, const std::string& vertFilepath,
              const std::string& fragFilepath,
              const PipelineConfigInfo& configInfo);

  ~LvePipeline();

  LvePipeline(const LvePipeline&) = delete;
  LvePipeline& operator=(const LvePipeline&) = delete;

  void bind(VkCommandBuffer commandBuffers);

  static void defaultPipelineConfigInfo(PipelineConfigInfo& configInfo);

 private:
  static std::vector<char> readFile(const std::string& filepath);

  void createGraphicsPipeline(const std::string& vertFilepath,
                              const std::string& fragFilepath,
                              const PipelineConfigInfo& configInfo);

  void createShaderModule(const std::vector<char>& code,
                          VkShaderModule* shaderModule);

  LveDevice& lveDevice;
  VkPipeline graphicsPipeline;
  VkShaderModule vertShaderModule;
  VkShaderModule fragShaderModule;
};
}  // namespace lve