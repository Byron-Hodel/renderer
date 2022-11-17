#include "vulkan_command_buffer.h"

int8_t vulkan_command_buffer_alloc(const vulkan_context_t context, vulkan_command_buffer_t* buffer,
                                   VkCommandPool pool, const int8_t is_primary)
{
	VkCommandBufferAllocateInfo alloc_info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
	alloc_info.commandPool = pool;
	alloc_info.level = is_primary ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
	alloc_info.commandBufferCount = 1;

	//vkAllocateCommandBuffers();
	buffer->state = COMMAND_BUFFER_STATE_NOT_ALLOCATED;
	VkResult r = vkAllocateCommandBuffers(context.selected_device.handle, &alloc_info, &buffer->handle);
	if(r != VK_SUCCESS) {
		buffer->handle = VK_NULL_HANDLE;
		return 0;
	}
	buffer->state = COMMAND_BUFFER_STATE_READY;
	return 1;
}

void vulkan_command_buffer_free(const vulkan_context_t context, vulkan_command_buffer_t* buffer, VkCommandPool pool) {
	vkFreeCommandBuffers(context.selected_device.handle, pool, 1, &buffer->handle);
	buffer->state = COMMAND_BUFFER_STATE_NOT_ALLOCATED;
}

void vulkan_command_buffer_begin_recording(vulkan_command_buffer_t* buffer, int8_t single_use,
                                           int8_t renderpass_continue, int8_t simultaneous_use)
{
	VkCommandBufferBeginInfo begin_info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
	if(single_use)          begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	if(renderpass_continue) begin_info.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
	if(simultaneous_use)    begin_info.flags |= VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	VkResult r = vkBeginCommandBuffer(buffer->handle, &begin_info);
	buffer->state = COMMAND_BUFFER_STATE_RECORDING;
}

void vulkan_command_buffer_end_recording(vulkan_command_buffer_t* buffer) {
	VkResult r = vkEndCommandBuffer(buffer->handle);
	if(r != VK_SUCCESS) {
		//
	}
	buffer->state = COMMAND_BUFFER_STATE_RECORDING_END;
}

void vulkan_command_buffer_submit(vulkan_command_buffer_t* buffer) {
	buffer->state = COMMAND_BUFFER_STATE_SUBMITTED;
}

void vulkan_command_buffer_reset(vulkan_command_buffer_t* buffer) {
	vkResetCommandBuffer(buffer->handle, 0);
	buffer->state = COMMAND_BUFFER_STATE_READY;
}