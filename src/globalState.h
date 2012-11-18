#ifndef GLOBALSTATE_H
#define GLOBALSTATE_H

#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include <set>
#include <vector>
#include <cmath>
#include "assert.h"
#include "tools.h"

using namespace std;

/**************************************************************
 * Encapsulates global states, i.e. tuples of local states.   *
 **************************************************************/
class globalState {

  friend ostream& operator<<(ostream& output, const globalState& gst);
  
 public:
  
  // Constructor.
  globalState() { 
    integerEncoding = -1;
  }

  // Map from automaton name to local state.
  map<string, string> localStates;

  // Needed for stl::set comparisons. Rather inefficient.
  bool operator<(const globalState& right) const;

  // This is a raw value of byte encoding of instance globalState
  // represented as integer. If equal to -1, then the instance hasn't been encoded yet.
  BIGINT integerEncoding;

  // The number of bits used in integerEncoding.
  static int bitsNo;

  // Computes (if needed) or returns integerEncoding.
  BIGINT rawencoding();

  // Returns a smtlib-rpn string of bools representing integerEncoding. 
  // Uses encodingVarPrefix with concatenated incremented startNo.
  string cvcEncode(int startNo);

  // Used as a prefix in state encoding.
  static string encodingVarPrefix;

  // Maximal index of used boolean encoding variable.
  static int maxBoolVarNr;

  // Initialises locationsMap and encodingBasis using locsMap (locationsMap from modelCrawler).
  static void init(map<string, map<string, int> >* locsMap);

  private:

  // Map from automata to map from its locations to corresponding numbers
  // (set by modelCrawler).
  static map<string, map<string, int> >* locationsMap;

  // As used in lexicogrValue, computed from locationsMap.
  static vector<int> encodingBasis;

};

#endif
