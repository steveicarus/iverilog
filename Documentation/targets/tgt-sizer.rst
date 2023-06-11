
The sizer Code Analyzer (-tvvp)
===============================

The sizer target does not generate any code. Instead it will print statistics about the Verilog code.

It is important to synthesize the Verilog code before invoking the sizer. This can be done with the `-S` flag passed to iverilog. Note, that behavioral code can not be synthesized and will generate a warning when passed to the sizer.

Example command::

    % iverilog -o sizer.txt -tsizer -S -s top input.v

With this example code:

.. code-block:: verilog

    module top (
        input clock,
        input reset,
        output blink
    );
        reg out;

        always @(posedge clock) begin
            if (reset) begin
                out = 1'b0;
            end else begin
                out <= !out;
            end
        end

        assign blink = out;

    endmodule

The resulting `sizer.txt` will contain::

    **** module/scope: top
         Flip-Flops   : 1
         Logic Gates  : 3
         MUX[2]: 1 slices
         LOG[13]: 1 unaccounted
         LOG[14]: 1 unaccounted
    **** TOTALS
         Flip-Flops   : 1
         Logic Gates  : 3
         MUX[2]: 1 slices
         LOG[13]: 1 unaccounted
         LOG[14]: 1 unaccounted
