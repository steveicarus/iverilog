VVP as a library
================

If configured with ::

  --enable-libvvp

the vvp program will be built as a small stub that
depends on a shared library, libvvp.so.
The library may also be used to include a vvp simulation
in a larger program.  Typically, the simulation communicates
with its host program using VPI, but since
almost all the functions of vvp are included in the library
it may be possible to use text output and interactive mode.

The accessible functions of the library are defined and documented
in the header file, vvp/libvvp.h. Although vvp is a C++ program, the
header file presents a C interface.

Note that the vvp software was not designed to be used this way
and the library is a straightforward recompilation of the program code.
That imposes some restrictions, mostly arising from the use
of static variables: only a single run of a single simulation instance
can be expected to work without special actions.
To mitigate these restrictions, the library may by loaded dynamically
and unloaded at the end of each simulation run.
Parallel simulation should be possible by making multiple copies
of the library with different names.

