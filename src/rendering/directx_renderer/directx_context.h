#ifndef DIRECTX_CONTEXT_H
#define DIRECTX_CONTEXT_H

#ifdef _WIN32

#define DIRECTX_SUPPORTED 1

typedef struct {
	
} directx_context_t;

#else

#define DIRECTX_SUPPORTED 0

typedef struct {
	
} directx_context_t;

#endif

#endif // DIRECTX_CONTEXT_H