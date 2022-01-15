module top();

always @(sub.i) sub.j = sub.i;

initial begin:sub
  static integer i = 1;
  static integer j = 0;
  #0 $display("%0d %0d", i, j);
  if ((i === 1) && (j === 0))
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule // main
