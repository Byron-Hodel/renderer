#ifndef VULKAN_SWAPCHAIN_H
#define VULKAN_SWAPCHAIN_H

#include "vulkan_types.h"

int8_t vulkan_swapchain_create(vulkan_context_t* context, platform_window_t* window, vulkan_swapchain_t* swapchain);
void vulkan_swapchain_destroy(const vulkan_context_t* context, vulkan_swapchain_t* swapchain);
int8_t vulkan_swapchain_recreate(vulkan_context_t* context, vulkan_swapchain_t* swapchain);

void vulkan_swapchain_present(vulkan_context_t* context, vulkan_swapchain_t* swapchain,
                              VkSemaphore render_complete_semaphore, uint32_t image_index);

#endif // VULKAN_SWAPCHAIN_H