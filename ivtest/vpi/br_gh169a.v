module dut1(input wire i1, output reg o1);

always @* o1 = i1;

endmodule

module dut2(input wire i2, output reg o2);

always @* o2 = i2;

endmodule

module test();

wire a, b, c;

dut2 dut2(a, b);
dut1 dut1(b, c);

initial begin
  $list_vars;
end

endmodule
