// Here are two examples of $strobe failing. It appears that thread data
// is being cleaned up too soon for the $strobe to access it.
module test;
  reg[4:0] j;
  reg [5:0] in [1:0];
  wire [5:0] out;

  assign out = in[j]; // This uses the current j.

  initial begin
    in[1] = 6'b110001;

    j = 1;
    #1; // Need some delay for the calculations to run.
    $display("out: %b, in[%0d] %b:", out, j, in[j]);
    $display("out[3:0]: %b, in[%0d] %b:", out[j*1-1 +: 4], j, in[j]);

    // in[j] is what is failing.
    $strobe("out: %b, in[%0d] %b:", out, j, in[j]);

    // out[j... is what is failing.
    $strobe("out[3:0]: %b, in[%0d] %b:", out[j*1-1 +: 4], j, in[j]);
//    #1; // Adding this will work around the bug.
  end

endmodule
