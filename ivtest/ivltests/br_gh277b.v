module dut;

reg a, b, c;
reg d;

function z(input x, input y);
  z = x + y;
endfunction

function y(input x);
  y = z(x, b) + z(x, c);
endfunction

always_comb begin
  d = y(a);
end

endmodule

module tb;

dut dut();

initial begin
  #1 dut.a = 0;
  #1 dut.b = 0;
  #1 dut.c = 1;
  #1 $display(dut.a,,dut.b,,dut.c,,dut.d);
  if (dut.d === 1)
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
