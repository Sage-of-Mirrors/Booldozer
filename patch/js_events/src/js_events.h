#pragma once
#include "duk_config.h"
#include "duktape.h"

// Global reference to our duktape context.
extern duk_context* DukGlobalContext;

// Initializes duktape's context.
void init_duktape();
// Ticks executing events every frame.
void tick_duktape();
// Deletes duktape's context.
void free_duktape();
// Handles fatal errors in duktape's execution.
void fatal_error(void *udata, const char *msg);

void* alloc(void* heap_udata, size_t size);
void* rrealloc(void* heap_udata, size_t size);
void ffree(void* ptr, void* heap_udata);
