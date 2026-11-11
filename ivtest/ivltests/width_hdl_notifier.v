// IEEE 1364-2005 15.5 / Table 15-13: X may become either binary value.
// Keep this fixture in Verilog-1995 syntax for the vlog95 round trip.
`timescale 1ps/1ps
primitive notifier_effect(q, reset, notifier);
  output q;
  reg q;
  input reset, notifier;
  table
    1    ? : ? : 0;
    (10) ? : ? : -;
    0    * : ? : x;
  endtable
endprimitive

module notifier_leaf(CLK, reset, q);
  parameter INITIAL = 1'b0;
  input CLK, reset;
  output q;
  reg notifier;
  initial notifier = INITIAL;
  notifier_effect effect(q, reset, notifier);
  specify $width(posedge CLK, 10, 0, notifier); endspecify
endmodule

module notifier_shared(CLK, reset, q);
  input CLK, reset;
  output q;
  reg notifier;
  initial notifier = 1'b0;
  notifier_effect effect(q, reset, notifier);
  specify
    $width(posedge CLK, 10, 0, notifier);
    $width(posedge CLK, 12, 0, notifier);
  endspecify
endmodule

module notifier_omitted(CLK);
  input CLK;
  specify $width(posedge CLK, 10); endspecify
endmodule

module tb;
  reg CLK, reset;
  wire q0, q1, qx, qz, qs;
  notifier_leaf #(1'b0) n0(CLK, reset, q0);
  notifier_leaf #(1'b1) n1(CLK, reset, q1);
  notifier_leaf #(1'bx) nx(CLK, reset, qx);
  notifier_leaf #(1'bz) nz(CLK, reset, qz);
  notifier_shared shared(CLK, reset, qs);
  notifier_omitted omitted(CLK);
  initial begin
    CLK = 0;
    #10 CLK = 1;
    #5 CLK = 0;
    #85 CLK = 1;
    #5 CLK = 0;
  end
  initial begin
    reset = 1;
    #1 $notifier_audit(0);
    #1 reset = 0;
    #14 $notifier_checkpoint(1);
    #90 $notifier_checkpoint(2);
    $finish;
  end
endmodule
