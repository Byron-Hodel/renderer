#include "vulkan_image.h"

int8_t vulkan_image_create(const vulkan_context_t* context, vulkan_image_t* image, VkImageType type,
                           uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
                           VkImageUsageFlags usage, VkMemoryPropertyFlags mem_flags, VkImageAspectFlags aspect_flags)
{
	VkImageCreateInfo image_info = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
	image_info.imageType = type;
	image_info.format = format;
	image_info.extent.width = width;
	image_info.extent.height = height;
	image_info.extent.depth = 1;
	image_info.mipLevels = 4;
	image_info.arrayLayers = 1;
	image_info.samples = VK_SAMPLE_COUNT_1_BIT;
	image_info.tiling = tiling;
	image_info.usage = usage;
	image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	VkResult r = vkCreateImage(context->selected_device.handle, &image_info, NULL, &image->handle);
	if(r != VK_SUCCESS) return 0;

	VkMemoryRequirements mem_requirements;
	vkGetImageMemoryRequirements(context->selected_device.handle, image->handle, &mem_requirements);


	VkPhysicalDevice physical_device = context->physical_devices.handles[context->selected_device.device_index];
	uint32_t mem_type_index = 0;
	VkPhysicalDeviceMemoryProperties memory_properties;
	vkGetPhysicalDeviceMemoryProperties(physical_device, &memory_properties);
	for(uint32_t i = 0; i < memory_properties.memoryTypeCount; i++) {
		if(mem_requirements.memoryTypeBits & (1 << i) && (memory_properties.memoryTypes[i].propertyFlags & mem_flags) == mem_flags) {
			mem_type_index = i;
			break;
		}
	}

	VkMemoryAllocateInfo alloc_info = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
	alloc_info.allocationSize = mem_requirements.size;
	alloc_info.memoryTypeIndex = mem_type_index;
	vkAllocateMemory(context->selected_device.handle, &alloc_info, NULL, &image->device_memory_handle);
	vkBindImageMemory(context->selected_device.handle, image->handle, image->device_memory_handle, 0);


	VkImageViewCreateInfo view_info = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
	view_info.image = image->handle;
	view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	view_info.format = format;
	view_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	view_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	view_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	view_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	view_info.subresourceRange.baseMipLevel = 0;
	view_info.subresourceRange.levelCount = 1;
	view_info.subresourceRange.baseArrayLayer = 0;
	view_info.subresourceRange.layerCount = 1;
	view_info.subresourceRange.aspectMask = aspect_flags;

	vkCreateImageView(context->selected_device.handle, &view_info, NULL, &image->view);

	return 1;
}

void vulkan_image_destroy(const vulkan_context_t* context, vulkan_image_t* image) {
	vkDestroyImage(context->selected_device.handle, image->handle, NULL);
	vkDestroyImageView(context->selected_device.handle, image->view, NULL);
	vkFreeMemory(context->selected_device.handle, image->device_memory_handle, NULL);
	image->handle = VK_NULL_HANDLE;
	image->view = VK_NULL_HANDLE;
	image->device_memory_handle = VK_NULL_HANDLE;
}