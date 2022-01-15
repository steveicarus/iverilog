module dut(i, o);

input  [3:0] i;
output [3:0] o;

wire logic [3:0] i;
reg        [3:0] o;

always @* o = i;

endmodule

module test();

logic [3:0] i, o;

dut dut(i, o);

reg failed = 0;

initial begin
  i = 4'b01xz;
  #0 $display("%b %b", i, o);
  if (o !== 4'b01xz) failed = 1;

  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
