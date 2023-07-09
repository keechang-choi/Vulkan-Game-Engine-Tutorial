#pragma once

#include "lve_device.hpp"
#include "lve_swap_chain.hpp"
#include "lve_window.hpp"

// std
#include <cassert>
#include <memory>
#include <vector>

namespace lve {
class LveRenderer {
 public:
  LveRenderer(LveWindow &window, LveDevice &device);
  ~LveRenderer();

  LveRenderer(const LveRenderer &) = delete;
  LveRenderer &operator=(const LveRenderer &) = delete;

  VkRenderPass getSwapChainRenderPass() const {
    return lveSwapChain->getRenderPass();
  }
  float getAspectRatio() const { return lveSwapChain->extentAspectRatio(); }
  bool isFrameInProgress() const { return isFrameStarted; }

  VkCommandBuffer getCurrentCommandBuffer() const {
    assert(isFrameStarted &&
           "Cannot get command buffer when frame not in progress.");
    return commandBuffers[currentFrameIndex];
  }

  int getFrameIndex() const {
    assert((isFrameStarted || isComputeFrameStarted) &&
           "Cannot get frame index when frame not in progress.");
    return currentFrameIndex;
  }
  VkCommandBuffer beginFrame();
  void endFrame();
  void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
  void endSwapChainRenderPass(VkCommandBuffer commandBuffer);
  VkCommandBuffer beginComputeFrame();
  void endComputeFrame();
  VkCommandBuffer getCurrentComputeCommandBuffer() const {
    assert(isComputeFrameStarted &&
           "Cannot get compute command buffer when frame not in progress.");
    return computeCommandBuffers[currentFrameIndex];
  }

 private:
  void createCommandBuffers();
  void freeCommandBuffers();
  void recreateSwapChain();
  void createComputeCommandBuffers();
  void freeComputeCommandBuffers();

  LveWindow &lveWindow;
  LveDevice &lveDevice;
  std::unique_ptr<LveSwapChain> lveSwapChain;
  std::vector<VkCommandBuffer> commandBuffers;
  std::vector<VkCommandBuffer> computeCommandBuffers;

  uint32_t currentImageIndex;
  int currentFrameIndex{0};
  bool isFrameStarted{false};
  bool isComputeFrameStarted{false};
};
}  // namespace lve