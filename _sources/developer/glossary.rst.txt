
Glossary
========

Throughout Icarus Verilog descriptions and source code, I use a
variety of terms and acronyms that might be specific to Icarus
Verilog, have an Icarus Verilog specific meaning, or just aren't
widely known. So here I define these terms.


LRM     - Language Reference Manual
    This is a generic acronym, but in the Verilog world we sometimes
    mean *the* language reference manual, the IEEE1364 standard.


PLI     - Programming Language Interface
    This is a C API into Verilog simulators that is defined by the
    IEEE1364. There are two major interfaces, sometimes called PLI 1
    and PLI 2. PLI 2 is also often called VPI.


UDP     - User Defined Primitive
    These are objects that Verilog programmers define with the
    "primitive" keyword. They are truth-table based devices. The
    syntax for defining them is described in the LRM.


VPI     - Verilog Procedural Interface
    This is the C API that is defined by the Verilog standard, and
    that Icarus Verilog partially implements. See also PLI.


VVM     - Verilog Virtual Machine
    This is the Icarus Verilog runtime that works with the code
    generator that generates C++.


VVP     - Verilog Virtual Processor
    This is the Icarus Verilog runtime that reads in custom code in a
    form that I call "VVP Assembly".

LPM     - Library of Parameterized Modules
    LPM (Library of Parameterized Modules) is EIS-IS standard 103-A. It is
    a standard library of abstract devices that are designed to be close
    enough to the target hardware to be easily translated, yet abstract
    enough to support a variety of target technologies without excessive
    constraints. Icarus Verilog uses LPM internally to represent idealized
    hardware, especially when doing target neutral synthesis.
