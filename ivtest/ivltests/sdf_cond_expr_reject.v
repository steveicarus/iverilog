`timescale 1ps/1ps
module reject_expr_cell(input A, E, F, input [1:0] V, input [0:0] W,
                        output Z, ZCASE, ZV, ZI, ZBARE, ZW, ZUNSIZED, ZSIGNED, ZWIDE);
  buf (Z, A), (ZCASE, A), (ZV, A), (ZI, A), (ZBARE, A), (ZW, A), (ZUNSIZED, A), (ZSIGNED, A), (ZWIDE, A);
  specify
    if (E == 1'b1) (A => Z) = (31, 37);
    if (E === 1'b1) (A => ZCASE) = (41, 43);
    if (V == 1'b1) (A => ZV) = (47, 49);
    if (V[0] == 1'b1) (A => ZI) = (53, 59);
    if (E) (A => ZBARE) = (61, 67);
    if (W == 1'b1) (A => ZW) = (71, 73);
    if (E == 1) (A => ZUNSIZED) = (79, 83);
    if (E == 1'sb1) (A => ZSIGNED) = (17, 19);
    if (E == 2'b01) (A => ZWIDE) = (23, 29);
  endspecify
endmodule
module top;
  reg A = 0, E = 0, F = 0;
  reg [1:0] V = 0;
  reg [0:0] W = 0;
  wire [8:0] Z;
  reject_expr_cell u(A, E, F, V, W, Z[0], Z[1], Z[2], Z[3], Z[4], Z[5], Z[6], Z[7], Z[8]);
  initial begin
    $sdf_annotate("ivltests/sdf_cond_expr_reject.sdf", top);
    #10 begin E = 1; V = 1; W = 1; end
    #90 A = 1;
    #100 A = 0;
    #100;
    $display("PASSED");
    $finish(0);
  end
  genvar i;
  for (i = 0; i < 9; i = i+1) begin: observe
    localparam RISE = i == 0 ? 3 : i == 1 ? 41 : i == 2 ? 47 : i == 3 ? 53 : i == 4 ? 61 : i == 5 ? 71 : i == 6 ? 79 : i == 7 ? 17 : 23;
    localparam FALL = i == 0 ? 5 : i == 1 ? 43 : i == 2 ? 49 : i == 3 ? 59 : i == 4 ? 67 : i == 5 ? 73 : i == 6 ? 83 : i == 7 ? 19 : 29;
    integer events = 0;
    always @(Z[i]) if ($time > 90) begin
      events = events + 1;
      if (!((events == 1 && Z[i] === 1'b1 && $time == 100+RISE) ||
            (events == 2 && Z[i] === 1'b0 && $time == 200+FALL)))
        $fatal(1, "FAIL rejected binding %0d at %0t = %b", i, $time, Z[i]);
    end
    final if (events != 2) $fatal(1, "FAIL rejection output %0d count %0d", i, events);
  end
endmodule
