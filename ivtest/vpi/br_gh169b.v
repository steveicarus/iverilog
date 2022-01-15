module dut1(input real i1, output real o1);

assign o1 = i1;

endmodule

module dut2(input real i2, output real o2);

assign o2 = i2;

endmodule

module test();

real a, b, c;

dut2 dut2(a, b);
dut1 dut1(b, c);

initial begin
  $list_vars;
end

endmodule
