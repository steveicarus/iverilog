`timescale 1ps/1ps
// The escaped bare name resembles the comparison's source spelling. Both
// share A->Z, so naive string concatenation would overwrite the wrong owner.
module escaped_expr_cell(input A, E, input \E==1'b1 , input \E&&F ,
                         input \E1:E1 , input \gate"quote , input \gate\back ,
                         output Z, ZC, ZQ, ZB);
  buf (Z, A), (ZC, A), (ZQ, A), (ZB, A);
  specify
    if (\E==1'b1 ) (A => Z) = (1, 1);
    if (E == 1'b1) (A => Z) = (1, 1);
    if (\E&&F  == 1'b1) (A => ZC) = (1, 1);
    if (\E1:E1 ) (A => Z) = (1, 1);
    if (\gate"quote  == 1'b1) (A => ZQ) = (1, 1);
    if (\gate\back  == 1'b1) (A => ZB) = (1, 1);
  endspecify
endmodule
module top;
  reg A = 0, E = 0, H = 0, K = 0;
  wire [3:0] Z;
  escaped_expr_cell u(A, E, H, K, K, K, K, Z[0], Z[1], Z[2], Z[3]);
  initial begin
    $sdf_annotate("ivltests/sdf_cond_expr_escaped.sdf", top);
    #10 begin H = 1; K = 1; end
    #90 A = 1;
    #100 A = 0;
    #50 begin H = 0; E = 1; end
    #50 A = 1;
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
      case (events)
        1: if (Z[i] !== 1'b1 || $time != (i == 0 ? 103 : i == 1 ? 113 : i == 2 ? 119 : 129))
             $fatal(1, "FAIL escaped rise %0d at %0t", i, $time);
        2: if (Z[i] !== 1'b0 || $time != (i == 0 ? 205 : i == 1 ? 217 : i == 2 ? 223 : 231))
             $fatal(1, "FAIL escaped fall %0d at %0t", i, $time);
        3: if (Z[i] !== 1'b1 || $time != (i == 0 ? 307 : i == 1 ? 313 : i == 2 ? 319 : 329))
             $fatal(1, "FAIL comparison rise %0d at %0t", i, $time);
        4: if (Z[i] !== 1'b0 || $time != (i == 0 ? 411 : i == 1 ? 417 : i == 2 ? 423 : 431))
             $fatal(1, "FAIL comparison fall %0d at %0t", i, $time);
        default: $fatal(1, "FAIL extra escaped event %0d", i);
      endcase
    end
    final if (events != 4) $fatal(1, "FAIL escaped output %0d count %0d", i, events);
  end
endmodule
