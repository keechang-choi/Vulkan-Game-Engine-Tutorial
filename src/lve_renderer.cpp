#include "lve_renderer.hpp"

// std
#include <array>
#include <iostream>
#include <memory>
#include <stdexcept>

namespace lve {

LveRenderer::LveRenderer(LveWindow& window, LveDevice& device)
    : lveWindow{window}, lveDevice{device} {
  recreateSwapChain();
  createCommandBuffers();
  createComputeCommandBuffers();
}
LveRenderer::~LveRenderer() {
  freeCommandBuffers();
  freeComputeCommandBuffers();
}

void LveRenderer::recreateSwapChain() {
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
    std::shared_ptr<LveSwapChain> oldSwapChain = std::move(lveSwapChain);
    lveSwapChain =
        std::make_unique<LveSwapChain>(lveDevice, extent, oldSwapChain);
    if (!oldSwapChain->compareSwapFormats(*lveSwapChain.get())) {
      throw std::runtime_error(
          "Swap Chain image(or depth) format has changed!");
    }
  }
  // some systems two swapchain cant coexist on the same window.
  // lveSwapChain = nullptr;
  // compatible -> may we not need to recreate
  // createPipeline();
}
void LveRenderer::createCommandBuffers() {
  commandBuffers.resize(LveSwapChain::MAX_FRAMES_IN_FLIGHT);

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

void LveRenderer::freeCommandBuffers() {
  vkFreeCommandBuffers(lveDevice.device(), lveDevice.getCommandPool(),
                       static_cast<uint32_t>(commandBuffers.size()),
                       commandBuffers.data());
  commandBuffers.clear();
}

// void LveRenderer::renderGameObjects(VkCommandBuffer commandBuffer) {
//   // update
//   int i = 0;
//   for (auto& obj : gameObjects) {
//     i++;
//     obj.transform2d.rotation = glm::mod<float>(
//         obj.transform2d.rotation + 0.00001f * i, glm::two_pi<float>());
//   }
//   // render
//   lvePipeline->bind(commandBuffer);
//   for (auto& obj : gameObjects) {
//     SimplePushConstantData push{};
//     push.offset = obj.transform2d.translation;
//     push.color = obj.color;
//     push.transform = obj.transform2d.mat2();
//
//     vkCmdPushConstants(
//         commandBuffer, pipelineLayout,
//         VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
//         sizeof(SimplePushConstantData), &push);
//     obj.model->bind(commandBuffer);
//     obj.model->draw(commandBuffer);
//   }
// }

VkCommandBuffer LveRenderer::beginFrame() {
  assert(!isFrameStarted && "Can't call beginFrame while already in progress.");

  auto result = lveSwapChain->acquireNextImage(&currentImageIndex);
  // window resized
  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    recreateSwapChain();
  }

  if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    throw std::runtime_error("failed to acquire swap chain image!");
  }

  isFrameStarted = true;

  auto commandBuffer = getCurrentCommandBuffer();
  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
    throw std::runtime_error("failed to begin recording command buffer!");
  }
  return commandBuffer;
}
void LveRenderer::endFrame() {
  assert(isFrameStarted &&
         "Can't call endFrame while frame is not in progress.");
  auto commandBuffer = getCurrentCommandBuffer();

  if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
    throw std::runtime_error("failed to record command buffer!");
  }

  auto result =
      lveSwapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex);
  // suboptimal : no longer matches the surface properties exactly, but can
  // still be used.
  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
      lveWindow.wasWindowResized()) {
    lveWindow.resetWindowResizedFlag();
    recreateSwapChain();
  } else if (result != VK_SUCCESS) {
    throw std::runtime_error("failed to present swap chain image!");
  }
  isFrameStarted = false;
  currentFrameIndex =
      (currentFrameIndex + 1) % LveSwapChain::MAX_FRAMES_IN_FLIGHT;
}
void LveRenderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer) {
  assert(isFrameStarted &&
         "Can't call beginSwapChainRenderPass if frame is not in progress.");
  assert(commandBuffer == getCurrentCommandBuffer() &&
         "Can't begin render pass on command buffer from a different frame.");

  VkRenderPassBeginInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassInfo.renderPass = lveSwapChain->getRenderPass();
  renderPassInfo.framebuffer = lveSwapChain->getFrameBuffer(currentImageIndex);

  renderPassInfo.renderArea.offset = {0, 0};
  renderPassInfo.renderArea.extent = lveSwapChain->getSwapChainExtent();

  std::array<VkClearValue, 2> clearValues{};
  clearValues[0].color = {0.01f, 0.01f, 0.01f, 1.0f};
  clearValues[1].depthStencil = {1.0f, 0};
  renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
  renderPassInfo.pClearValues = clearValues.data();

  vkCmdBeginRenderPass(commandBuffer, &renderPassInfo,
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
  vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
  vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}
void LveRenderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer) {
  assert(isFrameStarted &&
         "Can't call endSwapChainRenderPass if frame is not in progress.");
  assert(commandBuffer == getCurrentCommandBuffer() &&
         "Can't end render pass on command buffer from a different frame.");

  vkCmdEndRenderPass(commandBuffer);
}

void LveRenderer::createComputeCommandBuffers() {
  computeCommandBuffers.resize(LveSwapChain::MAX_FRAMES_IN_FLIGHT);

  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = lveDevice.getCommandPool();
  allocInfo.commandBufferCount =
      static_cast<uint32_t>(computeCommandBuffers.size());

  if (vkAllocateCommandBuffers(lveDevice.device(), &allocInfo,
                               computeCommandBuffers.data()) != VK_SUCCESS) {
    throw std::runtime_error("failed to allocate compute command buffers!");
  }
}
void LveRenderer::freeComputeCommandBuffers() {
  vkFreeCommandBuffers(lveDevice.device(), lveDevice.getCommandPool(),
                       static_cast<uint32_t>(computeCommandBuffers.size()),
                       computeCommandBuffers.data());
  computeCommandBuffers.clear();
}

VkCommandBuffer LveRenderer::beginComputeFrame() {
  assert(!isComputeFrameStarted &&
         "Can't call beginComputeFrame while already in progress.");

  // call swapchain compute fence wait
  lveSwapChain->prepareCompute();

  isComputeFrameStarted = true;
  auto computeCommandBuffer = getCurrentComputeCommandBuffer();
  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  if (vkBeginCommandBuffer(computeCommandBuffer, &beginInfo) != VK_SUCCESS) {
    throw std::runtime_error(
        "failed to begin recording compute command buffer!");
  }
  return computeCommandBuffer;
}

void LveRenderer::endComputeFrame() {
  assert(isComputeFrameStarted &&
         "Can't call endComputeFrame while frame is not in progress.");
  auto computeCommandBuffer = getCurrentComputeCommandBuffer();

  if (vkEndCommandBuffer(computeCommandBuffer) != VK_SUCCESS) {
    throw std::runtime_error("failed to record compute command buffer!");
  }
  // call swapchain compute queue submit
  lveSwapChain->submitComputeCommandBuffers(&computeCommandBuffer);

  isComputeFrameStarted = false;
}

}  // namespace lve
