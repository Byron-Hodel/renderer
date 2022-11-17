#ifndef RESOURCES_H
#define RESOURCES_H

#include <stdint.h>

#define RESOURCE_INVALID_ID (~(uint32_t)0)

typedef enum {
	RESOURCE_TYPE_TEXT,
	RESOURCE_TYPE_SHADER,
	RESOURCE_TYPE_COUNT
} resource_type_t;

typedef struct {
	uint64_t text_len;
	char text[];
} text_resource_data_t;

typedef struct {
	uint64_t vert_len;
	uint64_t frag_len;
	uint8_t* vert_src;
	uint8_t* frag_src;
} shader_resource_data_t;

typedef union {
	text_resource_data_t text_data;
	shader_resource_data_t shader_data;
} resource_data_t;

int8_t resource_system_init(void);
void resource_system_shutdown(void);

uint32_t resource_system_register_resource(const char* resource_path, const char* actual_path,
                                           const uint64_t size, const uint64_t offset,
                                           const resource_type_t type);

uint32_t resource_system_lookup_id(const char* resource_path);

int8_t resource_system_load(const uint32_t resource_id);
void resource_system_unload(const uint32_t resource_id);
resource_data_t* resource_system_get_data(const uint32_t resource_id);

#endif // RESOURCES_H