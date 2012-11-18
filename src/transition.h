#ifndef TRANSITION_H
#define TRANSITION_H

#include <string>
#include <vector>
#include <ostream>
#include <sstream>
#include <map>
#include <set>
#include "assert.h"
#include "tools.h"
#include "globalState.h"

using namespace std;

/**************************************************************
 * Encapsulates transition, together with all guards, resets  *
 * and invariants in smtlib form.                             *
 **************************************************************/
class Transition {

  friend ostream& operator<<(ostream& output, const Transition& tran);

  // Performs multiplication (see operator*(const Transition& rightTran)) between each Transition from
  // left and each Transition from right. Returns vector of such results.
  friend vector<Transition> operator*(const vector<Transition> left, const vector<Transition> right);
  
 public:

  globalState source; 
  globalState target; 

  string sourceInvariant;

  string actionName;

  string targetInvariant;

  string guardExpression,
         resetExpression;

  // Goes through all transitions in input vector, and builds a set of target globalStates.
  static set<globalState> getTargetGlobalStates(const vector<Transition>& transitions);

  // This is variable used to represent time steps.
  static string timedVar;

  // Parameterless constructor.
  Transition(){}

  // Builds Transition out of cTransition, i.e. moves us from C to C++. This is the main version.
  Transition(const struct cAutomaton* autom, const struct cTransition* trans, map<string, string>& locationsMap);

  // Mostly concatenations (rpn-smtlib conforming) of fields, see the source.
  Transition operator*(const Transition& rightTran);

  // Raw-encodes source and target (see globalState.h).
  void rawencode();

  // Encodes the transition, with appropriate label substitutions. 
  string tr_enc(int i);
  
  // Links transitions with underlying model structures. 
  static void init(vector<string> *clockNames);

 private:

  // Clocks' names: set by modelCrawler.
  static vector<string> *clockNamesPtr;

};

#endif
