VVP Interactive Mode
====================

The vvp command has an interactive debug mode, where you can stop the
simulation and browse the current state of the simulation. There are
a couple ways to enter the debug mode, but once in interactive debug
mode, the usage is the same. Consider the example below:

.. code-block:: verilog

  module clock(output reg clock);
    initial clock = 1'b1;
    always #100 clock = !clock;
  endmodule // clock

  module main;

    reg [2:0] foo;
    wire	     clk;

    clock foo_clock(clk);

    always @(posedge clk)
      foo <= foo + 1;

    initial begin
       foo = 3'b000;
      #250 $stop;
    end

  endmodule

In examples that follow, we will use the above sample program.

Enter Interactive Mode
----------------------

The first and most common method is to put "$stop" system task
calls in the simulation at the times where you want to simulation
to break and enter interactive mode. The example above has a $stop,
so the output looks like this::

  ../foo.vl:25: $stop called at 250 (1s)
  ** VVP Stop(0) **
  ** Flushing output streams.
  ** Current simulation time is 250 ticks.
  >

You can get some interactive help by using the "help" command::

  > help
  Commands can be from the following table of base commands,
  or can be invocations of system tasks/functions.

  cd       - Synonym for push.
  cont     - Resume (continue) the simulation
  finish   - Finish the simulation.
  help     - Get help.
  list     - List items in the current scope.
  load     - Load a VPI module, a la vvp -m.
  ls       - Shorthand for "list".
  pop      - Pop one scope from the scope stack.
  push     - Descend into the named scope.
  step     - Single-step the scheduler for 1 event.
  time     - Print the current simulation time.
  trace    - Control statement tracing (on/off) when the code is instrumented.
  where    - Show current scope, and scope hierarchy stack.

  If the command name starts with a '$' character, it
  is taken to be the name of a system task, and a call is
  built up and executed. For example, "$display foo" will
  call the function as $display(foo).

You can also enter interactive mode at the terminal by interrupting the
execution with a "^C" (Control-C) character. The vvp engine catches the
terminal interrupt and drops you into the interactive prompt::

  ^C** VVP Stop(0) **
  ** Flushing output streams.
  ** Current simulation time is 533928600 ticks.
  >

This could be useful if you suspect that your simulation is stuck in
an infinite loop and you want to rummage around and see what's going on.

And finally, you can pass the "-s" command line flag to vvp to tell it
to execute "$stop" at the beginning of the simulation, before any other
events are executed. This may be useful as a way to manually set up some
details about the simulation.

Browsing the Design
-------------------

Now that you are in the interactive prompt, you can browse
around the design::

  > ls
  2 items in this scope:
  package : $unit
  module  : main
  > cd main
  > ls
  3 items in this scope:
  reg     : foo[2:0]
  module  : foo_clock
  net     : clk
  > where
  module main
  > $display foo
  1
  > cd foo_clock
  > where
  module foo_clock
  module main
  > ls
  2 items in this scope:
  port    : clock -- output
  reg     : clock

In the above example, the 'cd' and 'pop' commands descend into a scope
or pop back up a scope level. The 'where' command shows the scope stack,
and the 'ls' command lists the items present in the scope. With these
commands, one can browse freely throughout the design scope hierarchy.

It is also possible to call system tasks within the debug mode. The call
to the "$display" function is an example of this. In general, any system
task can be invoked, in the current context, with the objects that are
included on the command line passed as arguments. The arguments can be
variables or nets, and various kinds of literals::

  > ls
  2 items in this scope:
  port    : clock -- output
  reg     : clock
  > $display "Hello, World! " 10 " " clock
  Hello, World!          10 1

This is a great way to call custom system tasks as well. And system task
that vvp knows about can be invoked this way.

Leave Interactive Mode
----------------------

After you are done probing around in the interactive mode, you can
resume the simulation, or termimate execution. Resume the simulation
with the "cont" command, and terminate the simulation with the
"finish" command. The latter is the same as executing the
"$finish" system task.
