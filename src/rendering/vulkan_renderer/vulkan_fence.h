#ifndef VULKAN_FENCE_H
#define VULKAN_FENCE_H

#include "vulkan_types.h"

int8_t vulkan_fence_create(const vulkan_context_t context, vulkan_fence_t* fence);
void vulkan_fence_destroy(const vulkan_context_t context, vulkan_fence_t* fence);

int8_t vulkan_fence_wait(const vulkan_context_t context, vulkan_fence_t* fence, const uint64_t timeout_ns);
void vulkan_fence_reset(const vulkan_context_t context, vulkan_fence_t* fence);

#endif // VULKAN_FENCE_H