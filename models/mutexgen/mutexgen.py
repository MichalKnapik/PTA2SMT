import sys

def gen_process(i):
    print "automaton Proc" + str(i) + " {"
    print "locations:"
    print "initial location(idle" + str(i) + ")"
    print "        invariant: true, labelling: false;"

    print "        location(trying" + str(i) + ")"
    print "        invariant: true, labelling: false;"

    print "        location(waiting" + str(i) + ")"
    print "        invariant: true, labelling: false;"

    print "        location(critical" + str(i) + ")"
    print "        invariant: true, labelling: bad"+str(i) + ";"

    print "transitions:"
    print "            (idle" + str(i) + ", trying" + str(i) + ")"
    print "            action: start" + str(i) + ","
    print "            guard: true,"
    print "            reset: x" + str(i) + "= 0;"

    print "            (trying" + str(i) + ", waiting" + str(i) + ")"
    print "            action: setx" + str(i) + ","
    print "            guard: x" + str(i) + " < Delta,"
    print "            reset: x" + str(i) + "= 0;"

    print "            (waiting" + str(i) + ", critical" + str(i) + ")"
    print "            action: enter" + str(i) + ","
    print "            guard: x" + str(i) + " > delta,"
    print "            reset: true;"

    print "            (critical" + str(i) + ", idle" + str(i) + ")"
    print "            action: setx0" + str(i) + ","
    print "            guard: true,"
    print "            reset: true;"        
    print "};"

def gen_synchro(i):

    print "automaton Sync {"
    print "locations:"

    print "initial location(zero)"
    print "        invariant: true, labelling: false;" 
    
    for j in xrange(i):
        print "        location(process" + str(j) +")"
        print "        invariant: true, labelling: false;"        

    print "transitions:"
    
    for j in xrange(i):
        print "            (zero, zero)"
        print "            action: start" + str(j) + ","
        print "            guard: true,"
        print "            reset: true;"

    for j in xrange(i):
        print "            (zero, process" + str(j) + ")"
        print "            action: setx" + str(j) + ","
        print "            guard: true,"
        print "            reset: true;"

    for j in xrange(i):
        print "            (process" + str(j) + ", zero)"
        print "            action: setx0" + str(j) + ","
        print "            guard: true,"
        print "            reset: true;"

    for j in xrange(i):
        print "            (process" + str(j) + ", process" + str(j) + " )"
        print "            action: enter" + str(j) + ","
        print "            guard: true,"
        print "            reset: true;"

    for j in xrange(i):
        for k in xrange(i):
            print "            (process" + str(j) + ", process" + str(k) + " )"
            print "            action: setx" + str(k) + ","
            print "            guard: true,"
            print "            reset: true;"
    
    print "};"    

def gen_property():
    print "property: bad0 and bad1;"

try:
    no = int(sys.argv[1])

    print "network Mutex {"

    for i in xrange(no):
        gen_process(i)

    gen_synchro(no)

    print "};"
    gen_property()

except:
    print "USAGE: python mutexgen.py number_of_nodes"
