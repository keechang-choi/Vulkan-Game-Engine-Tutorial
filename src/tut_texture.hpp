#pragma once

#include <lve_device.hpp>

namespace tut {
VkImageView createImageView(lve::LveDevice& device, VkImage image,
                            VkFormat format);

class TutImage {
 public:
  TutImage(lve::LveDevice& device, uint32_t width, uint32_t height,
           VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
           VkMemoryPropertyFlags properties);
  ~TutImage();

  TutImage(const TutImage&) = delete;
  TutImage& operator=(const TutImage&) = delete;
  VkImage getImage() const { return image; }

 private:
  lve::LveDevice& lveDevice;
  VkImage image = VK_NULL_HANDLE;
  VkDeviceMemory imageMemory = VK_NULL_HANDLE;
  void* mapped;
};
}  // namespace tut