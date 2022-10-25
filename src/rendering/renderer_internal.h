#ifndef RENDERER_INTERNAL_H
#define RENDERER_INTERNAL_H

#include "renderer.h"
#include "vulkan_renderer/vulkan_context.h"
#include "directx_renderer/directx_context.h"

typedef struct {
	render_target_t* (*create_render_target)(renderer_context_t* context, platform_window_t* platform_window);
	void (*destroy_render_target)(renderer_context_t* context, render_target_t* target);
} renderer_functions_t;

struct renderer_context_t {
	uint32_t api_backend;
	renderer_functions_t functions;
	union {
		vulkan_context_t vulkan;
		directx_context_t directx;
	};
};

#endif // RENDERER_INTERNAL_H