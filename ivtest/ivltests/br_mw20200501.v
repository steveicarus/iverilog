module test();

reg [9:0] buffer[$];
reg [9:0] out;

initial begin
  buffer.push_back(3);
  out = buffer.pop_front();
  $display("out = %0d", out);
  if (out === 3)
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
