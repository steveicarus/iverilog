`timescale 1ps/1ps
// Both source conditions are false during annotation. Each instance receives
// distinct values, and the two comparison identities must remain separate.
module compare_cell(input A,
`ifdef TWO_STATE
                    input bit E, F,
`else
                    input E, F,
`endif
                    output Z);
  buf (Z, A);
  specify
    if (E == 1'b1) (A => Z) = (1, 1);
    if (F == 1'h0) (A => Z) = (1, 1);
  endspecify
endmodule
module top;
  reg A = 0, E = 0, F = 1;
  wire [1:0] Z;
  compare_cell u0(A, E, F, Z[0]);
  compare_cell u1(A, E, F, Z[1]);
  initial begin
    $sdf_annotate("ivltests/sdf_cond_expr_compare.sdf", top);
    #10 E = 1;
    #90 A = 1;
    #100 A = 0;
    #50 begin E = 0; F = 0; end
    #50 A = 1;
    #100 A = 0;
    #50;
    $display("PASSED");
    $finish(0);
  end
  genvar i;
  for (i = 0; i < 2; i = i+1) begin: observe
    integer events = 0;
    always @(Z[i]) if ($time > 50) begin
      events = events + 1;
      case (events)
        1: if (Z[i] !== 1'b1 || $time != (i ? 113 : 103))
             $fatal(1, "FAIL E rise cell %0d at %0t", i, $time);
        2: if (Z[i] !== 1'b0 || $time != (i ? 217 : 205))
             $fatal(1, "FAIL E fall cell %0d at %0t", i, $time);
        3: if (Z[i] !== 1'b1 || $time != (i ? 319 : 307))
             $fatal(1, "FAIL F rise cell %0d at %0t", i, $time);
        4: if (Z[i] !== 1'b0 || $time != (i ? 423 : 411))
             $fatal(1, "FAIL F fall cell %0d at %0t", i, $time);
        default: $fatal(1, "FAIL extra cell %0d event", i);
      endcase
    end
    final if (events != 4) $fatal(1, "FAIL cell %0d count %0d", i, events);
  end
endmodule
