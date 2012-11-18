#include "stringGarbage.h"

//another ugly solution: this list is shared via linking process
struct stringGarbage* lexerStringBin = NULL; 

void initStringGarbageCollector(){

  lexerStringBin = malloc(sizeof(struct stringGarbage));  
  (*lexerStringBin).string = NULL;
  (*lexerStringBin).previous = NULL;

}

char* strdupgar(const char* str){

  char* strCp = strdup(str);
  (*lexerStringBin).string = strCp;

  struct stringGarbage* newNode = malloc(sizeof(struct stringGarbage));
  (*newNode).string = NULL;
  (*newNode).previous = lexerStringBin;
  lexerStringBin = newNode;

  return strCp;

}

void cleanStringGarbageBin(){
  
  struct stringGarbage* currNode = lexerStringBin;
  struct stringGarbage* helperNode = NULL;

  while(currNode != NULL){
    if((*currNode).string != NULL) free((*currNode).string);
    helperNode = currNode;
    currNode = (*currNode).previous;
    free(helperNode);
  }

}
