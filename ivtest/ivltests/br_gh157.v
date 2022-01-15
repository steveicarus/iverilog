module dut();

localparam x = 2;

initial $display("%0d", x);

endmodule

module test();

dut dut();

defparam dut.x = 1;

endmodule
