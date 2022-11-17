#ifndef VULKAN_PIPELINE_H
#define VULKAN_PIPELINE_H

#include "vulkan_types.h"
#include "rendering/renderer.h"

int8_t vulkan_create_graphics_pipeline(const vulkan_context_t* context, vulkan_graphics_pipeline_t* pipeline,
                                       graphics_pipeline_create_info_t create_info);

void vulkan_destroy_graphics_pipeline(const vulkan_context_t* context, vulkan_graphics_pipeline_t* pipeline);

#endif // VULKAN_PIPELINE_H