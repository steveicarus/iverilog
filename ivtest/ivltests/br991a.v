module test();

reg	  clk;
reg [1:0] state;

always begin
  case (state)
    0       : state = 1;
    1       : state = 2;
    default : /* do nothing */ ;
  endcase
  @(posedge clk);
end

reg failed = 0;

initial begin
  clk = 0;
  state = 0;
  #1 clk = 1;
  #1 clk = 0;
  if (state !== 1) failed = 1;
  #1 clk = 1;
  #1 clk = 0;
  if (state !== 2) failed = 1;
  #1 clk = 1;
  #1 clk = 0;
  if (state !== 2) failed = 1;

  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
