#include "modelCrawler.h"

modelCrawler::modelCrawler() {
  
  stepctr = 0;

}

modelCrawler::~modelCrawler() {

  cleanNetwork(&myNet);

}

int modelCrawler::readFile(const char* fName) {

  yyin = fopen(fName, "r");
  if(yyin == NULL) return 1;
  int parseResult = yyparse();  
  if(parseResult) return 1;
  fclose(yyin);

  return 0;
  
}

map<string, string> modelCrawler::mapLocations2Invariants(struct cAutomaton* autom) {

  //rewind to the beginning of the queue (just to be sure)
  struct cLocation* tempLocPtr = (*autom).locations;
  while((*tempLocPtr).previous != NULL) tempLocPtr = (*tempLocPtr).previous;

  //build map for current automaton
  map<string, string> helperMap;
  while(tempLocPtr != NULL){ //this goes over all locations in autom
    helperMap[(*tempLocPtr).locationName] = formulaTree2Cvc((*tempLocPtr).invariant);
    if((*tempLocPtr).isInitial) initialState.localStates[(*autom).autoName] = (*tempLocPtr).locationName; //gather initial states

    //gather propositions for given location
    struct fTree* label =  (*tempLocPtr).labelling;
    while(label != NULL) { //goes through all propositions in location

      if(propositionsMap[(*autom).autoName].count((*tempLocPtr).locationName) == 0) 
	propositionsMap[(*autom).autoName][(*tempLocPtr).locationName] = set<string>(); 

      propositionsMap[(*autom).autoName][(*tempLocPtr).locationName].insert( (*label).oper ); //assign proposition to location
      label = (*label).rightNode;

    }

    tempLocPtr = (*tempLocPtr).next;

  }
  
  return helperMap;

}

void modelCrawler::mapAutomata2Locations2Invariants(struct cAutomaton* autom) {
  
  //rewind to the beginning of the queue (just to be sure)
  struct cAutomaton* tempAutPtr = autom;
  while((*tempAutPtr).previous != NULL) tempAutPtr = (*tempAutPtr).previous;

  //build maps for all automata
  while(tempAutPtr != NULL){
    propositionsMap[(*tempAutPtr).autoName] = map<string, set<string> >(); //propositions gatherer
    invariantMap[(*tempAutPtr).autoName] = mapLocations2Invariants(tempAutPtr);
    tempAutPtr = (*tempAutPtr).next;
  }

}

void modelCrawler::mapAutomata2Locations2Invariants() {

  mapAutomata2Locations2Invariants(myNet.automata);

}

map<string, vector<Transition> > modelCrawler::mapLocations2Transitions(struct cAutomaton* autom) {

  string automName = string((*autom).autoName);
  knownActions[automName] = set<string>();

  //rewind to the beginning of the queue (just to be sure)
  struct cTransition* tempTranPtr = (*autom).transitions;
  while((*tempTranPtr).previous != NULL) tempTranPtr = (*tempTranPtr).previous;

  //build map for current automaton
  map<string, vector<Transition> > helperMap;

  //build the map from locations to vectors of outgoing transitions
  string sourceName;
  Transition foundTransition;
  while(tempTranPtr != NULL) {

    sourceName = string((*tempTranPtr).source);
    foundTransition = Transition(autom, tempTranPtr, invariantMap[automName]);
    knownActions[automName].insert(foundTransition.actionName); //remember label of curr. action
    allKnownActions.insert(foundTransition.actionName); //remember label of curr. action (no auto link)

    if(helperMap.count(sourceName) == 0) { //first occurence of sourceName
      helperMap[sourceName] = vector<Transition>();
    }
    helperMap[sourceName].push_back(foundTransition); //build the vector of transitions leaving sourceName
    
    tempTranPtr = (*tempTranPtr).next;
  }

  return helperMap;

}

void modelCrawler::mapAutomata2Locations2Transitions(struct cAutomaton* autom) {

  //rewind to the beginning of the queue (just to be sure)
  struct cAutomaton* tempAutPtr = autom;
  while((*tempAutPtr).previous != NULL) tempAutPtr = (*tempAutPtr).previous;

  //build maps for all automata
  while(tempAutPtr != NULL){
    transitionMap[string((*tempAutPtr).autoName)] = mapLocations2Transitions(tempAutPtr);
    tempAutPtr = (*tempAutPtr).next;
  }

}

