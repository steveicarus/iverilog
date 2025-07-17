module ssub(o);
  output o;
endmodule

module sub;
  reg x;
  ssub i(x);
endmodule

module top;
  sub i1();
  sub i2();
  // This will work for SystemVerilog
  initial $display("PASSED");
endmodule
