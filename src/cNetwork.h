#ifndef CNETWORK_H
#define CNETWORK_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//maximal number of positions for NUMBERs 
#define MAXNUM 100
//used when linear expressions are built
#define MAXLINS 255
//maximal number of clocks
#define MAXCLOCKS 100
//maximal number of parameters
#define MAXPARAMS 100

/**************************************************************
 * Contains the formula tree of a linear expression or reset. *
 **************************************************************/
struct fTree {
  char* oper; //logical or arithmetic operator
  struct fTree* leftNode;
  struct fTree* rightNode;
};

// Recursively frees a fTree.                                
void freefTree(struct fTree* ftree);

/**************************************************************
 * List of locations, where invariant is in form of fTree.    *
 * This is bidirectional list of locations for current autom. *
 **************************************************************/
struct cLocation {
  char* locationName;
  int isInitial;
  struct fTree* labelling;
  struct fTree* invariant;
  struct cLocation* previous; 
  struct cLocation* next;
};

// Recursively frees a cLocation.                             
void freecLocation(struct cLocation* clocation);

/**************************************************************
 * List of transitions with target, action and source names,  *
 * guard and reset (fTrees).                                  *
 * This is bidirectional list of locations for current autom. *
 **************************************************************/
struct cTransition {
  char* source;
  char* target;
  char* actionName;
  struct fTree* guard;  
  struct fTree* reset;  
  struct cTransition* previous; 
  struct cTransition* next;
};

// Recursively frees a cTransition.                           
void freecTransition(struct cTransition* ctransition);

/**************************************************************
 * Bidirectional list of automata.                            *
 **************************************************************/
struct cAutomaton {
  char* autoName;
  struct cLocation* locations; //recall that this is a queue too
  struct cTransition* transitions; //recall that this is a queue too
  struct cAutomaton* previous;
  struct cAutomaton* next;

};

// Recursively frees a cAutomaton.                            
void freecAutomaton(struct cAutomaton* cautomaton);

/**************************************************************
 * Contains the parser model.                                 *
 **************************************************************/
struct cNetwork {
  char* netName;

  int numberOfClocks;
  const char* clockNames[MAXCLOCKS];

  int numberOfParameters;
  const char* paraNames[MAXPARAMS];

  struct cAutomaton* automata; //this is queue of automata

  struct fTree* property;

};

// Frees network's automata.                                   
void cleanNetwork(struct cNetwork* cnetwork);

// Puts parameter (if new) in net, and increments param. ctr. 
void putParam(struct cNetwork* net, const char* param);

// Puts clock (if new) in net, and increments clocks ctr.    
void putClock(struct cNetwork* net, const char* clock);

// Pumps spaceNo spaces, ugh.                                 
void pumpSpaces(int spaceNo);

// Displays the formula from fTree.
void displayFormulaTree(struct fTree* top, int depth);

#endif
