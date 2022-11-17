#include "vulkan_device.h"
#include <string.h>
#include <malloc.h>

static int8_t physical_device_supported(const vulkan_context_t context, VkPhysicalDevice device);

int8_t vulkan_init_physical_devices(const vulkan_context_t context, vulkan_physical_devices_t* physical_devices) {
	VkResult r = vkEnumeratePhysicalDevices(context.instance, &physical_devices->count, NULL);
	if(r != VK_SUCCESS) return 0;
	physical_devices->handles = malloc(sizeof(VkPhysicalDevice) * physical_devices->count);
	physical_devices->supported = malloc(sizeof(uint8_t) * physical_devices->count);
	r = vkEnumeratePhysicalDevices(context.instance, &physical_devices->count, physical_devices->handles);
	if(r != VK_SUCCESS) {
		free(physical_devices->handles);
		free(physical_devices->supported);
		return 0;
	};
	for(uint32_t i = 0; i < physical_devices->count; i++) {
		physical_devices->supported[i] = physical_device_supported(context, physical_devices->handles[i]);
	}
	return 1;
}

void vulkan_cleanup_physical_devices(vulkan_physical_devices_t* physical_devices) {
	free(physical_devices->handles);
	free(physical_devices->supported);
	physical_devices->handles = NULL;
	physical_devices->supported = NULL;
	physical_devices->count = 0;
}

int32_t vulkan_select_physical_device(const vulkan_physical_devices_t physical_devices) {
	int32_t device_index = -1;
	for(uint32_t i = 0; i < physical_devices.count; i++) {
		if(physical_devices.supported[i] == 0) continue;
		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(physical_devices.handles[i], &properties);
		if(properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
			return i;
		}
		else if(properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
			device_index = i;
		}
	}
	return device_index;
}

