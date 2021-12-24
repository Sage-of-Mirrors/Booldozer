#include "js_events.h"

// Global reference to our duktape context.
duk_context* DukGlobalContext = NULL;

static duk_ret_t OSReportJS(duk_context* ctx){
  OSReport("%s\n", duk_to_string(ctx, 0));
  return 0;
}

// Initializes duktape's context.
void init_duktape()
{
  DukGlobalContext = NULL;
  DukGlobalContext = duk_create_heap(JKRHeap_alloc, JKRHeap_resize, JKRHeap_free, (void*)0xDEADBEEF, &fatal_error);
  
  if (DukGlobalContext == NULL)
    fatal_error(NULL, "Duktape global context was not initialized!");
  else
    OSReport("Duktape global context intialized at address 0x%X", DukGlobalContext);
  
  FUN_80006204();
}

// Ticks executing events every frame.
void tick_duktape()
{
  
}

// Deletes duktape's context.
void free_duktape()
{
  //if (DukGlobalContext != NULL)
    //duk_destroy_heap(DukGlobalContext);
}

// Handles fatal errors in duktape's execution.
void fatal_error(void *udata, const char *msg)
{
  OSReport("JS Fatal Error! Message: %s\n", msg != NULL ? msg : "(no message given)");
  //abort();
}
