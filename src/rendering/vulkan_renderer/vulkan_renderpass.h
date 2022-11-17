#ifndef VULKAN_RENDERPASS_H
#define VULKAN_RENDERPASS_H

#include "vulkan_types.h"

int8_t vulkan_create_renderpass(const vulkan_context_t* context, vulkan_renderpass_t* renderpass,
                                vec2_t pos, vec2_t extents, float depth, uint32_t stencil);
void vulkan_destroy_renderpass(const vulkan_context_t* context, vulkan_renderpass_t* renderpass);

void vulkan_renderpass_begin(vulkan_renderpass_t* renderpass, vulkan_command_buffer_t* buffer, vulkan_framebuffer_t* framebuffer);
void vulkan_renderpass_end(vulkan_renderpass_t* renderpass, vulkan_command_buffer_t* buffer);

#endif // VULKAN_RENDERPASS_H