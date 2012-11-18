import sys

def gen_producer():
    #locations
    print "automaton Producer {"
    print
    print "locations:"
    print "initial location(ProdReady)"
    print "        invariant: x1 <= d and xdummy >= 0, labelling: ProdReady;"
    print
    print "        location(ProdWaiting)"
    print "        invariant: x1 <= b, labelling: ProdWaiting;"
    #transitions    
    print
    print "transitions:"
    print "            (ProdReady, ProdWaiting)"
    print "            action: feed2,"
    print "            guard: x1 >= c,"
    print "            reset: x1 = 0;"
    print
    print "            (ProdWaiting, ProdReady)"
    print "            action: prodReset,"
    print "            guard: x1 >= a,"
    print "            reset: x1 = 0;"
    print
    print "};"

def gen_consumer(n):
    #locations
    print "automaton Consumer {"
    print
    print "locations:"
    print "initial location(ConsReady)"
    print "        invariant: x" + str(n) + " <= d, labelling: ConsReady;"
    print
    print "        location(ConsWaiting)"
    print "        invariant: x" + str(n) + " <= b, labelling: ConsWaiting;"
    #transitions    
    print
    print "transitions:"
    print "            (ConsReady, ConsWaiting)"
    print "            action: feed" + str(n) + ","
    print "            guard: x" + str(n) + " >= c,"
    print "            reset: x" + str(n) + " = 0;"
    print
    print "            (ConsWaiting, ConsReady)"
    print "            action: ConsReset,"
    print "            guard: x" + str(n) + " >= a,"
    print "            reset: x" + str(n) + " = 0;"
    print
    print "};"


def gen_node(i,k):
    #locations
    print "automaton Node" + str(i) + " {"
    print
    print "locations:"
    print "initial location(Node" + str(i) + "Ready)"
    print "        invariant: x" + str(i) + " <= d, labelling: Node" + str(i) + "Ready;"
    print
    print "        location(Node" + str(i) + "Send)"
    print "        invariant: x" + str(i) + " <= d, labelling: Node" + str(i) + "Send;"
    for j in xrange(k):
        print
        print "        location(Node" + str(i) + "Process"+ str(j + 1)  + ")"
        print "        invariant: x" + str(i) + " <= f, labelling: Node" + str(i) + "Process"+ str(j + 1) + ";"
    #transitions        
    print
    print "transitions:"
    print "            (Node" + str(i) + "Ready, Node" + str(i) + "Process1)"
    print "            action: feed" + str(i) + ","
    print "            guard: x" + str(i) + " >= c,"
    print "            reset: x" + str(i) + " = 0;"
    print
    print "            (Node" + str(i) + "Process" + str(k) +", Node" + str(i) + "Send)"
    print "            action: Node" + str(i) + "Process" + str(k) + ","
    print "            guard: x" + str(i) + " >= e,"
    print "            reset: x" + str(i) + " = 0;"
    print
    print "            (Node" + str(i) + "Send, Node" + str(i) + "Ready)"
    print "            action: feed" + str(i + 1) + ","
    print "            guard: x" + str(i) + " >= c,"
    print "            reset: x" + str(i) + " = 0;"
    print
    for j in xrange(k - 1):
        print
        print "            (Node" + str(i) + "Process" + str(j + 1) + ", Node" + str(i) + "Process" + str(j + 2) + ")"
        print "            action: Node" + str(i) + "Process" + str(j + 1) + ","
        print "            guard: x" + str(i) + " >= e,"
        print "            reset: x" + str(i) + " = 0;"
    print "};"


def gen_property():
    print "property: ConsWaiting and ProdReady and xdummy >= 5;"

try:
    length = int(sys.argv[1])
    width = int(sys.argv[2])

    print "network Pipeline {"
    gen_producer()

    for i in xrange(length):
        gen_node(i + 1, width)

    gen_consumer(length)
    print "};"

    gen_property()

except:
    print "USAGE: python pipelinegen.py number_of_nodes height_of_nodes"
