#include "renderer_internal.h"
#include "vulkan_renderer/vulkan_renderer.h"
#include <malloc.h>

renderer_context_t* renderer_create_context(const renderer_context_create_info_t create_info) {
	renderer_context_t* context = malloc(sizeof(renderer_context_t));
	context->platform_context = create_info.platform_context;
	switch (create_info.api_backend)
	{
	case RENDERER_API_BACKEND_VULKAN:
		context->api_backend = RENDERER_API_BACKEND_VULKAN;
		context->functions = VULKAN_RENDERER_FUNCTIONS;
		if(!vulkan_init_context(&context->vulkan, create_info.platform_context, create_info.app_name)) return NULL;
		break;
	case RENDERER_API_BACKEND_DIRECTX:
		#if DIRECTX_SUPPORTED
		context->api_backend = RENDERER_API_BACKEND_DIRECTX;
		#else
		free(context);
		return NULL;
		#endif // DIRECTX_SUPPORTED
	default:
		free(context);
		return NULL;
	}

	return context;
}
void renderer_destroy_context(renderer_context_t* context) {
	free(context);
}

render_target_t* renderer_create_render_target(renderer_context_t* context, platform_window_t* platform_window) {
	return context->functions.create_render_target(context, platform_window);
}

void renderer_destroy_render_target(renderer_context_t* context, render_target_t* target) {
	context->functions.destroy_render_target(context, target);
}

int8_t renderer_recreate_render_target(renderer_context_t* context, render_target_t* target) {
	return context->functions.recreate_render_target(context, target);
}

void draw_triangle(renderer_context_t* context, render_target_t* target) {
	context->functions.draw_triangle(context, target);
}