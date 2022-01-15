module test();

wire [7:0] value1;
reg  [7:0] value2;

reg clk;

assign value1[3:0] = 4'b1010;

always @(posedge clk) begin
  value2[3:0] <= value1;
end

(* ivl_synthesis_off *)
initial begin
  #1 clk = 0;
  #1 clk = 1;
  #1 clk = 0;
  $display("%b %b", value1, value2);
`ifdef __ICARUS_SYNTH__
  if (value2 === 8'bzzzz1010)
`else
  if (value2 === 8'bxxxx1010)
`endif
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
