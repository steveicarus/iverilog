module child;
  parameter string memName = "";
  initial $display("child memName=%s", memName);
endmodule
module mid;
  parameter string memName = "";
  child #(.memName(memName)) c();
endmodule
module top;
  mid #(.memName("foo")) m();
endmodule
