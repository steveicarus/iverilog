module dut;

function y(input x);
  y = x;
endfunction

reg a, b;
reg c, d;

always @* begin
  c = y(a);
  d = y(b);
end

endmodule

module tb;

dut dut();

initial begin
  #1 dut.a = 0;
  #1 dut.b = 1;
  #1 $display(dut.a,,dut.b,,dut.c,,dut.d);
  if (dut.c === 0 && dut.d === 1)
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
