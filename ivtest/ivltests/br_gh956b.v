module sub;
  reg y, z, ctrl;
  tranif1(y, ctrl, x);
  and(z, ctrl, y);
endmodule

module top;
  sub i1();
  sub i2();
  initial $display("FAILED - should be a compile error");
endmodule
