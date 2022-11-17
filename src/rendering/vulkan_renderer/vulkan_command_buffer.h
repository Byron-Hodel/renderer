#ifndef VULKAN_COMMAND_POOL_H
#define VULKAN_COMMAND_POOL_H

#include "vulkan_types.h"

int8_t vulkan_command_buffer_alloc(const vulkan_context_t context, vulkan_command_buffer_t* buffer,
                                   VkCommandPool pool, const int8_t is_primary);

void vulkan_command_buffer_free(const vulkan_context_t context, vulkan_command_buffer_t* buffer, VkCommandPool pool);

void vulkan_command_buffer_begin_recording(vulkan_command_buffer_t* buffer, int8_t single_use,
                                           int8_t renderpass_continue, int8_t simultaneous_use);
void vulkan_command_buffer_end_recording(vulkan_command_buffer_t* buffer);
void vulkan_command_buffer_submit(vulkan_command_buffer_t* buffer);

void vulkan_command_buffer_reset(vulkan_command_buffer_t* buffer);

#endif // VULKAN_COMMAND_POOL_H