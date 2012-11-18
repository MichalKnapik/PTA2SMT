#ifndef STRINGGARBAGE_H
#define STRINGGARBAGE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*****************************************
 * Poor man's string garbage collector.  *
 *****************************************/
struct stringGarbage {
  char* string;
  struct stringGarbage* previous;
};

// This is a garbage collector for lexed strings.
extern struct stringGarbage* lexerStringBin; 

// Garbage collector initialization.
void initStringGarbageCollector();

// Makes safe strdup, collecting the pointer for later deallocation.
char* strdupgar(const char* str);

// Frees memory allocated using strdupgar.
void cleanStringGarbageBin();

#endif
