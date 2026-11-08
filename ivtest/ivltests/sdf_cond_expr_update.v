`timescale 1ps/1ps
module duplicate_expr_cell(input A, E, output Z);
  buf (Z, A);
  specify
    if (E == 1'b1) (A => Z) = (1, 1);
    if (E == 1'b1) (A => Z) = (2, 2);
  endspecify
endmodule
module edge_expr_cell(input A, E, output Z);
  buf (Z, A);
  specify
    if (E == 1'b1) (posedge A => (Z:A)) = (1, 1);
    if (E == 1'b1) (negedge A => (Z:A)) = (1, 1);
  endspecify
endmodule
module ordinary_expr_cell(input A, E, output Z);
  buf (Z, A);
  specify if (E == 1'b1) (A => Z) = (1, 1); endspecify
endmodule
module top;
  reg A = 0, E = 0;
  wire [2:0] Z;
  duplicate_expr_cell ud(A, E, Z[0]);
  edge_expr_cell ue(A, E, Z[1]);
  ordinary_expr_cell uo(A, E, Z[2]);
  initial begin
    $sdf_annotate("ivltests/sdf_cond_expr_update.sdf", top);
    #10 E = 1;
    #90 A = 1;
    #100 A = 0;
    #50 $sdf_annotate("ivltests/sdf_cond_expr_update_late.sdf", top);
    #50 A = 1;
    #100 A = 0;
    #50;
    $display("PASSED");
    $finish(0);
  end
  genvar i;
  for (i = 0; i < 3; i = i+1) begin: observe
    localparam RISE = i == 0 ? 11 : i == 1 ? 3 : 19;
    localparam FALL = i == 0 ? 7 : i == 1 ? 5 : 23;
    integer events = 0;
    always @(Z[i]) if ($time > 50) begin
      events = events + 1;
      case (events)
        1: if (Z[i] !== 1'b1 || $time != 100+RISE)
             $fatal(1, "FAIL initial rise %0d at %0t", i, $time);
        2: if (Z[i] !== 1'b0 || $time != 200+FALL)
             $fatal(1, "FAIL initial fall %0d at %0t", i, $time);
        3: if (Z[i] !== 1'b1 || $time != 300+(i == 0 ? 13 : RISE))
             $fatal(1, "FAIL late rise %0d at %0t", i, $time);
        4: if (Z[i] !== 1'b0 || $time != 400+FALL)
             $fatal(1, "FAIL retained fall %0d at %0t", i, $time);
        default: $fatal(1, "FAIL extra update event %0d", i);
      endcase
    end
    final if (events != 4) $fatal(1, "FAIL update output %0d count %0d", i, events);
  end
endmodule
