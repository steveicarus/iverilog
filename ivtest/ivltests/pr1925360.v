`define MAC(i) $display(i);

module top;
initial begin
  if ("$display(in);" != ``MAC(in))
    $display("FAILED: expected \"display(in);\", got \"`MAC(in)\"");
  else $display("PASSED");
end
endmodule
