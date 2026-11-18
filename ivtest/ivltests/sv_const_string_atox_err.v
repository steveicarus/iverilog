module top;
  parameter string str = "Not a value";
  parameter integer rd = str.atoi(1'b1);
  parameter integer rh = str.atohex(1'b1);
  parameter integer ro = str.atooct(1'b1);
  parameter integer rb = str.atobin(1'b1);

  initial $display("FAILED");
endmodule
