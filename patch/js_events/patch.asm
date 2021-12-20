
.open "sys/main.dol"

.org @NextFreeSpace
.include "src/js_events.c"

.org 0x80006010
  lis r3,0x4b
.org 0x8000581c
  bl init_duktape
  
.close
