module dut(input [3:0] x);

initial begin
  $display("%b", x);
  if (x === 4'b1111)
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule

module test;
  dut dut('1);
endmodule
