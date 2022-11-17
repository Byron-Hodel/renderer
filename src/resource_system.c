#include "resource_system.h"
#include <malloc.h>
#include <string.h>

#define STARTING_TABLE_SIZE 128

typedef struct resource_t {
	resource_type_t type;
	uint32_t        id;
	char*           actual_path;
	uint32_t        actual_path_len;
	uint64_t        offset;
	uint64_t        size;
	void*           address;
} resource_t;

typedef struct resource_node_t {
	struct resource_node_t* next;
	char*                   resource_path;
	uint32_t                resource_path_len;
	uint32_t                resource_id;
} resource_node_t;


static uint32_t resource_count;
static uint32_t resource_table_size;
static resource_node_t** resource_table;

static uint32_t resources_capacity;
static resource_t* resources;

static uint32_t hash_resource_path(const char* resource_path, const uint32_t length);
static int8_t load_text_resource(const uint32_t resource_id);
static int8_t load_shader_resource(const uint32_t resource_id);

int8_t resource_system_init(void) {
	resource_count = 0;
	resource_table_size = STARTING_TABLE_SIZE;
	resource_table = calloc(STARTING_TABLE_SIZE, sizeof(resource_node_t));
	if(resource_table == NULL) return 0;
	return 1;
}
void resource_system_shutdown(void) {
	for(uint32_t i = 0; i < resource_table_size; i++) {
		resource_node_t* next = resource_table[i];
		while(next != NULL) {
			resource_node_t* current = next;
			next = next->next;
			free(current);
		}
	}
	resource_count = 0;
	resource_table_size = 0;
	for(uint32_t i = 0; i < resource_count; i++) {
		free(resources->actual_path);
	}
	free(resources);
}

uint32_t resource_system_register_resource(const char* resource_path, const char* actual_path,
                                           const uint64_t size, const uint64_t offset,
                                           const resource_type_t type)
{
	uint32_t resource_path_len = strlen(resource_path);
	uint32_t actual_path_len = strlen(actual_path);
	uint32_t table_index = hash_resource_path(resource_path, resource_path_len);
	resource_node_t* node = resource_table[table_index];
	while(node->next != NULL) {
		if(node->resource_path_len != resource_path_len) continue;
		if(strncmp(resource_path, node->resource_path, resource_path_len) == 0) return 0;
		node = node->next;
	}
	
	if(resource_count == resources_capacity) {
		resource_t* new_resources = realloc(resources, sizeof(resource_t) * (resources_capacity << 1));
		if(new_resources == NULL) return 0;
		resources_capacity <<= 1;
	}

	resource_t* r = resources + (resource_count++);
	r->type = type;
	r->id = resource_count - 1;
	r->actual_path = malloc(actual_path_len+1);
	strncpy(r->actual_path, actual_path, actual_path_len);
	r->actual_path_len = actual_path_len;
	r->offset = offset;
	r->size = size;
	r->address = NULL;
	return r->id;
}

uint32_t resource_system_lookup_id(const char* resource_path) {
	uint32_t resource_path_len = strlen(resource_path);
	uint32_t table_index = hash_resource_path(resource_path, resource_path_len);
	resource_node_t* node = resource_table[table_index];
	while(node->next != NULL) {
		if(node->resource_path_len != resource_path_len) continue;
		if(strncmp(resource_path, node->resource_path, resource_path_len) == 0) return node->resource_id;
		node = node->next;
	}
	return RESOURCE_INVALID_ID;
}

int8_t resource_system_load(const uint32_t resource_id) {
	
	return 0;
}
void resource_system_unload(const uint32_t resource_id) {
	
}
resource_data_t* resource_system_get_data(const uint32_t resource_id) {
	return NULL;
}

static uint32_t hash_resource_path(const char* resource_path, const uint32_t length) {
	uint32_t hash = 0;
	// TODO: improve later
	for(uint32_t i = 0; i < length; i++) {
		hash += resource_path[i];
	}
	return hash % resource_table_size;
}

static int8_t load_text_resource(const uint32_t resource_id) {
	/*
	char path_buffer[1024];
	string_t full_path;
	full_path.capacity = 1023;
	full_path.len = 0;
	full_path.c_str = path_buffer;

	string_cat(&full_path, registry.base_resource_path);
	if(full_path.c_str[full_path.len-1] != '/' && full_path.c_str[full_path.len-1] != '\\') {
		string_cat(&full_path, string_get("/"));
	}
	string_cat(&full_path, registry.paths[resource_id]);


	uint64_t file_size = registry.sizes[resource_id];
	resource_data_t* resource_data = calloc(1, sizeof(text_resource_data_t) + file_size);
	if(resource_data == NULL) return 0;

	resource_data->text_data.text_len = file_size;

	FILE* file = fopen(full_path.c_str, "r");
	if(file == NULL) return 0;

	uint32_t bytes_read = 0;
	while(bytes_read != file_size) {
		bytes_read += fread(resource_data->text_data.text, sizeof(char), file_size - bytes_read, file);
	}

	fclose(file);

	registry.addresses[resource_id] = resource_data;
	return 1;
	*/
	return 0;
}

static int8_t load_shader_resource(const uint32_t resource_id) {
	return 0;
}