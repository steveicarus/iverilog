`timescale 1ns/10ps

module TBUF_X2 (A, EN, Z);
  input A;
  input EN;
  output Z;

  bufif1(Z, A, EN);

  specify
    (A => Z) = (0.1, 0.2);
    (EN => Z) = (0.3, 0.4);
  endspecify
endmodule

module ckt (out, in, en);
  output out;
  input in, en;

  TBUF_X2 dut (.A ( in ) , .EN ( en ) , .Z ( out ) ) ;
endmodule

module top;
  wire out;
  reg in, en;

  ckt dut(out, in, en);

  initial begin
    $monitor($realtime,,out,"=",in,,en);
    $sdf_annotate("ivltests/br960d.sdf", dut);
    in = 1'b0;
    en = 1'b0;
    $display("Max (X->Z)");
    // X -> Z = max(enable))
    #10;
    en = 1'b1;
    $display("Fall (Z->0)");
    // Z -> 0 = tr(enable)
    #10;
    en = 1'b0;
    $display("Rise (0->Z)");
    // 0 -> Z = tr(enable)
    #5;
    in = 1'b1;
    #5;
    en = 1'b1;
    $display("Rise (Z->1)");
    // Z -> 1 = tr(enable)
    #10;
    en = 1'b0;
    $display("Fall (1->Z)");
    // 1 -> Z = tr(enable)
    #10;
  end
endmodule
