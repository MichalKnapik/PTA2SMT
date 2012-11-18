#ifndef MODALPROPERTY_H
#define MODALPROPERTY_H

#include <string>
#include <set>
#include "cNetwork.h"

using namespace std;

/**************************************************************
 * Modal property.                                            *
 **************************************************************/
class modalProperty {
  
 public:

  set<string> propositions;
  set<string> negatedPropositions;
  string linearexpression;

  modalProperty() {
    linearexpression = "";
  }
 
};

#endif
