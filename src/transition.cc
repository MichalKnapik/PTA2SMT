#include "transition.h"

//static definitions
vector<string> *Transition::clockNamesPtr = NULL;
string Transition::timedVar = "t";

Transition::Transition(const struct cAutomaton* autom, const struct cTransition* trans, map<string, string>& locationsMap) {
    
  //each location needs to be declared
  if(locationsMap.count((*trans).source) == 0) {
    cerr << "Location " << (*trans).source <<" in " << (*autom).autoName << " undeclared. Aborting." << endl;
  }
  if (locationsMap.count((*trans).target) == 0) {
    cerr << "Location " << (*trans).target <<" in " << (*autom).autoName << " undeclared. Aborting." << endl;
  }

  source.localStates[(*autom).autoName] = (*trans).source;
  sourceInvariant = locationsMap[(*trans).source];

  target.localStates[(*autom).autoName] = (*trans).target;
  targetInvariant = locationsMap[(*trans).target];

  actionName = (*trans).actionName;

  guardExpression = formulaTree2Cvc((*trans).guard);
  resetExpression = formulaTree2Cvc((*trans).reset);

}

ostream& operator<<(ostream& output, const Transition& tran) {
  
  output << "Transition from:\n" << tran.source
         << "with invariant: " << tran.sourceInvariant
         << "\nlabelled by: " << tran.actionName
         << "\nguarded by: " << tran.guardExpression
         << "\nreset with: " << tran.resetExpression
         << "\nto:\n" << tran.target
         << "with invariant: " << tran.targetInvariant;

  return output;

}

Transition Transition::operator*(const Transition& rightTran) {
  
  Transition result;

  //dealing with action 
  assert(actionName == rightTran.actionName); //assume that we only multiply trans.
  result.actionName = actionName;             //with the same action labels

  //combining into smtlib rpn
  stringstream ss1, ss2, ss3, ss4;
  ss1 << "( and " << sourceInvariant << " " << rightTran.sourceInvariant << " )";
  ss2 << "( and " << targetInvariant << " " << rightTran.targetInvariant << " )";
  ss3 << "( and " << guardExpression << " " << rightTran.guardExpression << " )";
  ss4 << "( and " << resetExpression << " " << rightTran.resetExpression << " )";
  result.sourceInvariant = ss1.str();
  result.targetInvariant = ss2.str();
  result.guardExpression = ss3.str();
  result.resetExpression = ss4.str();

  //build proper source and target globalStates
  result.source.localStates.insert(source.localStates.begin(), source.localStates.end());
  result.source.localStates.insert(rightTran.source.localStates.begin(), rightTran.source.localStates.end());
  result.target.localStates.insert(target.localStates.begin(), target.localStates.end());
  result.target.localStates.insert(rightTran.target.localStates.begin(), rightTran.target.localStates.end());

  return result;

}

set<globalState> Transition::getTargetGlobalStates(const vector<Transition>& transitions) {

  set<globalState> result;
  for(size_t i = 0; i < transitions.size(); ++i) {
    result.insert(transitions[i].target);
  }

  return result;

}


vector<Transition> operator*(const vector<Transition> left, const vector<Transition> right) {
  
  vector<Transition> result;
  assert(left.size() > 0 && right.size() > 0);

  for(vector<Transition>::const_iterator liter = left.begin(); liter != left.end(); ++liter) 
    for(vector<Transition>::const_iterator riter = right.begin(); riter != right.end(); ++riter) {
      Transition left = (*liter);
      Transition right = (*riter);
      result.push_back(left*right);
    }

  return result;

}

void Transition::rawencode() {

  source.rawencoding();
  target.rawencoding();

}

string Transition::tr_enc(int ctr) {

  static const size_t clockNo = (*clockNamesPtr).size();
  //prepare ctr-indexed versions of clockNames
  vector<string> iclockNames; //clockNames concatenated with ctr
  vector<string> iiclockNames; //clockNames concatenated with ctr + 1
  vector<string> riclockNames; //each clockName is substituted with ( - clockName*(ctr + 1) t*(ctr + 1) )
  vector<string> nriclockNames; //each clockName is substituted with ( = ( - clockName*(ctr + 1) t*(ctr + 1) ) clockName*ctr)
  for(size_t i = 0; i < clockNo; ++i) {
    stringstream hs1, hs2, hs3, hs4;
    hs1 << (*clockNamesPtr)[i] << ctr;
    iclockNames.push_back( hs1.str() );
    hs2 << (*clockNamesPtr)[i] << (ctr + 1);
    iiclockNames.push_back( hs2.str() );
    hs3 << "( - " << (*clockNamesPtr)[i] << (ctr + 1) << " " << timedVar << (ctr + 1) << " )";
    riclockNames.push_back( hs3.str() );
    hs4 << "( = " << hs3.str() << " " << hs1.str() << " )";
    nriclockNames.push_back( hs4.str() );
  }

  stringstream ss;
  ss << "( and " << source.cvcEncode( ctr * globalState::bitsNo ); 
  ss <<  " ( and " << substitute(sourceInvariant, iclockNames, *clockNamesPtr);

  ss << " ( and " << substitute(guardExpression, iclockNames, *clockNamesPtr);


  string resetEnc = substitute(resetExpression, riclockNames, *clockNamesPtr);
  //add nonreset clocks to resetExpression: this isn't optimal
  string nonresetGatherer = "";
  for(size_t i = 0; i < (*clockNamesPtr).size(); ++i) {
    if( resetExpression.find((*clockNamesPtr)[i] + " ") == string::npos ) {
      if(nonresetGatherer.size() == 0) nonresetGatherer = nriclockNames[i];
      else nonresetGatherer = "( and " + nonresetGatherer + " " + nriclockNames[i] + " )"; 
    }
  }
  if(nonresetGatherer.size() != 0) resetEnc = "( and " + resetEnc + " " + nonresetGatherer + " )";

  ss << " ( and " << resetEnc;  

  ss <<  " ( and " << target.cvcEncode( (ctr + 1) * globalState::bitsNo );
  ss <<  " " << substitute(targetInvariant, iiclockNames, *clockNamesPtr) << " ) ) ) ) ) ";

  return ss.str();

}

void Transition::init(vector<string> *clockNames) {

  Transition::clockNamesPtr = clockNames;

}
