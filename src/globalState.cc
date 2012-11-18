#include "globalState.h"

//static definitions
string globalState::encodingVarPrefix = "lv";
map<string, map<string, int> >* globalState::locationsMap = NULL;
vector<int> globalState::encodingBasis;
int globalState::bitsNo = 0;
int globalState::maxBoolVarNr = 0;

ostream& operator<<(ostream& output, const globalState& gst){
  
  for(map<string, string>::const_iterator iter = gst.localStates.begin(); 
      iter != gst.localStates.end(); ++iter) {
    output << "(" << (*iter).first << "," << (*iter).second << ")" << endl;
  }

  output << "raw encoding: ";
  if(gst.integerEncoding < 0) output << "unknown";
  else output << gst.integerEncoding;

  output << endl;

  return output;

}

bool globalState::operator<(const globalState& right) const {

  assert(right.localStates.size() == right.localStates.size());

  int compareval = 0;
  map<string, string>::const_iterator iterl = localStates.begin();
  map<string, string>::const_iterator iterr = right.localStates.begin();

  while((iterl != localStates.end()) && (iterr != right.localStates.end())) { //awkward
    assert((*iterl).first == (*iterr).first);

    compareval = (*iterl).second.compare((*iterr).second);
    if(compareval < 0) return true;
    else if(compareval > 0) return false;
 
    ++iterl;
    ++iterr;
  }

  return false;

}

void globalState::init(map<string, map<string, int> >* locsMap) {

  locationsMap = locsMap; 
  //prepare basis for encoding,
  //also compute number of bits needed to hold all the states
  //(to be changed in the future versions, so that we hold only the reachable
  //globalStates)
  double allStates = 1;

  map<string, map<string, int> >& locsMapRes = *locsMap;
  for(map<string, map<string, int> >::const_iterator iter = locsMapRes.begin();
      iter != locsMapRes.end(); ++iter) {
    encodingBasis.push_back((*iter).second.size());
    allStates *= (*iter).second.size();

  }

  
  bitsNo = ceil(log2(allStates));

}

BIGINT globalState::rawencoding() {

  if(integerEncoding < 0) {
    
    vector<int> locationsVal;
    for(map<string, string>::const_iterator iter = localStates.begin();
	iter != localStates.end(); ++iter) { //compute lexicographical vector of the instance
      //(*iter).first is an automaton
      //(*iter).second is a location
      locationsVal.push_back( (*locationsMap)[(*iter).first][(*iter).second] );
    }

    integerEncoding = lexicogrValue(locationsVal, encodingBasis);

  }

  return integerEncoding;

}

string globalState::cvcEncode(int startNo) {

  assert(integerEncoding >= 0); //the state has been raw-encoded already

  vector<bool> binval = int2binvect(integerEncoding, bitsNo);

  maxBoolVarNr = startNo + binval.size() - 1;

  stringstream ss;
  if(binval.size() == 1) {
    if(binval[0]) {
      ss << globalState::encodingVarPrefix << (0 + startNo);
    }
    else {
      ss << " ( not " << globalState::encodingVarPrefix << (0 + startNo) << " ) ";
    }
  }
  else {

    for(size_t i = 0; i < binval.size() - 1; ++i) {
      ss << " ( and ";
      if(binval[i]) ss << globalState::encodingVarPrefix << (i + startNo);
      else ss << " ( not " << globalState::encodingVarPrefix << (i + startNo) << " ) ";
    }

    if(binval[binval.size()-1]) ss <<" " << globalState::encodingVarPrefix << (binval.size() - 1 + startNo);
    else ss << " ( not " << globalState::encodingVarPrefix << (binval.size() - 1  + startNo) << " ) ";

    for(size_t i = 0; i < binval.size() - 1; ++i) {
      ss << " ) ";
    }
  }

  return ss.str();

}
