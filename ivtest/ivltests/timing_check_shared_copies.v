// A resettable flip-flop with three checks sharing delayed terminals, as in
// the IHP standard cells. Annotate after a record with no matching check.
`timescale 1ns/1ps
module copy_ff(C, D, R, Q, ns, nf, nr);
  input C, D, R;
  output Q, ns, nf, nr;
  reg Q, ns, nf, nr;
  wire dc, dd, dr;
  initial begin ns = 0; nf = 0; nr = 0; end
  always @(posedge dc or negedge dr)
    if (!dr) Q <= 0;
    else Q <= dd;
  specify
    $setuphold(posedge C, posedge D, 0, 0, ns,,, dc, dd);
    $setuphold(posedge C, negedge D, 0, 0, nf,,, dc, dd);
    $recrem(posedge R, posedge C, 0, 0, nr,,, dr, dc);
  endspecify
endmodule

`timescale 1ps/1ps
module tb;
  reg C, D, R;
  wire Q, ns, nf, nr;
  copy_ff u(C, D, R, Q, ns, nf, nr);
  initial begin
    C=0; D=0; R=0;
    $sdf_annotate("ivltests/timing_check_shared_copies.sdf", u);
    #200 R=1;
    #200 D=1;
    #100 C=1; // 100 ps is inside the annotated 116 ps SETUP limit.
    #1;
    if (Q !== 1 || ns !== 1 || nf !== 0 || nr !== 0) begin
      $display("FAILED sampled data or timing checks");
      $finish;
    end
    C=0; D=0; #1;
    D=1; #116 C=1; // Exactly the limit: the notifier must stay quiet.
    #1 R=0; #1;
    if (Q !== 0 || ns !== 1 || nf !== 0 || nr !== 0) begin
      $display("FAILED reset or boundary");
      $finish;
    end
    $display("PASSED");
    $finish;
  end
endmodule
