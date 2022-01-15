module test();

reg [1:0] src;
reg [2:0] dst;

initial begin
  assign dst = src;
  src = 2'b01;
  #1 $display("%b", dst);
  if (dst === 3'b001)
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
