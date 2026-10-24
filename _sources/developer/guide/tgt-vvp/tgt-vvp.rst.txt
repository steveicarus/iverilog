
The VVP Target
==============

Symbol Name Conventions
-----------------------

There are some naming conventions that the vvp target uses for
generating symbol names.

* wires and regs

Nets and variables are named V_<full-name> where <full-name> is the
full hierarchical name of the signal.

* Logic devices

Logic devices (and, or, buf, bufz, etc.) are named L_<full_name>. In
this case the symbol is attached to a functor that is the output of
the logic device.


General Functor Web Structure
-----------------------------

The net of gates, signals and resolvers is formed from the input
design. The basic structure is wrapped around the nexus, which is
represented by the ivl_nexus_t.

Each nexus represents a resolved value. The input of the nexus is fed
by a single driver. If the nexus in the design has multiple drivers,
the drivers are first fed into a resolver (or a tree of resolvers) to
form a single output that is the nexus.

The nexus, then, feeds its output to the inputs of other gates, or to
the .net objects in the design.
