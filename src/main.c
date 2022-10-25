#include <platform/platform.h>
#include <stdlib.h>

#include "rendering/renderer.h"

int main(void) {
	platform_context_t* platform_context = platform_create_context(NULL, NULL);
	if(platform_context == NULL) return -1;

	renderer_context_create_info_t create_info;
	create_info.app_name = "Vulkan Renderer";
	create_info.api_backend = RENDERER_API_BACKEND_VULKAN;
	create_info.platform_context = platform_context;

	renderer_context_t* renderer_context = renderer_create_context(create_info);
	if(renderer_context == NULL) {
		platform_destroy_context(platform_context, NULL);
		return -1;
	}

	platform_window_create_info_t window_create_info = {0};
	window_create_info.name = "Cat Window";
	window_create_info.x = 100;
	window_create_info.y = 100;
	window_create_info.width = 500;
	window_create_info.height = 300;
	window_create_info.parent = NULL;
	window_create_info.flags = PLATFORM_WF_SPLASH;
	platform_window_t* window = platform_create_window(platform_context, window_create_info, NULL);


	while(!platform_window_should_close(platform_context, window)) {
		platform_handle_events(platform_context);
		platform_sleep_miliseconds(32);
	}

	platform_destroy_window(platform_context, window, NULL);
	renderer_destroy_context(renderer_context);
	platform_destroy_context(platform_context, NULL);
	return 0;
}