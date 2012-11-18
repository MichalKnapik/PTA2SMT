#include "experiment.h"

#define TEMPFILENAME "temp"
#define ERRFILENAME "errors"

using namespace std;

/****************************************************************************
 Experiment file syntax:
 [smtchecker]
 [lower_param1 lower_parameter2 .... lower_paramk] (leave empty if none)
 [unwinding depth1] [number of steps if SAT1]
 .......
 [unwinding depthn] [number of steps if SATn]
 ****************************************************************************/

vector< pair<int, int> > unwindLengthAndSteps; //holds pairs [unwinding depthn] [number of steps if SATn]
set<string> lowerParameters;
set<string> upperParameters;
string smtchecker;
string logic;

int readExperimentDetails(string expFileName) {

  ifstream is(expFileName.c_str());
  string paramstring;

  if(is.is_open()) {

    if(is.good()) {
      getline(is, smtchecker); //checker command
      getline(is, logic);
      getline(is, paramstring);
      //tokenize
      string buffer;
      stringstream ss(paramstring);
      while(ss >> buffer) lowerParameters.insert(buffer);
    }

    int a,b;    
    while(is.good() && is >> a >> b) {
      unwindLengthAndSteps.push_back(pair<int,int>(a,b));
    }

    is.close();
  } else { 
    cerr << "Experiment file: I/O error!" << endl;
    return 1;
  }

  if(unwindLengthAndSteps.size() == 0 || smtchecker.length() == 0) {
    cerr << "Error in experiment file!" << endl;
    return 1;
  }

  return 0;

}

void printGreetings() {
  
  cout << "*************************************************************\n"
       << "*       Parametric Reachability SMT-Translator, 2012.       *\n" 
       << "*    If needed, contact M. Knapik: mknapik@ipipan.waw.pl.   *\n" 
       << "*-----------------------------------------------------------*\n"
       << "*       Usage: ./pta2smt modelFile experimentPlanFile       *\n"     
       << "*************************************************************\n" << endl;

}

int initExperiment(modelCrawler& model, int argc, char** argv) {

  int res = model.readFile(argv[1]);
  if(res == 1) {
    cerr << "Model file: I/O error!" << endl;
    return 1;
  }

  if(argc < 3) {
    cerr << "Specify experiment plan file." << endl;
    return 1;
  }
  if(readExperimentDetails(argv[2]) == 1) return 1;

  model.initModel(); 

  size_t foundElems = 0;
  for(size_t i = 0; i < model.parameterNames.size(); ++i) {
    if(count(lowerParameters.begin(), lowerParameters.end(), model.parameterNames[i]) == 1) ++foundElems;
    else upperParameters.insert(model.parameterNames[i]);
  }
  if(foundElems != lowerParameters.size()) {
    cerr << "There's something wrong in the parameter list!" << endl;
    return 1;
  }

  return 0;

}

