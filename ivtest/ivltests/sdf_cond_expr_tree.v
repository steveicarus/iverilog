`timescale 1ps/1ps
module tree_cell(input A, E0, E1, E2, E3, E4, output Z3, ZR, Z4, Z5);
  buf (Z3, A), (ZR, A), (Z4, A), (Z5, A);
  specify
    if (((E0 == 1'b1) && (E1 == 1'b0)) && (E2 == 1'b1))
      (A => Z3) = (1, 1);
    if ((E0 == 1'b1) && ((E1 == 1'b0) && (E2 == 1'b1)))
      (A => ZR) = (1, 1);
    if ((((E0 == 1'b1) && (E1 == 1'b0)) && (E2 == 1'b1)) && (E3 == 1'b0))
      (A => Z4) = (1, 1);
    if (((((E0 == 1'b1) && (E1 == 1'b0)) && (E2 == 1'b1)) && (E3 == 1'b0)) && (E4 == 1'b1))
      (A => Z5) = (1, 1);
  endspecify
endmodule
module top;
  reg A = 0, E0 = 0, E1 = 1, E2 = 0, E3 = 1, E4 = 0;
  wire [3:0] Z;
  tree_cell u(A, E0, E1, E2, E3, E4, Z[0], Z[1], Z[2], Z[3]);
  initial begin
    $sdf_annotate("ivltests/sdf_cond_expr_tree.sdf", top);
    #10 begin E0 = 1; E1 = 0; E2 = 1; E3 = 0; E4 = 1; end
    #90 A = 1;
    #100 A = 0;
    #50;
    $display("PASSED");
    $finish(0);
  end
  genvar i;
  for (i = 0; i < 4; i = i+1) begin: observe
    integer events = 0;
    always @(Z[i]) if ($time > 50) begin
      events = events + 1;
      if (!((events == 1 && Z[i] === 1'b1 && $time == 103+4*i) ||
            (events == 2 && Z[i] === 1'b0 && $time == 205+4*i)))
        $fatal(1, "FAIL ordered tree %0d at %0t = %b", i, $time, Z[i]);
    end
    final if (events != 2) $fatal(1, "FAIL ordered tree %0d count %0d", i, events);
  end
endmodule
