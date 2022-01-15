/*
 * The base vpi_get() is not returning the correct result for
 * a signed value. There are obviously other problems as well.
 */
module top;
  reg [7:0] rval;
  reg signed [7:0] base; // This fails (no sign extension?).
//  reg signed [31:0] base; // This works on a 32 bit machine.
//  integer base; // And this works

  initial begin
    rval = 8'b10100101;
    for (base = 0; base > -8; base = base -1) begin
      $displayb("%3d %b ", base, rval[base +: 8], rval[base +: 8]);
    end
    $display;
    for (base = 0; base > -8; base = base -1) begin
      $displayb("%3d %b ", base+1, rval[base+1 +: 8], rval[base+1 +: 8]);
    end
  end
endmodule
