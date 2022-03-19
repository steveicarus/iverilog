module tb();

reg  [7:0] in[1:0];
wire [7:0] out[1:0];

assign out[0] = $clog2(in[0]);
assign out[1] = $clog2(in[1]);

reg failed;

initial begin
  failed = 0;

  #1 in[0] = 1;
  #1 $display("%0d -> %0d", in[0],out[0]);
  if (out[0] !== 0) failed = 1;
  #1 in[1] = 2;
  #1 $display("%0d -> %0d", in[1],out[1]);
  if (out[1] !== 1) failed = 1;
  #1 in[0] = 3;
  #1 $display("%0d -> %0d", in[0],out[0]);
  if (out[0] !== 2) failed = 1;

  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
