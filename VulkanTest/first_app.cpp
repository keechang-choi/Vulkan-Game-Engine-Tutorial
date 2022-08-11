#include "first_app.hpp"

#include <glm/glm.hpp>

// std
#include <array>
#include <iostream>
#include <memory>
#include <stdexcept>

namespace {
std::vector<lve::LveModel::Vertex> generate_sierpinski(
    int level, const std::vector<lve::LveModel::Vertex>& current_triangles) {
  std::vector<lve::LveModel::Vertex> return_triangles;
  if (level == 0) {
    return_triangles = current_triangles;
  } else {
    std::vector<lve::LveModel::Vertex> prev_triangles;
    prev_triangles = generate_sierpinski(level - 1, current_triangles);

    int num_triangles = static_cast<int>(prev_triangles.size()) / 3;
    for (int i = 0; i < num_triangles; i++) {
      for (int j = 0; j < 3; j++) {
        int prev_j = (j + 3 - 1) % 3;
        int next_j = (j + 1) % 3;
        const lve::LveModel::Vertex& current_vertex = prev_triangles[3 * i + j];
        const lve::LveModel::Vertex& prev_vertex =
            prev_triangles[3 * i + prev_j];
        const lve::LveModel::Vertex& next_vertex =
            prev_triangles[3 * i + next_j];

        glm::vec2 p1 = current_vertex.position;
        glm::vec2 p2 =
            glm::mix(current_vertex.position, next_vertex.position, 0.5);
        glm::vec2 p3 =
            glm::mix(current_vertex.position, prev_vertex.position, 0.5);

        glm::vec3 c1 = current_vertex.color;
        glm::vec3 c2 = glm::mix(current_vertex.color, next_vertex.color, 0.5);
        glm::vec3 c3 = glm::mix(current_vertex.color, prev_vertex.color, 0.5);

        return_triangles.push_back({p1, c1});
        return_triangles.push_back({p2, c2});
        return_triangles.push_back({p3, c3});
      }
    }
  }
  return return_triangles;
}
}  // namespace

