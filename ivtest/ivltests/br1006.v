module dut(
  input  wire [3:0] i,
  output wire [3:0] o
);

assign o = i;

specify
  (i[3:0] => o[3:0]) = (1, 1);
endspecify

endmodule

module top();

reg  [3:0] i;
wire [3:0] o;

dut dut(i, o);

reg failed = 0;

initial begin
  $monitor($time,,i,,o);
  #1 i = 4'd1;
  #0 if (o !== 4'bx) failed = 1;
  #1 i = 4'd2;
  #0 if (o !== 4'd1) failed = 1;
  #1;
  #0 if (o !== 4'd2) failed = 1;
  #1;

  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
