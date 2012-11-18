%{

/***********************************
NOTES:
~ location can be marked by 'false'
  which means that it is not marked
  by any proposition
************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cNetwork.h"

//increase these two if bison can't parse large files
#define YYMAXDEPTH 50000 
#define YYINITDEPTH 70000

extern int yylineno;
extern struct cNetwork myNet; //used to contain the parsed network
extern void displayFormulaTree(struct fTree* top, int depth);

void yyerror(const char *msg);
%}

%union { //define yylval type (YYSTYPE)
  char* str;
  struct fTree* formulaNode;
  struct cLocation* sLocation;
  struct cAutomaton* sAutomaton;
  struct cTransition* sTransition;
}

%token NETWORK
%token PROPERTY
%token AUTOMATON
%token LOCATIONS
%token LOCATION
%token INVARIANT
%token LABELLING
%token TRANSITIONS
%token ACTION
%token GUARD
%token RESET
%token INITLOC
%token COMMA

%token<str> CLOCK
%token<str> NUMBER
%token<str> STRING

%token<str> LOGVAL
%token<str> LNEG
%token<str> LOPERATOR
%token<str> RELATION
%token<str> SUBSTITUTION
%token<str> MINADD
%token<str> MUL

%token LPAREN
%token RPAREN
%token LCURLY
%token RCURLY

%token SEMICOLON
%token COLON

%type<sAutomaton> autobody
%type<sAutomaton> automaton
%type<sAutomaton> automata

%type<sLocation> location
%type<sLocation> locationlist

%type<sTransition> transition
%type<sTransition> transitionlist

%type<formulaNode> terminal
%type<formulaNode> binom
%type<formulaNode> linearstatement
%type<formulaNode> linearexpression
%type<formulaNode> linearexpressions
%type<formulaNode> labellist
%type<formulaNode> resetexpression
%type<formulaNode> resetexpressions
%type<formulaNode> modalproperty
%type<formulaNode> biclause

%%

init: NETWORK STRING 
      {
	myNet.numberOfClocks = 0;
	myNet.numberOfParameters = 0;
	myNet.netName = (char*) $2;
      }
      LCURLY automata RCURLY SEMICOLON 
      {
	myNet.automata = (struct cAutomaton*) $5;
      }
      PROPERTY COLON modalproperty SEMICOLON 
      {
	myNet.property = $11;
      }

automata: automaton automata  
          {
	    struct cAutomaton* leftAuto = $1;
	    struct cAutomaton* rightAutoQueue = $2;
	    (*leftAuto).next = rightAutoQueue;
	    if(rightAutoQueue != NULL) (*rightAutoQueue).previous = leftAuto;
	  }
        | { $$ = NULL; }

automaton: AUTOMATON STRING LCURLY autobody RCURLY SEMICOLON 
           {
	     struct cAutomaton* currAuto = (struct cAutomaton*) $4;
	     (*currAuto).autoName = strdup($2);
	     $$ = currAuto;
           }

autobody: LOCATIONS COLON locationlist TRANSITIONS COLON transitionlist
          {
	    struct cAutomaton* currAuto = (struct cAutomaton*) malloc(sizeof(struct cAutomaton));
	    (*currAuto).locations = $3;
	    (*currAuto).transitions = $6;
	    (*currAuto).next = NULL;
	    (*currAuto).previous = NULL;
	    $$ = currAuto;
          }

locationlist: location locationlist 
              {
		struct cLocation* currLoc = $1;
		struct cLocation* nextLoc = $2;		
		(*currLoc).next = nextLoc;
		if(nextLoc != NULL) (*nextLoc).previous = currLoc;
		$$ = currLoc;
	      }
            | { $$ = NULL; }

location: LOCATION LPAREN STRING RPAREN INVARIANT COLON linearexpressions COMMA LABELLING COLON labellist SEMICOLON 
          {
	    struct cLocation* currloc = (struct cLocation*) malloc(sizeof(struct cLocation));
	    (*currloc).locationName = strdup($3);
	    (*currloc).isInitial = 0;
	    (*currloc).invariant = (struct fTree*) $7;
	    (*currloc).labelling = (struct fTree*) $11;
	    (*currloc).next = NULL;
	    (*currloc).previous = NULL;
	    $$ = currloc;
	  } 
          | INITLOC LOCATION LPAREN STRING RPAREN INVARIANT COLON linearexpressions COMMA LABELLING COLON labellist SEMICOLON
          {
	    struct cLocation* currloc = (struct cLocation*) malloc(sizeof(struct cLocation));
	    (*currloc).locationName = strdup($4);
	    (*currloc).isInitial = 1;
	    (*currloc).invariant = (struct fTree*) $8;
	    (*currloc).labelling = (struct fTree*) $12;
	    (*currloc).next = NULL;
	    (*currloc).previous = NULL;
	    $$ = currloc;
	  }

transitionlist: transition transitionlist
                {
		  struct cTransition* currTran = $1;
		  struct cTransition* nextTran = $2;		
		  (*currTran).next = nextTran;
		  if(nextTran != NULL) (*nextTran).previous = currTran;
		  $$ = currTran;
		}
              | { $$ = NULL; }

transition: LPAREN STRING COMMA STRING RPAREN
            ACTION COLON STRING COMMA
            GUARD COLON linearexpressions COMMA
            RESET COLON resetexpressions SEMICOLON 
            {
	      struct cTransition* currTrans = (struct cTransition*) malloc(sizeof(struct cTransition));
	      (*currTrans).source = strdup($2);
	      (*currTrans).target = strdup($4);
	      (*currTrans).actionName = strdup($8);
	      (*currTrans).guard = $12;
	      (*currTrans).reset = $16;
	      (*currTrans).next = NULL;
	      (*currTrans).previous = NULL;
	      $$ = currTrans;
            }

labellist: STRING COMMA labellist 
           {
	     struct fTree* currNode = (struct fTree*) malloc(sizeof(struct fTree));
	     (*currNode).oper = strdup($1);
	     (*currNode).leftNode = NULL;
	     (*currNode).rightNode = $3;
	     $$ = currNode;
	   }
         | STRING
	   {
	     struct fTree* currNode = (struct fTree*) malloc(sizeof(struct fTree));
	     (*currNode).oper = strdup($1);
	     (*currNode).leftNode = NULL;
	     (*currNode).rightNode = NULL;
	     $$ = currNode;
	   }
         | LOGVAL
	   {
	     struct fTree* currNode = (struct fTree*) malloc(sizeof(struct fTree));
	     (*currNode).oper = strdup($1);
	     (*currNode).leftNode = NULL;
	     (*currNode).rightNode = NULL;
	     $$ = currNode;
	   }


resetexpressions: resetexpression LOPERATOR resetexpressions
                  {
		    struct fTree* currNode = (struct fTree*) malloc(sizeof(struct fTree));
		    (*currNode).oper = strdup($2);
		    (*currNode).leftNode = $1;
		    (*currNode).rightNode = $3;
		    $$ = currNode;
                  }
                | resetexpression
		   {
		     $$ = $1;
		   }

resetexpression: LOGVAL
                  {
		    struct fTree* currNode = (struct fTree*) malloc(sizeof(struct fTree));
		    (*currNode).oper = strdup($1);
		    (*currNode).leftNode = NULL;
		    (*currNode).rightNode = NULL;
		    $$ = currNode;
		  }
               | terminal SUBSTITUTION terminal
                 {
		    struct fTree* currNode = (struct fTree*) malloc(sizeof(struct fTree));
		    (*currNode).oper = strdup($2);
		    (*currNode).leftNode = $1;
		    (*currNode).rightNode = $3;
		    $$ = currNode;
		 } 

linearexpressions: linearexpression LOPERATOR linearexpressions
                   {
		     struct fTree* currNode = (struct fTree*) malloc(sizeof(struct fTree));
		     (*currNode).oper = strdup($2);
		     (*currNode).leftNode = $1;
		     (*currNode).rightNode = $3;
		     $$ = currNode;
                   }
                 | linearexpression 
		   {
		     $$ = $1;
		   }

linearexpression: LOGVAL
                  {
		    struct fTree* currNode = (struct fTree*) malloc(sizeof(struct fTree));
		    (*currNode).oper = strdup($1);
		    (*currNode).leftNode = NULL;
		    (*currNode).rightNode = NULL;
		    $$ = currNode;
		  }
                | linearstatement RELATION linearstatement
                  {
		    struct fTree* currNode = (struct fTree*) malloc(sizeof(struct fTree));
		    (*currNode).oper = strdup($2);
		    (*currNode).leftNode = $1;
		    (*currNode).rightNode = $3;
		    $$ = currNode;
		  }

linearstatement: binom
                 {
		   $$ = $1;
		 }
               | binom MINADD linearstatement
	         {
		   struct fTree* currNode = (struct fTree*) malloc(sizeof(struct fTree));
		   (*currNode).oper = strdup($2);
		   (*currNode).leftNode = $1;
		   (*currNode).rightNode = $3;
		   $$ = currNode;
		  }

binom: terminal 
       {
	 $$ = $1;
       }
     | terminal MUL terminal
       {
	 struct fTree* currNode = (struct fTree*) malloc(sizeof(struct fTree));
	 (*currNode).oper = strdup($2);
	 (*currNode).leftNode = $1;
	 (*currNode).rightNode = $3;
	 $$ = currNode;
       }

terminal: NUMBER
        {
	  int numval = atoi($1);
	  char absnval[MAXNUM];
	  //alert: .0 is glued after each integer, because smtlib doesn't make any casts
	  sprintf(absnval, "%d.0", abs(numval)); 

	  struct fTree* currNode = (struct fTree*) malloc(sizeof(struct fTree));
	  (*currNode).oper = strdup(absnval);
	  (*currNode).leftNode = NULL;
	  (*currNode).rightNode = NULL;

	  //kind of awkward - smtlib doesn't recognize negative numbers
	  //in arithmetic theories, so -123 should be written as (- 123)
	  if(numval < 0){
	    struct fTree* awkwardNode = (struct fTree*) malloc(sizeof(struct fTree));
	    (*awkwardNode).oper = strdup("-");
	    (*awkwardNode).leftNode = NULL;
	    (*awkwardNode).rightNode = currNode;
	    currNode = awkwardNode;
	  }

	  $$ = currNode;
	}
     | STRING
        {
	  struct fTree* currNode = (struct fTree*) malloc(sizeof(struct fTree));
	  (*currNode).oper = strdup($1);
	  (*currNode).leftNode = NULL;
	  (*currNode).rightNode = NULL;
	  putParam(&myNet, (const char*)$1);
	  $$ = currNode;
	}
     | CLOCK
        {
	  struct fTree* currNode = (struct fTree*) malloc(sizeof(struct fTree));
	  (*currNode).oper = strdup($1);
	  (*currNode).leftNode = NULL;
	  (*currNode).rightNode = NULL;
	  putClock(&myNet, (const char*)$1);
	  $$ = currNode;
	}

/* 
   For now, LOPERATOR should be AND only!
*/
modalproperty: biclause LOPERATOR modalproperty
                {
		  struct fTree* currNode = (struct fTree*) malloc(sizeof(struct fTree));
		  (*currNode).oper = strdup($2);
		  (*currNode).leftNode = $1;
		  (*currNode).rightNode = $3;
		  $$ = currNode;
                }
             | biclause 
	       {
		 $$ = $1;
	       }


biclause: STRING 
           {
	     struct fTree* currNode = (struct fTree*) malloc(sizeof(struct fTree));
	     (*currNode).oper = strdup($1);
	     (*currNode).leftNode = NULL;
	     (*currNode).rightNode = NULL;
	     $$ = currNode;
	   }
        | LNEG STRING 
           {
	     struct fTree* propNode = (struct fTree*) malloc(sizeof(struct fTree));
	     (*propNode).oper = strdup($2);
	     (*propNode).leftNode = NULL;
	     (*propNode).rightNode = NULL;

	     struct fTree* currNode = (struct fTree*) malloc(sizeof(struct fTree));
	     (*currNode).oper = strdup($1);
	     (*currNode).leftNode = NULL;
	     (*currNode).rightNode = propNode;
	     $$ = currNode;
	   }
        | linearexpression 
	   {
	     $$ = $1;
	   }      


%%

void yyerror(const char *msg){
  printf("%s in line %d\n", msg, yylineno);
}
