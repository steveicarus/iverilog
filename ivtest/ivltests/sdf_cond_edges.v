`timescale 1ps/1ps
module edge_cell(input A, E, output Z);
  buf (Z, A);
  specify
    if (E) (posedge A => (Z:A)) = (1, 1);
    if (E) (negedge A => (Z:A)) = (1, 1);
  endspecify
endmodule
module ordinary_cell(input A, output Z);
  buf (Z, A);
  specify (A => Z) = (1, 1); endspecify
endmodule
module legacy_conditional_cell(input A, E, output Z);
  buf (Z, A);
  specify if (E) (A => Z) = (1, 1); endspecify
endmodule
module top;
  reg A = 0, E = 0;
  wire [2:0] Z;
  edge_cell edge_u(A, E, Z[0]);
  ordinary_cell ordinary_u(A, Z[1]);
  legacy_conditional_cell legacy_u(A, E, Z[2]);
  initial begin
    $sdf_annotate("ivltests/sdf_cond_edges.sdf", top);
    #10 E = 1;
    #90 A = 1;
    #100 A = 0;
    #50;
    $display("PASSED");
    $finish(0);
  end
  genvar i;
  for (i = 0; i < 3; i = i+1) begin: check_output
    integer events = 0;
    always @(Z[i]) if ($time > 50) begin
      events = events + 1;
      if (!((events == 1 && Z[i] === 1'b1 && $time == 103+4*i) ||
            (events == 2 && Z[i] === 1'b0 && $time == 205+4*i)))
        $fatal(1, "FAIL edge/ordinary cell %0d at %0t = %b", i, $time, Z[i]);
    end
    final if (events != 2) $fatal(1, "FAIL cell %0d observed %0d", i, events);
  end
endmodule