void modelCrawler::mapAutomata2Locations2Transitions() {

  mapAutomata2Locations2Transitions(myNet.automata);

}

void modelCrawler::initModel() {
  
  mapAutomata2Locations2Invariants();

  for(int i = 0; i < myNet.numberOfClocks; ++i){
    clockNames.push_back(string(myNet.clockNames[i]));
  }

  for(int i = 0; i < myNet.numberOfParameters; ++i){
    parameterNames.push_back(string(myNet.paraNames[i]));
  }

  mapAutomata2Locations2Transitions();

  //reuse the results from invariantMap to build locationsMap
  //(this should've been done earlier, on-the-fly)
  for(map< string, map<string, string> >::const_iterator iter = invariantMap.begin(); 
      iter != invariantMap.end(); ++iter) { //for each automaton...

    set<string> tempLocGatherer;
    map<string, string> tempLoc2Inv = (*iter).second;
    for(map<string, string>::const_iterator it = tempLoc2Inv.begin(); 
	it != tempLoc2Inv.end(); ++it) { //gather locations
      tempLocGatherer.insert((*it).first);
    }

    map<string, int> tempLoc2Ordinal;
    int ctr = 0;
    for(set<string>::const_iterator it = tempLocGatherer.begin(); 
	it != tempLocGatherer.end(); ++it) {
      tempLoc2Ordinal[(*it)] = ctr++;
    }

    locationsMap[(*iter).first] = tempLoc2Ordinal;
  }
  //P.S. I don't really like C++

  //initialise globalState stuff needed to provide lexicographical encodings
  globalState::init(&locationsMap);

  //initialise transition stuff 
  Transition::init(&clockNames);

  //initialise modal property things
  buildProperty(myNet.property);

}

void modelCrawler::buildProperty(struct fTree* tPtr) {

  if(tPtr == NULL) return;
  if(strcmp((*tPtr).oper, "and") == 0) {
    buildProperty((*tPtr).rightNode);
    buildProperty((*tPtr).leftNode);
  } else { //leaf or 'not' and then leaf
    if(strcmp((*tPtr).oper, "not") == 0) { //negated proposition
      property.negatedPropositions.insert((*tPtr).rightNode->oper);
    } else { //nonnegated propositions and linear expressions
      if(strcmp((*tPtr).oper, "<") == 0 || strcmp((*tPtr).oper, ">") == 0
	 || strcmp((*tPtr).oper, "<=") == 0 || strcmp((*tPtr).oper, ">=") == 0) { //linear expressions
	if(property.linearexpression.size() == 0) {
	  property.linearexpression = formulaTree2Cvc(tPtr);
	} else {
	  property.linearexpression = "( and " + property.linearexpression + " " + formulaTree2Cvc(tPtr) + " )";
	}
      }
      else { //nonnegated propositions
	property.propositions.insert((*tPtr).oper);
      }
    }
  }

}

bool modelCrawler::satisfiesProperty(const globalState& state) {
  
  //  check that the state is marked by all needed propositions
  //  and by none of negated propositions
  set<string> markedBy;
  for(map<string, string>::const_iterator localIter = state.localStates.begin(); 
      localIter != state.localStates.end(); ++localIter) { //take local state
    markedBy.insert(propositionsMap[(*localIter).first][(*localIter).second].begin(),
		    propositionsMap[(*localIter).first][(*localIter).second].end()); //gather its propositions
  }

  //check if all property's propositions are marked
  for(set<string>::const_iterator propos = property.propositions.begin();
      propos != property.propositions.end(); ++propos) { 
    if(markedBy.count(*propos) == 0) return false;
  }

  //check if none of property's negated propositions are marked
  for(set<string>::const_iterator npropos = property.negatedPropositions.begin();
      npropos != property.negatedPropositions.end(); ++npropos) { 
    if(markedBy.count(*npropos) != 0) return false;
  }
  
  return true;

}

