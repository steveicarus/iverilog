Observing UDP Data Before a Module Path
======================================

The vvp target can expose the actual scalar combinational UDP output that
feeds a module path, before the module path delays that output. This is an
Icarus-specific VPI extension for passive observation. It does not add a
second UDP or change the module path's delays or event scheduling.

Enable the extension when compiling the design::

    iverilog -gspecify -pudp_prepath_vpi=1 -o design.vvp design.v

The default is disabled. ``-gspecify`` enables the module paths needed by
this interface. The parameter applies to the vvp target.

Discovery
---------

Starting with a ``vpiModPath`` handle obtained by iterating the paths in a
module, request the extension relationship defined in ``vpi_user.h``:

.. code-block:: c

    vpiHandle data = vpi_handle(_vpiModPathData, path);
    if (data != NULL) {
        s_vpi_value value;
        value.format = vpiScalarVal;
        vpi_get_value(data, &value);
    }

``_vpiModPathData`` is not an IEEE VPI relationship. When available, the
returned handle has type ``vpiNet`` and size one. It refers to the existing
UDP output node, not the source input that triggers a path and not the
delayed public output. The existing
``vpiModPathOut`` / ``vpiExpr`` relationship continues to identify the
public output after module-path processing.

Different source-path declarations that use the same eligible output node
return the same data handle. A client that registers callbacks on every
path should deduplicate these handles when it wants one event stream per
node. The associated module and path terminals provide the design context.
The generated net name is an implementation detail: do not discover or
associate the node by parsing pointer strings or compiler-generated names.

Availability
------------

The relationship is available only when all of the following hold:

* The extension is enabled at compilation.
* The module-path destination is a scalar output in the UDP's own scope.
* Its driver is an unambiguous, single combinational UDP output.
* The primitive has no explicit delay.
* The connection uses plain wire semantics, without switches, pull defaults
  or wired-AND/wired-OR resolution.

The relationship returns ``NULL`` when the extension is disabled or the
path does not meet these conditions. In particular, this is not general
driver traversal: sequential UDPs, explicitly delayed UDPs, vectors,
multiple-driver connections and ordinary non-UDP logic are not exposed by
this relationship. Switch-connected, ``tri0``, ``tri1``, ``wand`` and
``wor`` connections are also excluded. Absence must not be replaced by
observing the public output and treating it as pre-delay data.

Values and callbacks
--------------------

Clients can discover the handles at ``cbEndOfCompile`` and register
``cbValueChange`` callbacks using the normal signal interface. Read the
value and simulation time at each callback to observe changes on the
actual node. A pre-delay event and its eventual external delivery are
different observations; for example, nonzero module-path delays normally
place the external transition later. Not every observed pre-delay change
necessarily becomes an external transition.

Before simulation starts, a read can precede evaluation of the UDP.
Do not treat that value as the settled result at time zero. Use the
callbacks to retain transitions and a ``cbReadOnlySynch`` observation to
read the settled value for a time slot. A settled snapshot does not replace
the intermediate callbacks. A callback reports a change on this node, not
every input event or every evaluation that leaves its value unchanged.

Ownership and passive use
-------------------------

The returned object is the existing, scope-owned signal handle. Queries
borrow that handle; they do not create a new signal or transfer ownership.
It remains valid for the simulation lifetime. Removing a callback removes
that registration, not the underlying signal. Releasing this persistent
handle with ``vpi_free_object`` does not delete the signal or invalidate
other users of it.

The extension makes an ordinary net visible. It is not a write-protected
view: the ordinary VPI signal interface still applies. Passive clients must
only read values and register observation callbacks; they must not force,
deposit or otherwise drive this node. Such writes can affect the design
and are outside the passive-observation contract.

Enabling the option also makes the selected net visible in its scope's
net iteration. It reuses the existing node and filter, with one owning
handle, rather than adding a duplicate driver or an alias with separate
cleanup ownership.
