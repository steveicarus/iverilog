module top;
  reg foo;
  always @* foo <= 0;
  initial #1 $display("foo is %b", foo);
endmodule