namespace lve {

FirstApp::FirstApp() {
  loadModels();
  createPipelineLayout();
  recreateSwapChain();
  createCommandBuffers();
}
FirstApp::~FirstApp() {
  vkDestroyPipelineLayout(lveDevice.device(), pipelineLayout, nullptr);
}

void FirstApp::run() {
  {
    while (!lveWindow.shouldClose()) {
      glfwPollEvents();
      drawFrame();
    }
    vkDeviceWaitIdle(lveDevice.device());
  }
}

void FirstApp::loadModels() {
  std::vector<LveModel::Vertex> vertices{{{0.0f, -0.9f}, {1.0f, 0.0f, 0.0f}},
                                         {{0.9f, 0.9f}, {0.0f, 1.0f, 0.0f}},
                                         {{-0.9f, 0.9f}, {0.0f, 0.0f, 1.0f}}};
  std::vector<LveModel::Vertex> sierpinski_vertices =
      ::generate_sierpinski(7, vertices);
  lveModel = std::make_unique<LveModel>(lveDevice, sierpinski_vertices);
}

void FirstApp::createPipelineLayout() {
  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = 0;
  pipelineLayoutInfo.pSetLayouts = nullptr;
  pipelineLayoutInfo.pushConstantRangeCount = 0;
  pipelineLayoutInfo.pPushConstantRanges = nullptr;
  if (vkCreatePipelineLayout(lveDevice.device(), &pipelineLayoutInfo, nullptr,
                             &pipelineLayout) != VK_SUCCESS) {
    throw std::runtime_error("failed to create pipeline layout!");
  }
}
void FirstApp::createPipeline() {
  assert(lveSwapChain != nullptr &&
         "Cannot create pipeline before swap chain.");
  assert(pipelineLayout != nullptr &&
         "Cannot create pipeline before pipeline layout.");

  PipelineConfigInfo pipelineConfig{};
  LvePipeline::defaultPipelineConfigInfo(pipelineConfig);
  pipelineConfig.renderPass = lveSwapChain->getRenderPass();
  pipelineConfig.pipelineLayout = pipelineLayout;
  lvePipeline =
      std::make_unique<LvePipeline>(lveDevice,
                                    "C:"
                                    "\\Users\\rlckd\\source\\repos\\VulkanTes"
                                    "t\\VulkanTest\\shaders\\\simple_"
                                    "shader.vert.spv",
                                    "C:"
                                    "\\Users\\rlckd\\source\\repos\\VulkanTes"
                                    "t\\VulkanTest\\shaders\\\simple_"
                                    "shader.frag.spv",
                                    pipelineConfig);
}

void FirstApp::recreateSwapChain() {
  auto extent = lveWindow.getExtent();
  // ex minimize
  while (extent.width == 0 || extent.height == 0) {
    extent = lveWindow.getExtent();
    glfwWaitEvents();
  }
  vkDeviceWaitIdle(lveDevice.device());
  if (lveSwapChain == nullptr) {
    lveSwapChain = std::make_unique<LveSwapChain>(lveDevice, extent);
  } else {
    lveSwapChain = std::make_unique<LveSwapChain>(lveDevice, extent,
                                                  std::move(lveSwapChain));
    if (lveSwapChain->imageCount() != commandBuffers.size()) {
      freeCommandBuffers();
      createCommandBuffers();
    }
  }
  // some systems two swapchain cant coexist on the same window.
  // lveSwapChain = nullptr;
  createPipeline();
}
void FirstApp::createCommandBuffers() {
  commandBuffers.resize(lveSwapChain->imageCount());

  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = lveDevice.getCommandPool();
  allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

  if (vkAllocateCommandBuffers(lveDevice.device(), &allocInfo,
                               commandBuffers.data()) != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate command buffers!");
  }
}

void FirstApp::freeCommandBuffers() {
  vkFreeCommandBuffers(lveDevice.device(), lveDevice.getCommandPool(),
                       static_cast<uint32_t>(commandBuffers.size()),
                       commandBuffers.data());
  commandBuffers.clear();
}

void FirstApp::recordCommandBuffer(int imageIndex) {
  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  if (vkBeginCommandBuffer(commandBuffers[imageIndex], &beginInfo) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to begin recording command buffer!");
  }

  VkRenderPassBeginInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassInfo.renderPass = lveSwapChain->getRenderPass();
  renderPassInfo.framebuffer = lveSwapChain->getFrameBuffer(imageIndex);

  renderPassInfo.renderArea.offset = {0, 0};
  renderPassInfo.renderArea.extent = lveSwapChain->getSwapChainExtent();

  std::array<VkClearValue, 2> clearValues{};
  clearValues[0].color = {0.1f, 0.1f, 0.1f, 0.1f};
  clearValues[1].depthStencil = {1.0f, 0};
  renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
  renderPassInfo.pClearValues = clearValues.data();

  vkCmdBeginRenderPass(commandBuffers[imageIndex], &renderPassInfo,
                       VK_SUBPASS_CONTENTS_INLINE);
  VkViewport viewport{};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = static_cast<float>(lveSwapChain->getSwapChainExtent().width);
  viewport.height =
      static_cast<float>(lveSwapChain->getSwapChainExtent().height);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  VkRect2D scissor{{0, 0}, lveSwapChain->getSwapChainExtent()};
  vkCmdSetViewport(commandBuffers[imageIndex], 0, 1, &viewport);
  vkCmdSetScissor(commandBuffers[imageIndex], 0, 1, &scissor);

  lvePipeline->bind(commandBuffers[imageIndex]);
  lveModel->bind(commandBuffers[imageIndex]);
  lveModel->draw(commandBuffers[imageIndex]);
  vkCmdEndRenderPass(commandBuffers[imageIndex]);
  if (vkEndCommandBuffer(commandBuffers[imageIndex]) != VK_SUCCESS) {
    throw std::runtime_error("failed to record command buffer!");
  }
}

void FirstApp::drawFrame() {
  uint32_t imageIndex;
  auto result = lveSwapChain->acquireNextImage(&imageIndex);

  // window resized
  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    recreateSwapChain();
    return;
  }

  if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    throw std::runtime_error("failed to acquire swap chain image!");
  }

  recordCommandBuffer(imageIndex);
  result = lveSwapChain->submitCommandBuffers(&commandBuffers[imageIndex],
                                              &imageIndex);
  // suboptimal : no longer matches the surface properties exactly, but can
  // still be used.
  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
      lveWindow.wasWindowResized()) {
    lveWindow.resetWindowResizedFlag();
    recreateSwapChain();
    return;
  }
  if (result != VK_SUCCESS) {
    throw std::runtime_error("failed to present swap chain image!");
  }
}

}  // namespace lve