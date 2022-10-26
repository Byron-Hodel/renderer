#include "vulkan_internal.h"
#include <malloc.h>
#include <string.h>

static int8_t physical_device_supported(VkPhysicalDevice device);

int8_t _vulkan_init_physical_devices(vulkan_context_t* context) {
	VkPhysicalDevice* physical_devices;
	VkResult r = vkEnumeratePhysicalDevices(context->instance, &context->physical_device_count, NULL);
	if(r != VK_SUCCESS) return 0;
	physical_devices = malloc(sizeof(VkPhysicalDevice) * context->physical_device_count);
	context->physical_devices = malloc(sizeof(vulkan_physical_device) * context->physical_device_count);
	r = vkEnumeratePhysicalDevices(context->instance, &context->physical_device_count, physical_devices);
	if(r != VK_SUCCESS) {
		free(physical_devices);
		free(context->physical_devices);
	}

	for(int i = 0; i < context->physical_device_count; i++) {
		context->physical_devices[i].handle = physical_devices[i];
		context->physical_devices[i].supported = physical_device_supported(physical_devices[i]);
	}

	free(physical_devices);
	return 1;
}

void _vulkan_cleanup_physical_device(vulkan_context_t* context) {
	free(context->physical_devices);
	context->physical_devices = NULL;
}

int8_t _vulkan_select_physical_device(vulkan_context_t* context) {
	int8_t selected_device = 0;
	for(int i = 0; i < context->physical_device_count; i++) {
		if(context->physical_devices[i].supported == 0) continue;
		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(context->physical_devices[i].handle, &properties);
		if(properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
			context->selected_device.physical_device = context->physical_devices[i];
			return 1;
		}
		else if(properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
			selected_device = 1;
			context->selected_device.physical_device = context->physical_devices[i];
		}
	}
	return selected_device;
}

int8_t _vulkan_init_selected_device(vulkan_context_t* context) {
	VkPhysicalDeviceFeatures device_features;
	vkGetPhysicalDeviceFeatures(context->selected_device.physical_device.handle, &device_features);

	uint32_t queue_family_count;
	VkQueueFamilyProperties* queue_families;
	vkGetPhysicalDeviceQueueFamilyProperties(context->selected_device.physical_device.handle,
	                                         &queue_family_count, NULL);
	queue_families = calloc(sizeof(VkQueueFamilyProperties), queue_family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(context->selected_device.physical_device.handle,
	                                         &queue_family_count, queue_families);

	memset(context->selected_device.queue_indices, -1, sizeof(int32_t) * 4);
	for(int i = 0; i < queue_family_count; i++) {
		uint32_t queue_flags = queue_families[i].queueFlags;

		if((queue_flags & VK_QUEUE_GRAPHICS_BIT) == VK_QUEUE_GRAPHICS_BIT &&
		    context->selected_device.queue_indices[GRAPHICS_QUEUE_INDEX] == -1)
		{
			context->selected_device.queue_indices[GRAPHICS_QUEUE_INDEX] = i;
		}
		if((queue_flags & VK_QUEUE_COMPUTE_BIT) == VK_QUEUE_COMPUTE_BIT &&
		    context->selected_device.queue_indices[COMPUTE_QUEUE_INDEX] == -1)
		{
			context->selected_device.queue_indices[COMPUTE_QUEUE_INDEX] = i;
		}
		if((queue_flags & VK_QUEUE_TRANSFER_BIT) == VK_QUEUE_TRANSFER_BIT &&
		    context->selected_device.queue_indices[TRANSFER_QUEUE_INDEX] == -1)
		{
			context->selected_device.queue_indices[TRANSFER_QUEUE_INDEX] = i;
		}
		if(queue_flags < queue_families[context->selected_device.queue_indices[TRANSFER_QUEUE_INDEX]].queueFlags) {
			context->selected_device.queue_indices[TRANSFER_QUEUE_INDEX] = i;
		}
		if((queue_flags & VK_QUEUE_SPARSE_BINDING_BIT) == VK_QUEUE_SPARSE_BINDING_BIT &&
		    context->selected_device.queue_indices[SPARSE_BIND_QUEUE_INDEX] == -1)
		{
			context->selected_device.queue_indices[SPARSE_BIND_QUEUE_INDEX] = i;
		}
	}
	if(context->selected_device.queue_indices[GRAPHICS_QUEUE_INDEX] == -1) return 0;
	if(context->selected_device.queue_indices[COMPUTE_QUEUE_INDEX] == -1) return 0;
	if(context->selected_device.queue_indices[TRANSFER_QUEUE_INDEX] == -1) return 0;

	uint32_t queue_info_indices[4];
	uint32_t queue_info_count = 1;
	queue_info_indices[0] = context->selected_device.queue_indices[0];
	for(int i = 1; i < 4; i++) {
		int8_t unique_index = 1;
		for(int j = 0; j < i; j++) {
			if(context->selected_device.queue_indices[i] == context->selected_device.queue_indices[j] ||
			   context->selected_device.queue_indices[i] == -1) 
			{
				unique_index = 0;
				break;
			}
		}
		if(unique_index) {
			queue_info_indices[queue_info_count++] = context->selected_device.queue_indices[unique_index];
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
	device_create_info.enabledLayerCount = layer_count;
	device_create_info.ppEnabledLayerNames = layer_names;
	device_create_info.enabledExtensionCount = device_extension_count;
	device_create_info.ppEnabledExtensionNames = device_extension_names;
	device_create_info.pEnabledFeatures = &device_features;

	VkResult device_result = vkCreateDevice(context->selected_device.physical_device.handle,
	                                     &device_create_info, NULL,
	                                     &context->selected_device.logical_device);
	if(device_result != VK_SUCCESS) {
		return 0;
	}

	for(int i = 0; i < 4; i++) {
		if(context->selected_device.queue_indices[i] != -1) {
			vkGetDeviceQueue(context->selected_device.logical_device,
			                 context->selected_device.queue_indices[i],
			                 0, &context->selected_device.queues[i]);
		}
		else {
			context->selected_device.queues[i] = VK_NULL_HANDLE;
		}
	}
	return 1;
}

void _vulkan_cleanup_selected_device(vulkan_context_t* context) {
	vkDestroyDevice(context->selected_device.logical_device, NULL);
	context->selected_device.physical_device.handle = VK_NULL_HANDLE;
	context->selected_device.physical_device.supported = 0;
	memset(context->selected_device.queue_indices, -1, sizeof(int32_t) * 4);
	memset(context->selected_device.queues, 0, sizeof(VkQueue) * 4);
}


static int8_t physical_device_supported(VkPhysicalDevice device) {
	if(device_extension_count == 0) return 1; // if no extensions are required return true
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

	for(int i = 0; i < device_extension_count; i++) {
		int8_t found_extension = 0;
		for(int j = 0; j < extension_property_count; j++) {
			if(strcmp(device_extension_names[i], device_extension_properties[j].extensionName) == 0) {
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