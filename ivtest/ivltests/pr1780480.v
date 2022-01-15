// show bug in icarus verilog
// this shouldn't crash, should it?

`timescale 1ns/1ns

module top ();

reg [1:0] count;

initial begin
   count = 0;
   #70 $finish(0);
end

always
    count = #20 count + 1;

initial
    // It seems to be count[0] that does it here,
    // If I change it to count then everything works fine.
    $monitor("%0t\ta(%b)", $time, count[0]);

endmodule
