#ifndef VULKAN_IMAGE_H
#define VULKAN_IMAGE_H

#include "vulkan_types.h"

int8_t vulkan_image_create(const vulkan_context_t context, vulkan_image_t* image, VkImageType type,
                           uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
                           VkImageUsageFlags usage, VkMemoryPropertyFlags mem_flags, VkImageAspectFlags aspect_flags);

void vulkan_image_destroy(const vulkan_context_t context, vulkan_image_t* image);

#endif // VULKAN_IMAGE_H