int8_t vulkan_init_device(const vulkan_context_t context, vulkan_device_t* device, const uint32_t device_index) {
	if(context.physical_devices.supported[device_index] == 0) return 0;
	VkPhysicalDevice physical_device = context.physical_devices.handles[device_index];

	VkPhysicalDeviceFeatures device_features;
	vkGetPhysicalDeviceFeatures(physical_device, &device_features);

	uint32_t queue_family_count;
	VkQueueFamilyProperties* queue_families;
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, NULL);
	queue_families = calloc(sizeof(VkQueueFamilyProperties), queue_family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families);

	memset(device->queue_family_indices, -1, sizeof(int32_t) * 4);
	for(int i = 0; i < queue_family_count; i++) {
		uint32_t queue_flags = queue_families[i].queueFlags;

		if(queue_flags & VK_QUEUE_GRAPHICS_BIT && device->queue_family_indices[GRAPHICS_QUEUE_INDEX] == -1) {
			device->queue_family_indices[GRAPHICS_QUEUE_INDEX] = i;
		}
		if(queue_flags & VK_QUEUE_COMPUTE_BIT && device->queue_family_indices[COMPUTE_QUEUE_INDEX] == -1) {
			device->queue_family_indices[COMPUTE_QUEUE_INDEX] = i;
		}
		if(queue_flags & VK_QUEUE_TRANSFER_BIT && device->queue_family_indices[TRANSFER_QUEUE_INDEX] == -1) {
			device->queue_family_indices[TRANSFER_QUEUE_INDEX] = i;
		}
		if(queue_flags < queue_families[device->queue_family_indices[TRANSFER_QUEUE_INDEX]].queueFlags) {
			device->queue_family_indices[TRANSFER_QUEUE_INDEX] = i;
		}
		if(queue_flags & VK_QUEUE_SPARSE_BINDING_BIT && device->queue_family_indices[SPARSE_BIND_QUEUE_INDEX] == -1) {
			device->queue_family_indices[SPARSE_BIND_QUEUE_INDEX] = i;
		}
	}
	if(device->queue_family_indices[GRAPHICS_QUEUE_INDEX] == -1) return 0;
	if(device->queue_family_indices[COMPUTE_QUEUE_INDEX] == -1) return 0;
	if(device->queue_family_indices[TRANSFER_QUEUE_INDEX] == -1) return 0;

	uint32_t queue_info_indices[4];
	uint32_t queue_info_count = 1;
	queue_info_indices[0] = device->queue_family_indices[0];
	for(int i = 1; i < 4; i++) {
		int8_t unique_index = 1;
		for(int j = 0; j < i; j++) {
			if(device->queue_family_indices[i] == device->queue_family_indices[j]) {
				unique_index = 0;
				break;
			}
		}
		if(unique_index) {
			queue_info_indices[queue_info_count++] = device->queue_family_indices[i];
		}
	}
	VkDeviceQueueCreateInfo queue_infos[4] = {0};
	for(int i = 0; i < queue_info_count; i++) {
		float queue_priority = 1.0f;
		queue_infos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_infos[i].queueFamilyIndex = queue_info_indices[i];
		queue_infos[i].queueCount = 1;
		queue_infos[i].pQueuePriorities = &queue_priority;
	}

	VkDeviceCreateInfo device_create_info = {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
	device_create_info.queueCreateInfoCount = queue_info_count;
	device_create_info.pQueueCreateInfos = queue_infos;
	device_create_info.enabledLayerCount = context.layer_count;
	device_create_info.ppEnabledLayerNames = (const char* const*)context.layer_names;
	device_create_info.enabledExtensionCount = context.device_extension_count;
	device_create_info.ppEnabledExtensionNames = (const char* const*)context.device_extension_names;
	device_create_info.pEnabledFeatures = &device_features;

	VkResult device_result = vkCreateDevice(physical_device, &device_create_info, NULL, &device->handle);
	if(device_result != VK_SUCCESS) {
		return 0;
	}

	for(int i = 0; i < 4; i++) {
		device->queues[i] = VK_NULL_HANDLE;
		if(device->queue_family_indices[i] != -1)
			vkGetDeviceQueue(device->handle, device->queue_family_indices[i], 0, &device->queues[i]);
	}

	VkCommandPoolCreateInfo graphics_cmd_pool_info = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
	graphics_cmd_pool_info.queueFamilyIndex = device->queue_family_indices[GRAPHICS_QUEUE_INDEX];
	graphics_cmd_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	VkResult cmd_buffer_result = vkCreateCommandPool(device->handle, &graphics_cmd_pool_info, NULL, &device->graphics_cmd_pool);
	if(cmd_buffer_result != VK_SUCCESS) {
		vkDestroyDevice(device->handle, NULL);
		return 0;
	}

	device->device_index = device_index;
	return 1;
}

void vulkan_cleanup_device(vulkan_device_t* device) {
	vkDeviceWaitIdle(device->handle);
	vkDestroyCommandPool(device->handle, device->graphics_cmd_pool, NULL);
	vkDestroyDevice(device->handle, NULL);
	device->handle = NULL;
	memset(device->queue_family_indices, -1, sizeof(int32_t) * 4);
	memset(device->queues, 0, sizeof(VkQueue) * 4);
}


static int8_t physical_device_supported(const vulkan_context_t context, VkPhysicalDevice device) {
	if(context.device_extension_count == 0) return 1; // if no extensions are required return true
	uint32_t extension_property_count = 0;
	VkExtensionProperties device_extension_properties[512];
	VkResult r = vkEnumerateDeviceExtensionProperties(device, NULL, &extension_property_count, NULL);
	if(r != VK_SUCCESS) {
		return 0;
	}
	r = vkEnumerateDeviceExtensionProperties(device, NULL, &extension_property_count, device_extension_properties);
	if(r != VK_SUCCESS) {
		return 0;
	}

	for(int i = 0; i < context.device_extension_count; i++) {
		int8_t found_extension = 0;
		for(int j = 0; j < extension_property_count; j++) {
			if(strcmp(context.device_extension_names[i], device_extension_properties[j].extensionName) == 0) {
				found_extension = 1;
				break;
			}
		}
		if(!found_extension) {
			VkPhysicalDeviceProperties props;
			vkGetPhysicalDeviceProperties(device, &props);
			return 0;
		}
	}
	VkPhysicalDeviceProperties props;
	vkGetPhysicalDeviceProperties(device, &props);
	return 1;
}