`timescale 1ps/1ps
module conditional_cell(input A, E, F, output Z);
  buf (Z, A);
  specify
    if (E) (A => Z) = (11, 13);
    if (F) (A => Z) = (17, 19);
  endspecify
endmodule
module expression_cell(input A, E, output Z);
  buf (Z, A);
  // A logically related expression must not acquire the bare identifier's
  // identity through condition-net optimization.
  specify if (E == 1'b1) (A => Z) = (31, 37); endspecify
endmodule
module vector_condition_cell(input A, input [1:0] E, output Z);
  buf (Z, A);
  specify if (E) (A => Z) = (41, 43); endspecify
endmodule
module selected_condition_cell(input A, input [1:0] E, output Z);
  buf (Z, A);
  specify if (E[0]) (A => Z) = (47, 49); endspecify
endmodule
module unconditional_cell(input A, E, output Z);
  buf (Z, A);
  specify (A => Z) = (17, 19); endspecify
endmodule
module top;
  reg A = 0, E = 0, F = 0, EE = 0;
  wire Z, ZE;
  wire [2:0] ZR;
  integer events = 0, expression_events = 0;
  conditional_cell u(A, E, F, Z);
  expression_cell ue(A, EE, ZE);
  vector_condition_cell uv(A, {1'b0, EE}, ZR[0]);
  selected_condition_cell us(A, {1'b0, EE}, ZR[1]);
  unconditional_cell uu(A, EE, ZR[2]);
  initial begin
    $sdf_annotate("ivltests/sdf_cond_reject.sdf", top);
    #10 begin E = 1; EE = 1; end
    #90 A = 1;
    #100 A = 0;
    #50 begin E = 0; F = 1; end
    #50 A = 1;
    #100 A = 0;
    #50;
    if (events != 4 || expression_events != 4)
      $fatal(1, "FAIL counts %0d %0d", events, expression_events);
    $display("PASSED");
    $finish(0);
  end
  always @(Z) if ($time > 50) begin
    events = events + 1;
    case (events)
      1: if (Z !== 1'b1 || $time != 111) $fatal(1, "FAIL unknown condition wrote rise at %0t", $time);
      2: if (Z !== 1'b0 || $time != 213) $fatal(1, "FAIL unknown condition wrote fall at %0t", $time);
      3: if (Z !== 1'b1 || $time != 323) $fatal(1, "FAIL later valid rise at %0t", $time);
      4: if (Z !== 1'b0 || $time != 429) $fatal(1, "FAIL later valid fall at %0t", $time);
      default: $fatal(1, "FAIL extra event at %0t", $time);
    endcase
  end
  always @(ZE) if ($time > 50) begin
    expression_events = expression_events + 1;
    case (expression_events)
      1: if (ZE !== 1'b1 || $time != 131) $fatal(1, "FAIL expression rise at %0t", $time);
      2: if (ZE !== 1'b0 || $time != 237) $fatal(1, "FAIL expression fall at %0t", $time);
      3: if (ZE !== 1'b1 || $time != 331) $fatal(1, "FAIL expression next rise at %0t", $time);
      4: if (ZE !== 1'b0 || $time != 437) $fatal(1, "FAIL expression next fall at %0t", $time);
      default: $fatal(1, "FAIL extra expression event at %0t", $time);
    endcase
  end
  genvar i;
  for (i = 0; i < 3; i = i+1) begin: check_rejected_target
    localparam RISE = i == 0 ? 41 : i == 1 ? 47 : 17;
    localparam FALL = i == 0 ? 43 : i == 1 ? 49 : 19;
    integer observed = 0;
    always @(ZR[i]) if ($time > 50) begin
      observed = observed + 1;
      case (observed)
        1: if (ZR[i] !== 1'b1 || $time != 100+RISE)
             $fatal(1, "FAIL rejected target %0d rise at %0t", i, $time);
        2: if (ZR[i] !== 1'b0 || $time != 200+FALL)
             $fatal(1, "FAIL rejected target %0d fall at %0t", i, $time);
        3: if (ZR[i] !== 1'b1 || $time != 300+RISE)
             $fatal(1, "FAIL rejected target %0d next rise at %0t", i, $time);
        4: if (ZR[i] !== 1'b0 || $time != 400+FALL)
             $fatal(1, "FAIL rejected target %0d next fall at %0t", i, $time);
        default: $fatal(1, "FAIL extra rejected-target event %0d", i);
      endcase
    end
    final if (observed != 4) $fatal(1, "FAIL target %0d observed %0d", i, observed);
  end
endmodule
