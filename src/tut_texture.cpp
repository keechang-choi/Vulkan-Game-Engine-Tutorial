#include "tut_texture.hpp"

namespace tut {
TutImage::TutImage(lve::LveDevice& device, uint32_t width, uint32_t height,
                   VkFormat format, VkImageTiling tiling,
                   VkImageUsageFlags usage, VkMemoryPropertyFlags properties)
    : lveDevice{device} {
  VkImageCreateInfo imageInfo{};
  imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageInfo.imageType = VK_IMAGE_TYPE_2D;
  imageInfo.extent.width = width;
  imageInfo.extent.height = height;
  imageInfo.extent.depth = 1;
  imageInfo.mipLevels = 1;
  imageInfo.arrayLayers = 1;
  imageInfo.format = format;
  imageInfo.tiling = tiling;
  imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  imageInfo.usage = usage;
  imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  device.createImageWithInfo(imageInfo, properties, image, imageMemory);
}

TutImage::~TutImage() {
  vkDestroyImage(lveDevice.device(), image, nullptr);
  vkFreeMemory(lveDevice.device(), imageMemory, nullptr);
}
}  // namespace tut