void modelCrawler::displayAutomaton(struct cAutomaton* automaton) {

  cout << "Automaton " << (*automaton).autoName << " with locations:" << endl;
  struct cLocation* currLoc = (*automaton).locations;
  while(currLoc != NULL){
    cout << "\t" << (*currLoc).locationName <<" with invariant " 
	 << formulaTree2Cvc((*currLoc).invariant) << endl;
    currLoc = (*currLoc).next;
  }

  cout << "and transitions:" << endl;
  struct cTransition* currTran = (*automaton).transitions;
  while(currTran != NULL){
    cout << "\taction " << (*currTran).actionName << " from " << (*currTran).source 
	 << " to " << (*currTran).target << "\n\t\tguarded by: " << formulaTree2Cvc((*currTran).guard) 
	 << "\n\t\twith reset: " << formulaTree2Cvc((*currTran).reset) << endl; 
    currTran = (*currTran).next;
  }

}

void modelCrawler::displayNetwork(struct cNetwork* net) {

  cout << "\nNetwork named " << (*net).netName << " has " << (*net).numberOfClocks 
       << " clocks and " << (*net).numberOfParameters << " parameters:" << endl;
  int i;
  cout << "parameters: ";
  for(i = 0; i<(*net).numberOfParameters; ++i) cout << (*net).paraNames[i] << " "; 
  cout << "\nclocks: ";
  for(i = 0; i<(*net).numberOfClocks; ++i) cout << (*net).clockNames[i] << " "; 
  cout <<"\n\nComposition:" << endl;

  struct cAutomaton* currAuto = (*net).automata;
  while(currAuto != NULL){ 
    displayAutomaton(currAuto);
    currAuto = (*currAuto).next;
  }

}

void modelCrawler::displayModel() {

  cout << "\nDisplaying clocks in model: " << endl;
  int ctr = 0;
  for(vector<string>::const_iterator iter = clockNames.begin(); 
      iter != clockNames.end(); ++iter) {
    cout << *iter << ((++ctr % 10) ? " " : "\n");
    }
  
  cout << "\n\nDisplaying invariants: " << endl;
  dismapmap(invariantMap);

  cout << "\nDisplaying initial states: " << endl;
  dismap(initialState.localStates);

  cout << "\nDisplaying transitions: " << endl;
  for(map< string, map<string, vector<Transition> > >::const_iterator iter = transitionMap.begin();
      iter != transitionMap.end(); ++iter) {
    cout << "In automaton " << (*iter).first << ":" << endl;
    for(map<string, vector<Transition> >::const_iterator it = (*iter).second.begin();
	it != (*iter).second.end(); ++it) {
      cout << "location " << (*it).first << " is left by transitions:" << endl;
      for(size_t i = 0; i < (*it).second.size(); ++i) cout << (*it).second[i];
    }
  }

  cout << "\nDisplaying known actions: " << endl;
  for(map<string, set<string> >::const_iterator it = knownActions.begin(); 
      it != knownActions.end(); ++it) {
    cout << (*it).first << " is aware of following actions: " << endl;
      for(set<string>::const_iterator iter = (*it).second.begin();
	  iter != (*it).second.end(); ++iter){
	cout << (*iter) << " ";
          }
      cout << endl;
      }

}

