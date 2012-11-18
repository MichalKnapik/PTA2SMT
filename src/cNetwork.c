#include "cNetwork.h"
#include "stringGarbage.h"

struct cNetwork myNet; //this is a shared object containing the result of parsing

void putParam(struct cNetwork* net, const char* param){

  int i = 0; 
  for(i = 0; i < (*net).numberOfParameters; ++i){
    if(strcmp((*net).paraNames[i], param) == 0) return;
  }

  if((*net).numberOfParameters >= MAXPARAMS) abort();
  (*net).paraNames[((*net).numberOfParameters)++] = param;

}

void putClock(struct cNetwork* net, const char* clock){

  int i = 0; 
  for(i = 0; i < (*net).numberOfClocks; ++i){
    if(strcmp((*net).clockNames[i], clock) == 0) return;
  }

  if((*net).numberOfClocks >= MAXCLOCKS) abort();
  (*net).clockNames[((*net).numberOfClocks)++] = clock;

}

void pumpSpaces(int spaceNo){

  int i = 0;
  for(i = 0; i<spaceNo; ++i){
    printf(" ");
  }

}

void displayFormulaTree(struct fTree* top, int depth){ 

  ++depth;
  if(top != NULL) {
    displayFormulaTree((*top).leftNode, depth);
    pumpSpaces(depth);
    printf("%s\n",(*top).oper);
    displayFormulaTree((*top).rightNode, depth);
  }

}

void freefTree(struct fTree* ftree){

  if(ftree == NULL) return;
  free((*ftree).oper);
  freefTree((*ftree).leftNode);
  freefTree((*ftree).rightNode);

  free(ftree);

}

void freecLocation(struct cLocation* clocation){

  if(clocation == NULL) return;
  free((*clocation).locationName);
  freefTree((*clocation).labelling);
  freefTree((*clocation).invariant);
  freecLocation((*clocation).next);

  free(clocation);

}

void freecTransition(struct cTransition* ctransition){

  if(ctransition == NULL) return;
  free((*ctransition).source);
  free((*ctransition).target);
  free((*ctransition).actionName);
  freefTree((*ctransition).guard);
  freefTree((*ctransition).reset);
  freecTransition((*ctransition).next);

  free(ctransition);

}

void freecAutomaton(struct cAutomaton* cautomaton){

  if(cautomaton == NULL) return;
  freecLocation((*cautomaton).locations);
  freecTransition((*cautomaton).transitions);
  freecAutomaton((*cautomaton).next);

  free(cautomaton);

}

void cleanNetwork(struct cNetwork* cnetwork){

  freecAutomaton((*cnetwork).automata);
  freefTree((*cnetwork).property);
  cleanStringGarbageBin();

}
