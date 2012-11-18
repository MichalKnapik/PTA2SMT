all:
	bison -d timedAutomataParser.y
	flex timedAutomataLexer.l
	gcc -c -Wall timedAutomataParser.tab.c stringGarbage.c lex.yy.c cNetwork.c -lfl 
	g++ -c -Wall experiment.cc tools.cc transition.cc globalState.cc modalProperty.cc modelCrawler.cc 
	g++ tools.o experiment.o transition.o globalState.o modalProperty.o modelCrawler.o stringGarbage.o cNetwork.o timedAutomataParser.tab.o lex.yy.o -o pta2smt

clean:
	rm lex.yy.c *.tab.c *.tab.h *.o pta2smt
