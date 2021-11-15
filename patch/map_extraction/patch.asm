
.open "sys/main.dol"

.org @NextFreeSpace
.include "src/map_extraction.c"

.org 0x80012738
  bl InitMap_External
  
.org 0x8000BF50
  bl FreeMap_External
  
.close
