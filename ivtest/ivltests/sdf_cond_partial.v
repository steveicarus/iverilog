`timescale 1ps/1ps
module conditional_cell(input A, E, output Z);
  buf (Z, A);
  specify if (E) (A => Z) = (1, 1); endspecify
endmodule
module top;
  reg A = 0, E = 0;
  wire Z;
  integer events = 0;
  conditional_cell u(A, E, Z);
  initial begin
    $sdf_annotate("ivltests/sdf_cond_partial.sdf", top);
    #10 E = 1;
    #90 A = 1;
    #100 A = 0;
    #50 $sdf_annotate("ivltests/sdf_cond_partial_late.sdf", top);
    #50 A = 1;
    #100 A = 0;
    #50;
    if (events != 4) $fatal(1, "FAIL observed %0d events", events);
    $display("PASSED");
    $finish(0);
  end
  always @(Z) if ($time > 50) begin
    events = events + 1;
    case (events)
      1: if (Z !== 1'b1 || $time != 111) $fatal(1, "FAIL initial rise at %0t", $time);
      2: if (Z !== 1'b0 || $time != 207) $fatal(1, "FAIL retained fall at %0t", $time);
      3: if (Z !== 1'b1 || $time != 313) $fatal(1, "FAIL late rise at %0t", $time);
      4: if (Z !== 1'b0 || $time != 407) $fatal(1, "FAIL late retained fall at %0t", $time);
      default: $fatal(1, "FAIL extra event at %0t", $time);
    endcase
  end
endmodule
