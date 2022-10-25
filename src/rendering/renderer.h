#ifndef RENDERER_H
#define RENDERER_H

#include <platform/platform.h>

#include <stdint.h>

typedef struct {
	char* app_name;
	uint32_t api_backend;
	platform_context_t* platform_context;
} renderer_context_create_info_t;

typedef struct renderer_context_t renderer_context_t;
typedef struct render_target_t render_target_t;

#define RENDERER_API_BACKEND_VULKAN  1
#define RENDERER_API_BACKEND_DIRECTX 2

renderer_context_t* renderer_create_context(const renderer_context_create_info_t create_info);
void renderer_destroy_context(renderer_context_t* context);

render_target_t* renderer_create_render_target(renderer_context_t* context, platform_window_t* platform_window);
void renderer_destroy_render_target(renderer_context_t* context, render_target_t* target);



#endif // RENDERER_H