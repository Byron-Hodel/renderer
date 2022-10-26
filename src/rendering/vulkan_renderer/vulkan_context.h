#ifndef VULKAN_CONTEXT_H
#define VULKAN_CONTEXT_H

#include <vulkan/vulkan.h>

typedef struct {
	VkPhysicalDevice handle;
	int8_t supported;
} vulkan_physical_device;

typedef struct {
	vulkan_physical_device physical_device;
	VkDevice logical_device;
	int32_t queue_indices[4];
	VkQueue queues[4];
} vulkan_device_t;

typedef struct {
	VkInstance instance;
	uint32_t physical_device_count;
	vulkan_physical_device* physical_devices;
	vulkan_device_t selected_device;
} vulkan_context_t;

#endif // VULKAN_CONTEXT_H