
What Is LPM
===========

LPM (Library of Parameterized Modules) is EIS-IS standard 103-A. It is
a standard library of abstract devices that are designed to be close
enough to the target hardware to be easily translated, yet abstract
enough to support a variety of target technologies without excessive
constraints. Icarus Verilog uses LPM internally to represent idealized
hardware, especially when doing target neutral synthesis.

In general, the user does not even see the LPM that Icarus Verilog
generates, because the LPM devices are translated into technology
specific devices by the final code generator or target specific
optimizers.

Internal Uses Of LPM
--------------------

Internally, Icarus Verilog uses LPM devices to represent the design in
abstract, especially when synthesizing such functions as addition,
flip-flops, etc. The `synth` functor generates LPM modules when
interpreting procedural constructs. The functor generates the LPM
objects needed to replace a behavioral description, and uses
attributes to tag the devices with LPM properties.

Code generators need to understand the supported LPM devices so that
they can translate the devices into technology specific devices.