vector<Transition> modelCrawler::outgoingTransitions(set<globalState>& states) {

  vector<Transition> globalTransitionsFromStates;

  for(set<globalState>::const_iterator iter = states.begin(); iter != states.end(); ++iter) {

    const globalState &state = *iter;

    for(set<string>::const_iterator actiter = allKnownActions.begin();
	actiter != allKnownActions.end(); ++actiter) { //for each action...

      string currAction = (*actiter);
      //this is a map from automaton name (recall that maps are sorted by names)
      //to the vector of transitions labelled with currAction, starting from input state
      map< string, vector<Transition> > currActionGlobalTrans; 

      for(map<string, string>::const_iterator iter = state.localStates.begin();
	  iter != state.localStates.end(); ++iter) { //for each local state (each autom.)...

	string autoName = (*iter).first;
	string autoLocalStateName = (*iter).second;
	//take all Transitions from current automaton's local input state...
	vector<Transition> outgoingLocalTrans = transitionMap[autoName][autoLocalStateName]; 

	//select Transitions containing the current action...
	vector<Transition> conformingLocalTrans;
	for(size_t i = 0; i < outgoingLocalTrans.size(); ++i) {
	  if(outgoingLocalTrans[i].actionName == currAction)
	    conformingLocalTrans.push_back(outgoingLocalTrans[i]);
	}

	//if the current automaton is aware of currAction but there is no transition
	//from the local state labelled by currAction, then there is no global transition
	//for currAction

	if(!conformingLocalTrans.empty()) {
	  currActionGlobalTrans[autoName] = conformingLocalTrans;
	} else 
	  if(knownActions[autoName].count(currAction) > 0) { //current automaton knows current action
	    currActionGlobalTrans.clear(); 
	    break;
	  } else { //current automaton doesn't know current action, so we stay at the input state
	    Transition dummyTrans;
	    dummyTrans.source.localStates[autoName] = autoLocalStateName;
	    dummyTrans.sourceInvariant = "true"; //try invariantMap[autoName][autoLocalStateName] if smth goes wrong here
	    dummyTrans.target.localStates[autoName] = autoLocalStateName;
	    dummyTrans.targetInvariant = "true"; //try invariantMap[autoName][autoLocalStateName] if smth goes wrong here
	    dummyTrans.guardExpression = "true";
	    dummyTrans.resetExpression = "true";
	    dummyTrans.actionName = currAction; //dummy label
	    vector<Transition> dummyTransVec;
	    dummyTransVec.push_back(dummyTrans);
	    currActionGlobalTrans[autoName] = dummyTransVec;
	  }
      
      } //EO local state selector

      if(currActionGlobalTrans.empty()) continue; //no global transition from input state for current action
      vector<Transition> curractTransitions = localTransitions2GlobalTransitions(currActionGlobalTrans); //global transitions for curr. action
      globalTransitionsFromStates.insert(globalTransitionsFromStates.end(), curractTransitions.begin(), curractTransitions.end());
    } //EO action selector

  }

  //provide raw encoding for used Transitions
  for(size_t i = 0; i != globalTransitionsFromStates.size(); ++i) globalTransitionsFromStates[i].rawencode();

  return globalTransitionsFromStates;

}

vector<Transition> modelCrawler::localTransitions2GlobalTransitions(const map< string, vector<Transition> >& globalTransSet) { 

  //globalTransSet[automaton1] is a vector of local transitions:
  //[src1, actLabel,..., target11]
  //[src1, actLabel,..., target12]
  //   ........................
  //[src1, actLabel,..., target1k]

  vector<Transition> globalTransitionsCombined;
  //initialise globalTransitionsCombined by copying transitions for first automaton
  map< string, vector<Transition> >::const_iterator globalTransIter = globalTransSet.begin();
  globalTransitionsCombined.resize((*globalTransIter).second.size());
  copy((*globalTransIter).second.begin(), (*globalTransIter).second.end(), globalTransitionsCombined.begin());

  ++globalTransIter;
  for(; globalTransIter != globalTransSet.end(); ++globalTransIter) { //for each automaton...
    //...combine its transitions with previous (* overload in transition.h/cc)
    globalTransitionsCombined = globalTransitionsCombined * (*globalTransIter).second; 
  }

  return globalTransitionsCombined;

}

string modelCrawler::encodeProperty() {

  assert(stepctr > 0);  
  stringstream ss; 
  
  int goodLocCtr = 0;
  for(set<globalState>::const_iterator iter = instates.begin(); 
      iter != instates.end(); ++iter) { //for each final location from last unwinding
    if( satisfiesProperty((*iter)) ) { //encode that location
      ++goodLocCtr;
      globalState tempState = (*iter);
      ss << "( or " << tempState.cvcEncode( stepctr * globalState::bitsNo );
    }
  }
  ss << " false ";
  while(goodLocCtr > 0) {
    ss << " )";
    --goodLocCtr;
  }

  //encode linear expression using current clock numbers
  vector<string> iiclockNames;
  for(size_t i = 0; i < clockNames.size(); ++i) {
    stringstream hs1;
    hs1 << clockNames[i] << stepctr;
    iiclockNames.push_back( hs1.str() );
  }

  string result = "";

  if(property.linearexpression.size() > 0) {
    result = "( and " + ss.str() + " " +
      substitute(property.linearexpression, iiclockNames, clockNames) + " )"; 
    return result;
  }

  return ss.str();

}

