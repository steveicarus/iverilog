module dut(a,);

  input wire a;

endmodule

module top;

  wire a;

  dut i(.*);

endmodule
