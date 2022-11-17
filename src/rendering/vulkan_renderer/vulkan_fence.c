#include "vulkan_fence.h"

int8_t vulkan_fence_create(const vulkan_context_t context, vulkan_fence_t* fence) {
	VkFenceCreateInfo fence_info = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
	if(fence->signaled) fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	VkResult r = vkCreateFence(context.selected_device.handle, &fence_info, NULL, &fence->handle);
	if(r != VK_SUCCESS) return 0;
	return 1;
}
void vulkan_fence_destroy(const vulkan_context_t context, vulkan_fence_t* fence) {
	vkDestroyFence(context.selected_device.handle, fence->handle, NULL);
	fence->signaled = 0;
}

int8_t vulkan_fence_wait(const vulkan_context_t context, vulkan_fence_t* fence, const uint64_t timeout_ns) {
	if(fence->signaled) return 1;
	VkResult r = vkWaitForFences(context.selected_device.handle, 1, &fence->handle, 1, timeout_ns);
	if(r == VK_SUCCESS) {
		fence->signaled = 1;
		return 1;
	}
	return 0;
}

void vulkan_fence_reset(const vulkan_context_t context, vulkan_fence_t* fence) {
	VkResult r = vkResetFences(context.selected_device.handle, 1, &fence->handle);
	if(r != VK_SUCCESS) {
		return;
	}
	fence->signaled = 0;
}