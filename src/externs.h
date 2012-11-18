#ifndef EXTERNS_H
#define EXTERNS_H

extern "C" int yyparse();
extern "C" FILE *yyin;
/* This is a hack I'm not proud of:                          *  
 * myNet is a shared object built and filled in the process  *
 * of bison/flex parsing. The object resides in cNetwork.c.  */
extern "C" struct cNetwork myNet;  
extern "C" void cleanNetwork(struct cNetwork* cnetwork);
extern "C" void displayFormulaTree(struct fTree* top, int depth);
#endif
