module test();

integer src;
reg     dst;

initial begin
  assign dst = src;
  src = 1;
  #1 $display(dst);
  if (dst === 1)
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
