#ifndef VULKAN_RENDERER_H
#define VULKAN_RENDERER_H

#include "rendering/renderer_internal.h"
#include "vulkan_context.h"
#include <vulkan/vulkan.h>

int8_t vulkan_init_context(vulkan_context_t* context, platform_context_t* platform_context, const char* app_name);
void vulkan_cleanup_context(vulkan_context_t* context);

render_target_t* vulkan_create_render_target(renderer_context_t* context, platform_window_t* platform_window);
void vulkan_destroy_render_target(renderer_context_t* context, render_target_t* target);
int8_t vulkan_recreate_render_target(renderer_context_t* context, render_target_t* target);

void vulkan_draw_triangle(renderer_context_t* context, render_target_t* target);

#define VULKAN_RENDERER_FUNCTIONS (renderer_functions_t) { \
	.create_render_target = vulkan_create_render_target, \
	.destroy_render_target = vulkan_destroy_render_target, \
	.recreate_render_target = vulkan_recreate_render_target, \
	.draw_triangle = vulkan_draw_triangle \
}

#endif // VULKAN_RENDERER_H