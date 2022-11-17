#ifndef VULKAN_FRAMEBUFFER_H
#define VULKAN_FRAMEBUFFER_H

#include "vulkan_types.h"

int8_t vulkan_framebuffer_create(const vulkan_context_t* context, vulkan_framebuffer_t* framebuffer,
                                 vulkan_renderpass_t* renderpass, const uint32_t width, const uint32_t height,
                                 uint32_t attachment_count, VkImageView* attachments);

void vulkan_framebuffer_destroy(const vulkan_context_t* context, vulkan_framebuffer_t* framebuffer);

#endif // VULKAN_FRAMEBUFFER_H