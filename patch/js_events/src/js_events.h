#pragma once
#include "duk_config.h"
#include "duktape.h"

// Global reference to our duktape context.
extern duk_context* DukGlobalContext;

// Reference to the memory allocation function that duktape will use
extern void* JKRHeap_alloc;
// Reference to the memory resize function that duktape will use
extern void* JKRHeap_resize;
// Reference to the memory free function that duktape will use
extern void* JKRHeap_free;

// Initializes duktape's context.
void init_duktape();
// Ticks executing events every frame.
void tick_duktape();
// Deletes duktape's context.
void free_duktape();
// Handles fatal errors in duktape's execution.
void fatal_error(void *udata, const char *msg);