int runSingleExperiment(modelCrawler& model, int unwindingDepth, int noOfSteps, string logic) {

  static int previousUnwinding = 0;

  int steps = unwindingDepth - previousUnwinding;
  if(steps <= 0) {
    cerr << "There's something wrong in the experiment plan: " << previousUnwinding 
	 << " unwinding is not decremental wrt the next one" << endl;
    return 1;
  }
  
  //model unwinding
  struct timeval timr;
  gettimeofday(&timr, NULL);
  double unwindingstart = timr.tv_sec + (timr.tv_usec/1000000.0);
  cout << "Unwinding model up to "<< unwindingDepth <<", starting from " << previousUnwinding << ": [" << flush;  

  while(--steps > 0) {
    model.encodeUnwinding();
    cout << "=" << flush;
  }

  string unw = model.encodeUnwinding().str(); 
  string preamble = model.makePreamble(unwindingDepth, logic);
  string tail = model.unwindingTail(unwindingDepth);
  string property = model.encodeProperty();
  string complementClause = "true";

  gettimeofday(&timr, NULL);
  double unwindingend = timr.tv_sec + (timr.tv_usec/1000000.0);
  cout << "=] (" << (unwindingend - unwindingstart) << " sec.)" << endl;

  //iteration over SATs
  FILE *cfile;
  char buffer[1024];
  int ctr = 0;
  while(noOfSteps-- > 0) {
    
    //output the formula to temporary file 
    ofstream of(TEMPFILENAME);
    //save model 
    of << preamble << endl 
       << "(assert " << unw << tail << " )" << endl
       << "(assert " << property << " )" << endl;
    //reuse of previous run (discard already known valuations)
    of << "(assert " << complementClause << " )" << endl;
    //needed to start computations
    of << "(check-sat)" << endl;
    //valuation fetching
    if(model.parameterNames.size() > 0) {
      of << "(get-value (";
      for(size_t i = 0; i < model.parameterNames.size(); ++i) {
	of << model.parameterNames[i] << " ";
      }
      of << "))" << endl;
    }
    of.close();

    //get temp file size
    struct stat tempfiledata;
    if(stat(TEMPFILENAME, &tempfiledata) == -1) {
      cerr << "I/O error while accessing temporary " << TEMPFILENAME << " file!" << endl;
      return 1;
    }
    cout << "Formula file size: " << (tempfiledata.st_size / 1048576.0) << " MB." << endl;

    //call smtchecker    
    cout << "SMT-check in progress: " << flush;
    gettimeofday(&timr, NULL);
    double checkerstart = timr.tv_sec + (timr.tv_usec/1000000.0);
    cfile = popen((smtchecker + " " + TEMPFILENAME + " 2>>" + ERRFILENAME).c_str(), "r"); 
    if(!cfile) {
      cerr << "I/O error while accessing temporary " << TEMPFILENAME << " file!" << endl;
      return 1;
    }

    //capture output
    fgets(buffer, sizeof(buffer), cfile);

    //buffer now contains "sat" or "unsat" + rubbish
    assert(buffer[0] == 's' || buffer[0] == 'u');

    if(buffer[0] == 's') { //SAT

      cout << "SAT on step " << ++ctr << " of unwinding with depth " << unwindingDepth << ". ";
      //harvest parameters valuation
      vector<string> tempVals;
      vector<int> processedTempVals;
      while(fgets(buffer, sizeof(buffer), cfile) != NULL) {
	tempVals.push_back(buffer);
      }    
      tempVals[0] = tempVals[0].substr(1); //remove '('
      if(tempVals.size() > 1) {
	int llen = tempVals[tempVals.size() - 1].length();
	tempVals[tempVals.size() - 1] = tempVals[tempVals.size() - 1].substr(0, llen); //remove ')'
      }
      for(size_t i = 0; i < tempVals.size(); ++i) processedTempVals.push_back(atoi(tempVals[i].c_str()));
      /**********************************************************
       The idea of complement: 
        for each lower parameter l and generated value lval
        add clause l > lval, and for each upper parameter u
        and generated value rval add clause u < rval if
        rval > 0.
        All these clauses are then joined by ORs.
      ***********************************************************/
      cout << "\nValuation: ";
      stringstream complpropstr; 
      int openbrackets = 0;
      complpropstr << "( and "; //join with previous complements
      for(size_t i = 0; i < model.parameterNames.size(); ++i) {
	string parameter = model.parameterNames[i];
	int value = processedTempVals[i];
	cout << parameter << " -> " << value << " ";
	if(lowerParameters.count(parameter) == 1) { //lower parameter
	  complpropstr << "( or ( > " << parameter << " " << value << " )";
	  ++openbrackets;
	} else { //upper parameter
	  if(value > 0) {
	    complpropstr << "( or ( < " << parameter << " " << value << " )";
	    ++openbrackets;
	  }
	}
      }
      complpropstr << " false";
      while(openbrackets-- > 0) complpropstr << " )";

      complpropstr << " " << complementClause << ")"; //join with previous complements

      complementClause = complpropstr.str(); //update complement clause

    } else { //nonSAT

      cout << "unSAT on step " << ++ctr << " of unwinding with depth " << unwindingDepth << ". ";
      previousUnwinding = unwindingDepth;
      pclose(cfile);
      gettimeofday(&timr, NULL);
      double checkerend = timr.tv_sec + (timr.tv_usec/1000000.0);
      cout << "("  << (checkerend - checkerstart) << " sec.)" << endl;
      return 0; 

    } 

    gettimeofday(&timr, NULL);
    double checkerend = timr.tv_sec + (timr.tv_usec/1000000.0);
    cout << "("  << (checkerend - checkerstart) << " sec.)" << endl;

    pclose(cfile);

  }
  //----------------------------------

  previousUnwinding = unwindingDepth;

  return 0;

}

int main(int argc, char** argv){

  printGreetings();

  modelCrawler model;

  if(initExperiment(model, argc, argv) == 1) return 1;

  system(string(string("rm ") + ERRFILENAME).c_str());
  for(size_t i = 0; i < unwindLengthAndSteps.size(); ++i) {
    if(runSingleExperiment(model, unwindLengthAndSteps[i].first, unwindLengthAndSteps[i].second, logic) == 1) return 1; 
  }

  return 0;

}
