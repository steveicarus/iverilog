
THE VVP TARGET

SYMBOL NAME CONVENTIONS

There are some naming conventions that the vp target uses for
generating symbol names.

* wires and regs

Nets and variables are named V_<full-name> where <full-name> is the
full hierarchical name of the signal.

* Logic devices

Logic devices (and, or, buf, bufz, etc.) are named L_<full_name>. In
this case the symbol is attached to a functor that is the output of
the logic device.
