#ifndef MODELCRAWLER_H
#define MODELCRAWLER_H

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <sstream>
#include <map>
#include <set>
#include <iterator>
#include <vector>
#include <algorithm>
#include "timedAutomataParser.tab.h"
#include "externs.h"
#include "transition.h"
#include "globalState.h"
#include "modalProperty.h"
#include "cNetwork.h"
#include "tools.h"

using namespace std;

/***************************************************************
 * This is the main class. It provides aims for model reading, *
 * storage, encoding, etc.                                     *
 *-------------------------------------------------------------*
 * FUTURE VERSION:                                             *
 * ~ I've chosen a stupid method to encode global states, i.e. *
 *   all states (not only the reachable ones) are a part of    *
 *   the enumeration by integers. Change this so that raw      *
 *   encodings are assigned to reachable states only.          *
 *   Possible gain: substantial decrease in number of vars!    *
 ***************************************************************/
class modelCrawler {

 public:

  // Names of clocks in model specification.
  vector<string> clockNames;

  // Names of parameters in model specification.
  vector<string> parameterNames;

  // Map from names of automata to its initial states.
  globalState initialState;

  // The property to be checked.
  modalProperty property;

  // Map from automata name to map from location to invariant in smtlib,
  // i.e. invariantMap[automaton][location] is an invariant in 
  // "location" of "automaton".
  map< string, map<string, string> > invariantMap;

  // Map from automata to map from its locations to corresponding numbers.
  // May seem redundant, but speeds up globalstates encoding.
  map<string, map<string, int> > locationsMap;

  // Map from automata to map from location's name to the set of
  // the propositions.
  map<string, map<string, set<string> > > propositionsMap;

  // Map from automata name to map from location to outgoing transitions,
  // i.e. transitionMap[automaton][location] is a vector of all Transitions
  // leaving "location" in "automaton".
  map< string, map<string, vector<Transition> > > transitionMap;

  // Map from automaton name to the set of names of actions present in the automaton.
  map<string, set<string> > knownActions;

  // All known actions, automata omitted.
  set<string> allKnownActions;

  // Returns all global transitions starting in a set of input states.
  vector<Transition> outgoingTransitions(set<globalState>& states); 

  // Constructors and destructors.                              
  modelCrawler();
  ~modelCrawler();

  // Read model from a file and attempt to parse it. 
  // Returns 0 if all is ok, 1 for I/O error, and 2 for syntax error. 
  int readFile(const char* fName);
  
  // Initializes internal structures of modelCrawler. The idea is
  // to move from the results of C/Bison/Flex parsing to C++.
  void initModel();

  // Builds property.
  void buildProperty(struct fTree* tPtr);

  // Displays the network (C++ maps).
  void displayModel();

  // Returns the encoding of the unwinding of model's computation tree;
  // i-th consecutive call returns unwinding up to depth i + 1.
  // The formula after i-th call is not complete yet. To complete it
  // with a tail of form "true ))...)) true)" concatenate it with
  // unwindingTail(number of calls to encodeUnwinding).
  stringstream& encodeUnwinding();

  // Returns tail of formula encoding the unwinding of the model (see encodeUnwinding).
  string unwindingTail(int unwindingCalls);

  // Returns smtlib declarations preamble.
  string makePreamble(int maxClockVarno, string logicName);

  // Tests if given globalState satisfied the property.
  bool satisfiesProperty(const globalState& state);

  // Selects from instates set of globalStates only those for which the 
  // property holds, and encodes them using the same variables as
  // in last call of encodeUnwinding.
  string encodeProperty();
  
 private:

  // This is incremented in each unwinding.
  int stepctr;

  // Starting states of (stepctr+1)-th unwinding.
  set<globalState> instates;

  // Place to hold the computed transitions of (stepctr+1)-th unwinding.
  vector<Transition> nxt;

  // Builds map from automata name to map from location to invariant in smtlib.
  void mapAutomata2Locations2Invariants(struct cAutomaton* autom);

  // Builds map from automata name to map from location to invariant in smtlib.
  // (Parameterless version)
  void mapAutomata2Locations2Invariants();

  // Builds map from automata name to map from location to outgoing transitions 
  // in smtlib.
  void mapAutomata2Locations2Transitions(struct cAutomaton* autom);

  // Builds map from automata name to map from location to outgoing transitions 
  // in smtlib. (Parameterless version)
  void mapAutomata2Locations2Transitions();

  // Returns a map from locations to invariants in given automaton.
  map<string, string> mapLocations2Invariants(struct cAutomaton* autom);

  // Returns a map from locations to outgoing Transitions in given automaton.
  map<string, vector<Transition> > mapLocations2Transitions(struct cAutomaton* autom);

  // Takes globalTransSet which is a map from automata names to vectors of transitions
  // representing local transitions from given local states
  // and produces corresponding (product) global transitions.
  // (It actually assumes that all input transitions have the same action label).
  vector<Transition> localTransitions2GlobalTransitions(const map< string, vector<Transition> > &globalTransSet);

  // For debug purposes - presents network's components (underlying C structs).     
  void displayNetwork(struct cNetwork* net = &myNet);

  // For debug purposes: presents single automaton from cAutomaton (underlying C structs).
  void displayAutomaton(struct cAutomaton* automaton);
  
};

#endif
