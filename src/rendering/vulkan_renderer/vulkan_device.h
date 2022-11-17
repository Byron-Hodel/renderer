#ifndef VULKAN_DEVICE_H
#define VULKAN_DEVICE_H

#include "vulkan_types.h"

int8_t vulkan_init_physical_devices(const vulkan_context_t context, vulkan_physical_devices_t* physical_devices);
void vulkan_cleanup_physical_devices(vulkan_physical_devices_t* physical_devices);

int32_t vulkan_select_physical_device(const vulkan_physical_devices_t physical_devices);

int8_t vulkan_init_device(const vulkan_context_t context, vulkan_device_t* device, const uint32_t device_index);
void vulkan_cleanup_device(vulkan_device_t* device);

#endif // VULKAN_DEVICE_H