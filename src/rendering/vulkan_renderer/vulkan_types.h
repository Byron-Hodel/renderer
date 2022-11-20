#ifndef VULKAN_TYPES_H
#define VULKAN_TYPES_H

#define PLATFORM_VULKAN
#include <platform/platform.h>
#include "rendering/renderer.h"
#include <vulkan/vulkan.h>
#include "math.h"

#define GRAPHICS_QUEUE_INDEX    0
#define COMPUTE_QUEUE_INDEX     1
#define TRANSFER_QUEUE_INDEX    2
#define SPARSE_BIND_QUEUE_INDEX 3

#define COMMAND_BUFFER_STATE_NOT_ALLOCATED 0
#define COMMAND_BUFFER_STATE_READY         1
#define COMMAND_BUFFER_STATE_RECORDING     2
#define COMMAND_BUFFER_STATE_RECORDING_END 3
#define COMMAND_BUFFER_STATE_SUBMITTED     4
#define COMMAND_BUFFER_STATE_INVALID       5

typedef struct vulkan_device_t {
	uint32_t      device_index;
	VkDevice      handle;
	int32_t       queue_family_indices[4];
	VkQueue       queues[4];
	VkCommandPool graphics_cmd_pool;
} vulkan_device_t;

typedef struct vulkan_physical_devices_t {
	uint32_t          count;
	VkPhysicalDevice* handles;
	uint8_t*          supported;
} vulkan_physical_devices_t;

typedef struct vulkan_renderpass_t {
	VkRenderPass handle;
	vec2_t       pos;
	vec2_t       extents;
	float        depth;
	uint32_t     stencil;
} vulkan_renderpass_t;

typedef struct vulkan_graphics_pipeline_t {
	VkPipeline       handle;
	VkShaderModule   vert_shader_module;
	VkShaderModule   frag_shader_module;
	VkPipelineLayout layout;
} vulkan_graphics_pipeline_t;

typedef struct {
	VkCommandBuffer handle;
	uint32_t        state;
} vulkan_command_buffer_t;

typedef struct vulkan_image_t {
	VkImage        handle;
	VkImageView    view;
	VkDeviceMemory device_memory_handle;
} vulkan_image_t;

typedef struct vulkan_framebuffer_t {
	VkFramebuffer        handle;
	uint32_t             width;
	uint32_t             height;
	uint32_t             attachment_count;
	VkImageView*         attachments;
	vulkan_renderpass_t* renderpass;
} vulkan_framebuffer_t;

typedef struct vulkan_fence_t {
	VkFence handle;
	int8_t  signaled;
} vulkan_fence_t;

typedef struct vulkan_swapchain_t {
	VkSwapchainKHR           handle;
	VkSurfaceKHR             surface;
	VkSurfaceCapabilitiesKHR surface_capabilities;
	VkExtent2D               extent;
	VkColorSpaceKHR          color_space;
	VkPresentModeKHR         present_mode;
	VkFormat                 format;
	VkFormat                 depth_format;
	uint32_t                 image_count;
	VkImage*                 images;
	VkImageView*             image_views;
	vulkan_image_t*          depth_images;
	vulkan_framebuffer_t*    default_renderpass_frame_buffers;
	platform_window_t*       window;
} vulkan_swapchain_t;


typedef struct vulkan_window_render_target_t {
	vulkan_swapchain_t      swapchain;
	vulkan_command_buffer_t command_buffer;
	VkSemaphore             image_available_semaphore;
	VkSemaphore             render_complete_semaphore;
	vulkan_fence_t          in_flight_fence;
	uint32_t                image_index;
} vulkan_window_render_target_t;

typedef struct vulkan_context_t {
	VkInstance                  instance;
	uint32_t                    layer_count;
	char**                      layer_names;
	uint32_t                    device_extension_count;
	char**                      device_extension_names;
	vulkan_physical_devices_t   physical_devices;
	vulkan_device_t             selected_device;
	vulkan_renderpass_t         default_renderpass; // 3d renderpass
	vulkan_renderpass_t         ui_renderpass; // ui and 2d renderpass
	
} vulkan_context_t;


#endif // VULKAN_TYPES_H