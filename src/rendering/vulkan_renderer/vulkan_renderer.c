#include "vulkan_renderer.h"
#include "vulkan_internal.h"

#ifdef NDEBUG
	const uint32_t layer_count = 0;
	const char* layer_names[] = { NULL };
#else
	const uint32_t layer_count = 1;
	const char* layer_names[] = {
		"VK_LAYER_KHRONOS_validation"
	};
#endif // NDEBUG

const uint32_t device_extension_count = 1;
const char* device_extension_names[] = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};


int8_t vulkan_init_context(vulkan_context_t* context, platform_context_t* platform_context, const char* app_name) {
	VkApplicationInfo app_info = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
	app_info.pApplicationName = app_name;
	app_info.pEngineName = "Vulkan Render Engine";
	app_info.apiVersion = VK_API_VERSION_1_1;

	uint32_t extension_count = 0;
	const char** extension_names = (const char**)platform_vulkan_required_extensions(platform_context, &extension_count);

	VkInstanceCreateInfo instance_create_info = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
	instance_create_info.pApplicationInfo = &app_info;
	instance_create_info.enabledLayerCount = layer_count;
	instance_create_info.ppEnabledLayerNames = layer_names;
	instance_create_info.enabledExtensionCount = extension_count;
	instance_create_info.ppEnabledExtensionNames = extension_names;
	VkResult instance_result = vkCreateInstance(&instance_create_info, NULL, &context->instance);
	if(instance_result != VK_SUCCESS) return 0;

	if(!_vulkan_init_physical_devices(context)) {
		vkDestroyInstance(context->instance, NULL);
		return 0;
	}
	if(!_vulkan_select_physical_device(context)) {
		vkDestroyInstance(context->instance, NULL);
		return 0;
	}
	if(!_vulkan_init_selected_device(context)) {
		vkDestroyInstance(context->instance, NULL);
		return 0;
	}

	return 1;
}
void vulkan_cleanup_context(vulkan_context_t* context) {
	_vulkan_cleanup_physical_device(context);
	_vulkan_cleanup_selected_device(context);
	vkDestroyInstance(context->instance, NULL);
	context->instance = VK_NULL_HANDLE;
}

render_target_t* vulkan_create_render_target(renderer_context_t* context, platform_window_t* platform_window) {
	return NULL;
}

void vulkan_destroy_render_target(renderer_context_t* context, render_target_t* target) {

}