module top;
  reg in, out;

  not(out, in); // This is an error in orignal Verilog

  initial $display("PASSED"); // ,but works when forcing SV
endmodule
