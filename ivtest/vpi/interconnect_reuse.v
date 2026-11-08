`timescale 1ps/1ps

module identity(input I, output Q);
  assign Q = I;
endmodule

module top;
  reg drive;
  wire source_q, left_q, right_q, peer_q;
  identity source(.I(drive), .Q(source_q));
  identity left(.I(source_q), .Q(left_q));
  identity right(.I(source_q), .Q(right_q));
  identity peer(.I(source_q), .Q(peer_q));

  initial begin
    $interconnect_reuse;
    $sdf_annotate("vpi/interconnect_reuse.sdf", top);
    $interconnect_reuse;
    drive = 0;
    #50 $interconnect_reuse;
    #50 drive = 1;
    #100 drive = 0;
    #50 $sdf_annotate("vpi/interconnect_reuse_partial.sdf", top);
    $interconnect_reuse;
    #50 drive = 1;
    #100 drive = 0;
    #50 $sdf_annotate("vpi/interconnect_reuse.sdf", top);
    $interconnect_reuse;
    #50 drive = 1;
    #100 drive = 0;
    #50 $finish(0);
  end
endmodule
