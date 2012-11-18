#ifndef EXPERIMENT_H
#define EXPERIMENT_H

#include "modelCrawler.h"
#include "cNetwork.h"
#include "tools.h"
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <utility>
#include <fstream>

int readExperimentDetails(string expFileName);

int initExperiment(modelCrawler& model, int argc, char** argv);

int runSingleExperiment(modelCrawler& model, int unwindingDepth, int noOfSteps, string logic);

#endif

