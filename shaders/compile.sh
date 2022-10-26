#!/bin/bash

$VULKAN_SDK/bin/glslc shaders/shader.vert -o vert.spv
$VULKAN_SDK/bin/glslc shaders/shader.frag -o frag.spv