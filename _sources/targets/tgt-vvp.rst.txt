
The vvp Code Generator (-tvvp)
==============================

The vvp target generates code for the "vvp" run time. This is the most
commonly used target for Icarus Verilog, as it is the main simulation engine.

Example command::

    %  iverilog -o top.vvp -s top hello_world.v

Equivalent command::

    %  iverilog -o top.vvp -tvvp -s top hello_world.v

With this example code in `hello_world.v`:

.. code-block:: verilog

    module top;
        initial $display("Hello World!");
    endmodule

The resulting `top.vvp` will contain something similar to::

    #! /usr/local/bin/vvp
    :ivl_version "13.0 (devel)" "(s20221226-119-g8cb2e1a05-dirty)";
    :ivl_delay_selection "TYPICAL";
    :vpi_time_precision + 0;
    :vpi_module "/usr/local/lib/ivl/system.vpi";
    :vpi_module "/usr/local/lib/ivl/vhdl_sys.vpi";
    :vpi_module "/usr/local/lib/ivl/vhdl_textio.vpi";
    :vpi_module "/usr/local/lib/ivl/v2005_math.vpi";
    :vpi_module "/usr/local/lib/ivl/va_math.vpi";
    S_0x563c3c5d1540 .scope module, "top" "top" 2 1;
     .timescale 0 0;
        .scope S_0x563c3c5d1540;
    T_0 ;
        %vpi_call 2 2 "$display", "Hello World!" {0 0 0};
        %end;
        .thread T_0;
    # The file index is used to find the file name in the following table.
    :file_names 3;
        "N/A";
        "<interactive>";
        "hello_world.v";

The first line contains the shebang. If this file is executed, the shebang tells the shell to use vvp for the execution of this file.

To run the simulation, execute::

    %  ./top.vvp

Or you can call vvp directly::

    %  vvp top.vvp

Next are some directives. The first one, `:ivl_version` specifies which version of iverilog this file was created with. Next is the delay selection with "min:typical:max" values and the time precision, which we did not set specifically, so the default value is used. The next lines tell vvp which VPI modules to load and in which order. The next lines tell vvp which VPI modules to load and in what order. Next, a new scope is created with the `.scope` directive and the timescale is set with `.timescale`. A thread `T_0` is created that contains two instructions: `%vpi_call` executes the VPI function `$display` with the specified arguments, and `%end` terminates the simulation.

Opcodes
-------

The various available opcodes can be seen in :doc:`Opcodes <../developer/guide/vvp/opcodes>`
