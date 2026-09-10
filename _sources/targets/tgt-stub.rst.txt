
The stub Code Generator (-tstub)
================================

The stub code generator is a debugging aid for the Icarus Verilog compiler
itself. It outputs a text dump of the elaborated design as it is passed to
code generators.

Example command::

    % iverilog -o stub.txt -tstub -s top input.v

With this example code:

.. code-block:: verilog

    module top;
        initial $display("Hello World!");
    endmodule

The resulting `stub.txt` will contain::

    root module = top
    scope: top (0 parameters, 0 signals, 0 logic) module top time units = 1e0
     time precision = 1e0
    end scope top
    # There are 0 constants detected
    initial
        Call $display(1 parameters); /* hello_world.v:2 */
            <string="Hello World!", width=96, type=bool>
