
Debug Aids For VVP
==================

Debugging vvp can be fiendishly difficult, so there are some built in
debugging aids. These are enabled by setting the environment variable
VVP_DEBUG to the path to an output file. Then, various detailed debug
tools can be enabled as described below.

* .resolv

The .resolv can print debug information along with a label by
specifying the debug output label on the .resolv line::

   .resolv tri$<label>

In this case, the "$" character directly after the "tri" enables debug
dumps for this node, and the <label> is the label to prepend to log
messages from this node.
