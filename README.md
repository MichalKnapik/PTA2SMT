Satisfiability Modulo Theories Parametric Timed Automata Tool
=============================================================

This is a prototype tool for synthesis of parameters under which some given state of a system
described using Parametric Timed Automata is reachable. It's a companion to paper
http://www.ceur-ws.org/Vol-851/paper6.pdf.

Configuration / prerequisites
---------------------------------------------------------------------------

Apart from usual dependencies the tool needs to have an access to smtlib2
compliant SMT checker. (It has been tested with CVC3 only.)
The command to start SMT checker needs to be included in the experiment's 
plan file. 

Basic usage
---------------------------------------------------------------------------

Below you can find an example of 2-process Mutex consisting of two 
processes (Proc0, Proc1) and synchronising shared variable model (Sync).
To understand the model you need to consult the cited paper: one look
at Figure 1 should explain everything. The only nonintuitive thing is
perhaps that nonlabeled states need to be marked with *labelling: false*.
(The parser was written in a hurry.)


```
network Mutex {

automaton Proc0 {
locations:
initial location(idle0)
        invariant: true, labelling: false;

        location(trying0)
        invariant: true, labelling: false;

        location(waiting0)
        invariant: true, labelling: false;

        location(critical0)
        invariant: true, labelling: bad0;
transitions:
            (idle0, trying0)
            action: start0,
            guard: true,
            reset: x0 = 0;

            (trying0, waiting0)
            action: setx0,
            guard: x0 < Delta,
            reset: x0 = 0;

            (waiting0, critical0)
            action: enter0,
            guard: x0 > delta,
            reset: true;

            (critical0, idle0)
            action: setx00,
            guard: true,
            reset: true;
};

automaton Proc1 {
locations:
initial location(idle1)
        invariant: true, labelling: false;

        location(trying1)
        invariant: true, labelling: false;

        location(waiting1)
        invariant: true, labelling: false;

        location(critical1)
        invariant: true, labelling: bad1;
transitions:
            (idle1, trying1)
            action: start1,
            guard: true,
            reset: x1 = 0;

            (trying1, waiting1)
            action: setx1,
            guard: x1 < Delta,
            reset: x1 = 0;

            (waiting1, critical1)
            action: enter1,
            guard: x1 > delta,
            reset: true;

            (critical1, idle1)
            action: setx01,
            guard: true,
            reset: true;
};

automaton Sync {
locations:
initial location(zero)
        invariant: true, labelling: false;

        location(process0)
        invariant: true, labelling: false;

        location(process1)
        invariant: true, labelling: false;
transitions:
            (zero, zero)
            action: start0,
            guard: true,
            reset: true;

            (zero, zero)
            action: start1,
            guard: true,
            reset: true;

            (zero, process0)
            action: setx0,
            guard: true,
            reset: true;

            (zero, process1)
            action: setx1,
            guard: true,
            reset: true;

            (process0, zero)
            action: setx00,
            guard: true,
            reset: true;

            (process1, zero)
            action: setx01,
            guard: true,
            reset: true;

            (process0, process0 )
            action: enter0,
            guard: true,
            reset: true;

            (process1, process1 )
            action: enter1,
            guard: true,
            reset: true;

            (process0, process0 )
            action: setx0,
            guard: true,
            reset: true;

            (process0, process1)
            action: setx1,
            guard: true,
            reset: true;

            (process1, process0)
            action: setx0,
            guard: true,
            reset: true;

            (process1, process1)
            action: setx1,
            guard: true,
            reset: true;
};
};
property: bad0 and bad1;
```

The model source should be paired with an experiment plan file. For example the
below plan first gives the command invoking the cvc3 SMT checker 
( *cvc3 -lang smtlib2* ), then follows with the definition of the logic 
( *QF_LRA* ), indicates the lower parameters in the model ( *delta* )
and says that the experiment should start with unwinding of the model up to 2
and synthesis of not more than 10 parameters and then move to 4 and 5 parameters,
etc.

```
cvc3 -lang smtlib2
QF_LRA
delta
2 10
4 5
5 2
```
