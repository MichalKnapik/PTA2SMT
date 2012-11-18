#ifndef TOOLS_H
#define TOOLS_H

#include <stdio.h>
#include <string>
#include <iostream>
#include <map>
#include <iterator>
#include <vector>
#include <assert.h>
#include "cNetwork.h"

#define BIGINT long long 

using namespace std;

// Transforms infix linear or reset expression to smtlib rpn. 
// The returned pointer needs to be freed later (strdup use). 
string formulaTree2Cvc(struct fTree* top);

// Substitute in expression string clocks from clockNames so that
// newClockNames[i] goes for clockNames[i] for all i.
string substitute(const string& inexpression, vector<string> &newClockNames, vector<string>& clockNames);

// Debug help: display map<string, string>.
void dismap(map<string, string> m);

// Debug help: display map< string, map<string, string> >.
void dismapmap(map< string, map<string, string> > mm);

// Encodes int vectors as ordinals, using lexicographical order using provided vector of max sizes
// as a basis. E.g. [1,2,3] -> 17 under basis [3,4,4] (max. 3 on pos. 1, max 4 on pos. 2, etc.).
// Works in linear time w.r.t. size.
BIGINT lexicogrValue(vector<int>& inputArray, vector<int>& maxSizesBasis);

// Returns bool vector bin value of the number.
vector<bool> int2binvect(BIGINT number, int usedBits);

#endif
