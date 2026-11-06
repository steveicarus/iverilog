`timescale 1ps/1ps
// Both matching declarations must be annotated. Leaving either default active
// changes the selected minimum from the intended 3/5 ps to 1/1 or 2/2 ps.
module conditional_cell(input A, E, output Z);
  buf (Z, A);
  specify
    if (E) (A => Z) = (1, 1);
    if (E) (A => Z) = (2, 2);
  endspecify
endmodule
module top;
  reg A = 0, E = 0;
  wire Z;
  integer events = 0;
  conditional_cell u(A, E, Z);
  initial begin
    $sdf_annotate("ivltests/sdf_cond_multimatch.sdf", top);
    #10 E = 1;
    #90 A = 1;
    #100 A = 0;
    #50;
    if (events != 2) $fatal(1, "FAIL observed %0d events", events);
    $display("PASSED");
    $finish(0);
  end
  always @(Z) if ($time > 50) begin
    events = events + 1;
    if (!((events == 1 && Z === 1'b1 && $time == 103) ||
          (events == 2 && Z === 1'b0 && $time == 205)))
      $fatal(1, "FAIL all-matches at %0t = %b", $time, Z);
  end
endmodule
