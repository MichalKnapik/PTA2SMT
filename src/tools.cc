#include "tools.h"

string formulaTree2Cvc(struct fTree* top) {

  if(top == NULL) return "";
  if(((*top).leftNode == NULL) && ((*top).rightNode == NULL)){ //terminal node
    return (*top).oper;
  }

  string result = "( " + string((*top).oper) + " "  + 
                  formulaTree2Cvc((*top).leftNode) + " " +
		  formulaTree2Cvc((*top).rightNode) + " )";

  return result;

}

string substitute(const string& inexpression, vector<string>& newClockNames, vector<string>& clockNames) {

  string expression = inexpression;
  for(size_t i = 0; i < clockNames.size(); ++i){

    size_t startpos = 0;
    size_t found = expression.find(clockNames[i] + " ");
    size_t charsNo = 0;

    while(found != string::npos) {
      charsNo = clockNames[i].length();
      expression.replace(found, charsNo, newClockNames[i]);
      startpos = found + charsNo;
      found = expression.find(clockNames[i] + " ", startpos);
    }

  }

  return expression;

}

void dismap(map<string, string> m) {
    
  for(map<string, string>::const_iterator iter = m.begin();
      iter != m.end(); ++iter){
    cout << iter->first << ": " << iter->second << endl;
  }
    
}

void dismapmap(map< string, map<string, string> > mm) {

  for(map<string, map<string, string> >::const_iterator iter = mm.begin();
      iter != mm.end(); ++iter){
    cout << "In " << iter->first << ":" << endl;
    dismap(iter->second);
  }

}

BIGINT lexicogrValue(vector<int>& inputArray, vector<int>& maxSizesBasis) {

  BIGINT currPi = 1;
  BIGINT result = 0;

  for(int i = maxSizesBasis.size() - 1; i >= 0; --i) {
    result += inputArray[i] * currPi;
    currPi *= maxSizesBasis[i];
  }

  return result;
  
}

vector<bool> int2binvect(BIGINT number, int usedBits){

  //it's convenient to reverse true <-> false here

  vector<bool> result;
  while(number > 0) {
    if(number % 2 == 1) result.push_back(false);
    else result.push_back(true);
    number = number / 2;
    --usedBits;
  }

  while(usedBits > 0) {
    result.push_back(true);
    --usedBits;    
  }

  return result;

}
