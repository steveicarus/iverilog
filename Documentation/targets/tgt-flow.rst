The Flow Dataflow Exporter (-tflow)
===================================

The flow code generator exports the elaborated design as a JSON
*dataflow database* rather than a simulation or netlist artifact. The
output describes the module hierarchy, the ports/parameters/signals of
each module, and the dataflow graph (which processes and continuous
assignments read and drive which nets), so that IDEs and analysis tools
can render dataflow panels, browse the elaborated hierarchy, and trace
input-to-output continuity across the design.

The emitted schema (``flowtracer1.merged.v0``) is shared with the GHDL
``--flow`` exporter, so a single consumer works across Verilog (via this
target) and VHDL (via GHDL) designs.


USAGE
-----

::

  % iverilog -tflow -o<path>.flow  <source files>...

The flow target runs at elaboration time; no simulation is performed.
The root module of the elaborated design becomes the ``top`` of the
exported database. Because the exporter reads the fully elaborated
netlist, parameter values, generate blocks, and instance bindings are
all resolved.


OUTPUT
------

The output is a single JSON object with these top-level fields:

``schema``
    Always ``"flowtracer1.merged.v0"``.

``top``
    The basename of the root module instance.

``modules``
    One entry per module *type*, each with ``ports`` (name/direction/
    type), ``generics`` (parameters), ``signals``, the name-based
    ``processes`` and ``assignments`` (with ``reads``/``drives``/
    ``sensitivity`` given as signal names), and the ``instances`` it
    contains (with ``port_map`` and resolved ``generic_map``).

``hierarchy``
    The elaborated instance tree. Each node carries its ``port_map``
    (formal-to-actual associations resolved through the shared net) and
    ``children``. Generate blocks appear as transparent frames marked
    ``"scope": true`` with a ``generate`` object describing the kind
    (``for``/``if``), label, and index.

``nets`` and ``cells``
    A secondary, integer-id dataflow graph: ``nets`` are the canonical
    nexuses (with ``drivers``/``loads`` referring to cell ids) and
    ``cells`` are the processes, gates, LPM devices, and constants (with
    ``drives``/``reads`` referring to net ids, plus ``clocked`` and
    ``clock_net``).


LIMITATIONS
-----------

Continuity through bit- and part-selects (for example a generate that
connects ``.d(bus[i])``) is not yet represented at the individual-bit
level; whole-signal connections are tracked.

For a process sensitive to several edges (for example an
asynchronous-reset flip-flop ``@(posedge clk or posedge rst)``), the
``clock_net`` field reports the first edge signal.

Constant tie-offs and synthetic compiler temporaries are summarized
rather than reported individually.
