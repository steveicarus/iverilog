module child;
  parameter string memName = "";
  initial $display("child memName=%s", memName);
endmodule
module top;
  parameter string memName = "m";
  child #(.memName({memName, "_", "0"})) c();
endmodule