stringstream& modelCrawler::encodeUnwinding() {

  /* This function actually produces:
     ( and or-clause1 ( and or-clause2 (and or-clause3 ....
     in unwindstream;
     or-clausei is an encoding of transitions from states reachable
     in i-th step.
     This string needs to be completed with:  true ))...)) true). */

  static stringstream unwindstream; //we build encoded unwinding here

  if(stepctr == 0) { //prepare the initial states
    initialState.rawencoding();
    instates.insert(initialState);
    nxt = outgoingTransitions(instates);
  }

  unwindstream << "( and "; //join or-clauses

  //build or-clause (i-th disjunction over transitions)
  size_t tokenctr = 0; //build first orclause
  
  if(nxt.size() == 0) {
    cerr << "\nBreaking: no more outgoing transitions." << endl;
    abort();
  }

  for(tokenctr = 0; tokenctr < nxt.size() - 1; ++tokenctr) {
    unwindstream << "( or " << nxt[tokenctr].tr_enc(stepctr);
  }

  unwindstream << " " << nxt[nxt.size() - 1].tr_enc(stepctr);
  while(tokenctr-- > 0) unwindstream <<  " )";
  //we've put ( and (or trans1 (or trans2 (...))) on unwindstream

  instates = Transition::getTargetGlobalStates(nxt); //compute start states of stepctr+1-th unwinding (bottleneck!)

  nxt = outgoingTransitions(instates); //compute transitions of that unwinding (real bottleneck!)

  ++stepctr;

  return unwindstream;

}

string modelCrawler::makePreamble(int maxClockVarno, string logicName) {
  
  //this should be run after encoding of the unwinding
  stringstream ss;

  //declare logic
  ss << "(set-logic " << logicName << ")" << endl;

  //declare clocks
  for(int i = 0; i <= maxClockVarno + 1; ++i) {
    for(size_t j = 0; j < clockNames.size(); ++j) {
      //clocks are real
      ss << "(declare-fun " << clockNames[j] << i << " () Real)" << endl;
      //and non-negative
      ss << "(assert ( >= " << clockNames[j] << i << " 0 ) )" << endl;      
    }
  }

  //initial clock values
  //prepare first timed var, which is Real
  ss << "(declare-fun " << Transition::timedVar << "init" << " () Real)" << endl;
  //and non-negative
  ss << "(assert ( >= " << Transition::timedVar << "init" << " 0 ) )" << endl;      
  //set initial clocks on first timed var
  for(size_t i = 0; i < clockNames.size(); ++i) {
    ss << "(assert (= " << clockNames[i] << 0 << " " << Transition::timedVar << "init"<< " ) )" << endl;
  }

  //declare parameters
  for(size_t i = 0; i < parameterNames.size(); ++i) {
    //parameters are real (no casts to int in smtlib)
    ss << "(declare-fun " << parameterNames[i] << " () Real)" << endl;
    //and integer
    ss << "(assert ( is_int " << parameterNames[i] << " ) )" << endl;      
    //and non-negative
    ss << "(assert ( >= " << parameterNames[i] << " 0 ) )" << endl;      
  }

  //declare timed vars
  for(int i = 0; i <= maxClockVarno + 1; ++i) {
    //timed vars are real
    ss << "(declare-fun " << Transition::timedVar << i << " () Real)" << endl;
    //and non-negative
    ss << "(assert ( >= " << Transition::timedVar << i << " 0 ) )" << endl;      
  }

  //declare booleans
  for(int bctr = 0; bctr <= globalState::maxBoolVarNr; ++bctr) {
    ss << "(declare-fun " << globalState::encodingVarPrefix << bctr << " () Bool)" << endl;    
  }

  return ss.str();

}

string modelCrawler::unwindingTail(int unwindingCalls) {

  stringstream ss;
  ss << " true ";
  for(int i = 0; i < unwindingCalls - 1; ++i) {
    ss << ") "; //complete the unwinding clause
  }
  ss << " true )";

  return ss.str();

}

