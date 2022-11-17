#ifndef VULKAN_RENDERER_H
#define VULKAN_RENDERER_H

#include "rendering/renderer.h"

int8_t vulkan_renderer_init(const char* app_name);
void vulkan_renderer_shutdown(void);

graphics_pipeline_t* vulkan_renderer_create_graphics_pipeline(const graphics_pipeline_create_info_t create_info);
void vulkan_renderer_destroy_graphics_pipeline(graphics_pipeline_t* pipeline);

int8_t vulkan_renderer_swapchain_init(swapchain_t* swapchain, platform_window_t* window);
void vulkan_renderer_swapchain_cleanup(swapchain_t* swapchain);
void vulkan_renderer_swapchain_next_framebuffer(swapchain_t swapchain, framebuffer_t* buffer);

int8_t vulkan_renderer_begin_draw(framebuffer_t target_buffer);
int8_t vulkan_renderer_end_draw(framebuffer_t target_buffer);

void draw_triangle(framebuffer_t target_buffer, graphics_pipeline_t* pipeline);

#endif // VULKAN_RENDERER_H